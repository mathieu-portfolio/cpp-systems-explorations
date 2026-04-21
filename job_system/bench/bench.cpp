#include "job_system.hpp"
#include "thread_pool.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace
{
enum class Scenario
{
  Independent,
  Chain,
  FanIn
};

struct Config
{
  Scenario scenario = Scenario::Independent;
  std::size_t workers = 4;
  std::size_t jobs = 10000;
  std::size_t work_units = 1000;
  std::size_t iterations = 5;
};

struct Result
{
  std::string scenario;
  std::string mode;
  std::size_t workers = 0;
  std::size_t jobs = 0;
  std::size_t work_units = 0;
  std::size_t iterations = 0;
  double best_ms = 0.0;
  double avg_ms = 0.0;
  double throughput_jobs_per_sec = 0.0;
  double speedup_vs_direct = 0.0;
};

struct Options
{
  bool sweep = false;
  std::string preset = "all";
  std::string csv_out;
  Config config;
};

void print_usage(const char* program)
{
  std::cout
    << "Usage: " << program << " [options]\n"
    << '\n'
    << "Single-run options:\n"
    << "  --scenario NAME   independent | chain | fan_in (default: independent)\n"
    << "  --workers N       Number of worker threads (default: 4)\n"
    << "  --jobs N          Total number of jobs (default: 10000)\n"
    << "  --work N          Work units per job (default: 1000)\n"
    << "  --iterations N    Number of repeated runs (default: 5)\n"
    << '\n'
    << "Sweep options:\n"
    << "  --sweep           Run a benchmark matrix instead of a single scenario\n"
    << "  --preset NAME     scaling | granularity | dependencies | all (default: all)\n"
    << "  --csv-out PATH    Write CSV results to PATH\n"
    << '\n'
    << "Other:\n"
    << "  --help            Show this help message\n";
}

std::size_t parse_size(std::string_view text, const char* flag)
{
  try
  {
    return static_cast<std::size_t>(std::stoull(std::string(text)));
  }
  catch (const std::exception&)
  {
    throw std::invalid_argument(std::string("Invalid numeric value for ") + flag);
  }
}

Scenario parse_scenario(std::string_view text)
{
  if (text == "independent") return Scenario::Independent;
  if (text == "chain") return Scenario::Chain;
  if (text == "fan_in") return Scenario::FanIn;
  throw std::invalid_argument("Invalid scenario. Expected independent, chain, or fan_in");
}

std::string scenario_name(Scenario scenario)
{
  switch (scenario)
  {
    case Scenario::Independent: return "independent";
    case Scenario::Chain: return "chain";
    case Scenario::FanIn: return "fan_in";
  }

  return "unknown";
}

void validate_config(const Config& config)
{
  if (config.workers == 0)
  {
    throw std::invalid_argument("--workers must be greater than 0");
  }

  if (config.jobs == 0)
  {
    throw std::invalid_argument("--jobs must be greater than 0");
  }

  if (config.work_units == 0)
  {
    throw std::invalid_argument("--work must be greater than 0");
  }

  if (config.iterations == 0)
  {
    throw std::invalid_argument("--iterations must be greater than 0");
  }

  if (config.scenario == Scenario::FanIn && config.jobs < 2)
  {
    throw std::invalid_argument("fan_in requires --jobs >= 2");
  }
}

Options parse_args(int argc, char** argv)
{
  Options options;

  for (int i = 1; i < argc; ++i)
  {
    const std::string arg = argv[i];

    auto require_value = [&](const char* flag) -> std::string_view {
      if (i + 1 >= argc)
      {
        throw std::invalid_argument(std::string("Missing value for ") + flag);
      }

      return argv[++i];
    };

    if (arg == "--scenario")
    {
      options.config.scenario = parse_scenario(require_value("--scenario"));
    }
    else if (arg == "--workers")
    {
      options.config.workers = parse_size(require_value("--workers"), "--workers");
    }
    else if (arg == "--jobs")
    {
      options.config.jobs = parse_size(require_value("--jobs"), "--jobs");
    }
    else if (arg == "--work")
    {
      options.config.work_units = parse_size(require_value("--work"), "--work");
    }
    else if (arg == "--iterations")
    {
      options.config.iterations = parse_size(require_value("--iterations"), "--iterations");
    }
    else if (arg == "--sweep")
    {
      options.sweep = true;
    }
    else if (arg == "--preset")
    {
      options.preset = std::string(require_value("--preset"));
    }
    else if (arg == "--csv-out")
    {
      options.csv_out = std::string(require_value("--csv-out"));
    }
    else if (arg == "--help")
    {
      print_usage(argv[0]);
      std::exit(0);
    }
    else
    {
      throw std::invalid_argument("Unknown argument: " + arg);
    }
  }

  if (
    options.preset != "scaling" &&
    options.preset != "granularity" &&
    options.preset != "dependencies" &&
    options.preset != "all")
  {
    throw std::invalid_argument("Invalid preset. Expected scaling, granularity, dependencies, or all");
  }

  validate_config(options.config);
  return options;
}

void do_spin_work(std::size_t work_units)
{
  volatile std::uint64_t sink = 0;

  for (std::size_t i = 0; i < work_units; ++i)
  {
    sink += static_cast<std::uint64_t>(i * 1664525u + 1013904223u);
    sink ^= (sink << 13);
    sink ^= (sink >> 7);
    sink ^= (sink << 17);
  }

  (void)sink;
}

template <typename Fn>
double time_ms(Fn&& fn)
{
  const auto start = std::chrono::steady_clock::now();
  fn();
  const auto end = std::chrono::steady_clock::now();

  return std::chrono::duration<double, std::milli>(end - start).count();
}

Result benchmark_direct(const Config& config)
{
  std::vector<double> samples;
  samples.reserve(config.iterations);

  for (std::size_t iteration = 0; iteration < config.iterations; ++iteration)
  {
    samples.push_back(time_ms([&] {
      for (std::size_t i = 0; i < config.jobs; ++i)
      {
        do_spin_work(config.work_units);
      }
    }));
  }

  const double best_ms = *std::min_element(samples.begin(), samples.end());

  double avg_ms = 0.0;
  for (double sample : samples)
  {
    avg_ms += sample;
  }
  avg_ms /= static_cast<double>(samples.size());

  Result result;
  result.scenario = scenario_name(config.scenario);
  result.mode = "direct";
  result.workers = 1;
  result.jobs = config.jobs;
  result.work_units = config.work_units;
  result.iterations = config.iterations;
  result.best_ms = best_ms;
  result.avg_ms = avg_ms;
  result.throughput_jobs_per_sec = (static_cast<double>(config.jobs) * 1000.0) / best_ms;
  result.speedup_vs_direct = 1.0;
  return result;
}

Result benchmark_thread_pool(const Config& config, double direct_best_ms)
{
  std::vector<double> samples;
  samples.reserve(config.iterations);

  for (std::size_t iteration = 0; iteration < config.iterations; ++iteration)
  {
    samples.push_back(time_ms([&] {
      ThreadPool pool(config.workers);

      std::atomic<std::size_t> completed{0};
      std::mutex completion_mutex;
      std::condition_variable completion_cv;

      for (std::size_t i = 0; i < config.jobs; ++i)
      {
        pool.submit([&] {
          do_spin_work(config.work_units);

          const std::size_t done =
            completed.fetch_add(1, std::memory_order_acq_rel) + 1;

          if (done == config.jobs)
          {
            std::lock_guard<std::mutex> lock(completion_mutex);
            completion_cv.notify_one();
          }
        });
      }

      std::unique_lock<std::mutex> lock(completion_mutex);
      completion_cv.wait(lock, [&] {
        return completed.load(std::memory_order_acquire) == config.jobs;
      });
    }));
  }

  const double best_ms = *std::min_element(samples.begin(), samples.end());

  double avg_ms = 0.0;
  for (double sample : samples)
  {
    avg_ms += sample;
  }
  avg_ms /= static_cast<double>(samples.size());

  Result result;
  result.scenario = scenario_name(config.scenario);
  result.mode = "thread_pool";
  result.workers = config.workers;
  result.jobs = config.jobs;
  result.work_units = config.work_units;
  result.iterations = config.iterations;
  result.best_ms = best_ms;
  result.avg_ms = avg_ms;
  result.throughput_jobs_per_sec = (static_cast<double>(config.jobs) * 1000.0) / best_ms;
  result.speedup_vs_direct = direct_best_ms / best_ms;
  return result;
}

void populate_job_system(JobSystem& js, const Config& config)
{
  std::vector<JobId> ids;
  ids.reserve(config.jobs);

  for (std::size_t i = 0; i < config.jobs; ++i)
  {
    ids.push_back(js.create_job([work = config.work_units] {
      do_spin_work(work);
    }));
  }

  if (config.scenario == Scenario::Independent)
  {
    return;
  }

  if (config.scenario == Scenario::Chain)
  {
    for (std::size_t i = 1; i < ids.size(); ++i)
    {
      js.add_dependency(ids[i], ids[i - 1]);
    }
    return;
  }

  if (config.scenario == Scenario::FanIn)
  {
    const JobId final_job = ids.back();
    for (std::size_t i = 0; i + 1 < ids.size(); ++i)
    {
      js.add_dependency(final_job, ids[i]);
    }
  }
}

Result benchmark_job_system(const Config& config, double direct_best_ms)
{
  std::vector<double> samples;
  samples.reserve(config.iterations);

  for (std::size_t iteration = 0; iteration < config.iterations; ++iteration)
  {
    samples.push_back(time_ms([&] {
      JobSystem js(config.workers);
      populate_job_system(js, config);
      js.run();
      js.wait();
    }));
  }

  const double best_ms = *std::min_element(samples.begin(), samples.end());

  double avg_ms = 0.0;
  for (double sample : samples)
  {
    avg_ms += sample;
  }
  avg_ms /= static_cast<double>(samples.size());

  Result result;
  result.scenario = scenario_name(config.scenario);
  result.mode = "job_system";
  result.workers = config.workers;
  result.jobs = config.jobs;
  result.work_units = config.work_units;
  result.iterations = config.iterations;
  result.best_ms = best_ms;
  result.avg_ms = avg_ms;
  result.throughput_jobs_per_sec = (static_cast<double>(config.jobs) * 1000.0) / best_ms;
  result.speedup_vs_direct = direct_best_ms / best_ms;
  return result;
}

void print_result(const Result& result)
{
  std::cout
    << std::left
    << std::setw(14) << result.scenario
    << std::setw(12) << result.mode
    << " workers=" << std::setw(3) << result.workers
    << " jobs=" << std::setw(8) << result.jobs
    << " work=" << std::setw(8) << result.work_units
    << " best_ms=" << std::setw(10) << std::fixed << std::setprecision(3) << result.best_ms
    << " avg_ms=" << std::setw(10) << std::fixed << std::setprecision(3) << result.avg_ms
    << " throughput=" << std::setw(12) << std::fixed << std::setprecision(1) << result.throughput_jobs_per_sec
    << " speedup=" << std::setw(8) << std::fixed << std::setprecision(3) << result.speedup_vs_direct
    << '\n';
}

void write_csv(const std::string& path_string, const std::vector<Result>& results)
{
  const std::filesystem::path path(path_string);

  if (path.has_parent_path())
  {
    std::filesystem::create_directories(path.parent_path());
  }

  std::ofstream out(path);
  if (!out)
  {
    throw std::runtime_error("Failed to open CSV output file: " + path.string());
  }

  out << "scenario,mode,workers,jobs,work_units,iterations,best_ms,avg_ms,throughput_jobs_per_sec,speedup_vs_direct\n";
  out << std::fixed << std::setprecision(6);

  for (const auto& result : results)
  {
    out
      << result.scenario << ','
      << result.mode << ','
      << result.workers << ','
      << result.jobs << ','
      << result.work_units << ','
      << result.iterations << ','
      << result.best_ms << ','
      << result.avg_ms << ','
      << result.throughput_jobs_per_sec << ','
      << result.speedup_vs_direct << '\n';
  }
}

void run_one_scenario(std::vector<Result>& results, const Config& config)
{
  const Result direct = benchmark_direct(config);
  const Result pool = benchmark_thread_pool(config, direct.best_ms);
  const Result job_system = benchmark_job_system(config, direct.best_ms);

  results.push_back(direct);
  results.push_back(pool);
  results.push_back(job_system);

  print_result(direct);
  print_result(pool);
  print_result(job_system);
}

void append_scaling_scenarios(std::vector<Result>& results)
{
  const std::vector<std::size_t> workers = {1, 2, 4, 8};

  for (std::size_t worker_count : workers)
  {
    Config config;
    config.scenario = Scenario::Independent;
    config.workers = worker_count;
    config.jobs = 10000;
    config.work_units = 1000;
    config.iterations = 5;

    run_one_scenario(results, config);
  }
}

void append_granularity_scenarios(std::vector<Result>& results)
{
  const std::vector<std::size_t> work_units = {10, 100, 1000, 10000, 100000};

  for (std::size_t work : work_units)
  {
    Config config;
    config.scenario = Scenario::Independent;
    config.workers = 4;
    config.jobs = 10000;
    config.work_units = work;
    config.iterations = 5;

    run_one_scenario(results, config);
  }
}

void append_dependency_scenarios(std::vector<Result>& results)
{
  for (Scenario scenario : {Scenario::Independent, Scenario::Chain, Scenario::FanIn})
  {
    Config config;
    config.scenario = scenario;
    config.workers = 4;
    config.jobs = 1000;
    config.work_units = 1000;
    config.iterations = 5;

    run_one_scenario(results, config);
  }
}

} // namespace

int main(int argc, char** argv)
{
  try
  {
    const Options options = parse_args(argc, argv);
    std::vector<Result> results;

    if (options.sweep)
    {
      if (options.preset == "scaling" || options.preset == "all")
      {
        append_scaling_scenarios(results);
      }

      if (options.preset == "granularity" || options.preset == "all")
      {
        append_granularity_scenarios(results);
      }

      if (options.preset == "dependencies" || options.preset == "all")
      {
        append_dependency_scenarios(results);
      }
    }
    else
    {
      run_one_scenario(results, options.config);
    }

    if (!options.csv_out.empty())
    {
      write_csv(options.csv_out, results);
      std::cout << "csv_out=" << options.csv_out << '\n';
    }

    return 0;
  }
  catch (const std::exception& ex)
  {
    std::cerr << "Benchmark configuration error: " << ex.what() << '\n';
    return 1;
  }
}
