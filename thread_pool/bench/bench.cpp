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
enum class WorkloadType
{
  Spin,
  Sleep
};

struct Config
{
  std::size_t workers = 4;
  std::size_t jobs = 10000;
  std::size_t producers = 1;
  std::size_t work_units = 1000;
  std::size_t iterations = 5;
  WorkloadType workload = WorkloadType::Spin;
};

struct Result
{
  std::string scenario;
  std::string mode;
  std::string workload;
  std::size_t workers = 0;
  std::size_t producers = 0;
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
    << "  --workers N       Number of worker threads (default: 4)\n"
    << "  --jobs N          Total number of jobs (default: 10000)\n"
    << "  --producers N     Number of submitter threads (default: 1)\n"
    << "  --workload NAME   spin | sleep (default: spin)\n"
    << "  --work N          Work units per job (default: 1000)\n"
    << "  --iterations N    Number of repeated runs (default: 5)\n"
    << '\n'
    << "Sweep options:\n"
    << "  --sweep           Run a benchmark matrix instead of a single scenario\n"
    << "  --preset NAME     scaling | granularity | contention | all (default: all)\n"
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

WorkloadType parse_workload(std::string_view text)
{
  if (text == "spin")
  {
    return WorkloadType::Spin;
  }

  if (text == "sleep")
  {
    return WorkloadType::Sleep;
  }

  throw std::invalid_argument("Invalid workload. Expected 'spin' or 'sleep'");
}

std::string workload_name(WorkloadType type)
{
  switch (type)
  {
    case WorkloadType::Spin:
      return "spin";
    case WorkloadType::Sleep:
      return "sleep";
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

  if (config.producers == 0)
  {
    throw std::invalid_argument("--producers must be greater than 0");
  }

  if (config.iterations == 0)
  {
    throw std::invalid_argument("--iterations must be greater than 0");
  }
}

Options parse_args(int argc, char** argv)
{
  Options options;

  for (int i = 1; i < argc; ++i)
  {
    const std::string arg = argv[i];

    auto require_value = [&](const char* flag) -> std::string_view
    {
      if (i + 1 >= argc)
      {
        throw std::invalid_argument(std::string("Missing value for ") + flag);
      }
      return argv[++i];
    };

    if (arg == "--workers")
    {
      options.config.workers = parse_size(require_value("--workers"), "--workers");
    }
    else if (arg == "--jobs")
    {
      options.config.jobs = parse_size(require_value("--jobs"), "--jobs");
    }
    else if (arg == "--producers")
    {
      options.config.producers = parse_size(require_value("--producers"), "--producers");
    }
    else if (arg == "--work")
    {
      options.config.work_units = parse_size(require_value("--work"), "--work");
    }
    else if (arg == "--iterations")
    {
      options.config.iterations = parse_size(require_value("--iterations"), "--iterations");
    }
    else if (arg == "--workload")
    {
      options.config.workload = parse_workload(require_value("--workload"));
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
    options.preset != "contention" &&
    options.preset != "all")
  {
    throw std::invalid_argument("Invalid preset. Expected scaling, granularity, contention, or all");
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

void do_sleep_work(std::size_t work_units)
{
  std::this_thread::sleep_for(std::chrono::microseconds(work_units));
}

void do_work(WorkloadType type, std::size_t work_units)
{
  switch (type)
  {
    case WorkloadType::Spin:
      do_spin_work(work_units);
      break;
    case WorkloadType::Sleep:
      do_sleep_work(work_units);
      break;
  }
}

template <typename Fn>
double time_ms(Fn&& fn)
{
  const auto start = std::chrono::steady_clock::now();
  fn();
  const auto end = std::chrono::steady_clock::now();
  return std::chrono::duration<double, std::milli>(end - start).count();
}

Result benchmark_direct(const std::string& scenario, const Config& config)
{
  std::vector<double> samples;
  samples.reserve(config.iterations);

  for (std::size_t iteration = 0; iteration < config.iterations; ++iteration)
  {
    samples.push_back(time_ms([&] {
      for (std::size_t job = 0; job < config.jobs; ++job)
      {
        do_work(config.workload, config.work_units);
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
  result.scenario = scenario;
  result.mode = "direct";
  result.workload = workload_name(config.workload);
  result.workers = 1;
  result.producers = 1;
  result.jobs = config.jobs;
  result.work_units = config.work_units;
  result.iterations = config.iterations;
  result.best_ms = best_ms;
  result.avg_ms = avg_ms;
  result.throughput_jobs_per_sec = (static_cast<double>(config.jobs) * 1000.0) / best_ms;
  result.speedup_vs_direct = 1.0;
  return result;
}

Result benchmark_pool(const std::string& scenario, const Config& config, double direct_best_ms)
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

      const auto submit_one_job = [&] {
        pool.submit([&] {
          do_work(config.workload, config.work_units);

          const std::size_t done = completed.fetch_add(1, std::memory_order_acq_rel) + 1;
          if (done == config.jobs)
          {
            std::lock_guard<std::mutex> lock(completion_mutex);
            completion_cv.notify_one();
          }
        });
      };

      const std::size_t jobs_per_producer = config.jobs / config.producers;
      const std::size_t remainder = config.jobs % config.producers;

      std::vector<std::thread> producers;
      producers.reserve(config.producers);

      for (std::size_t producer_index = 0; producer_index < config.producers; ++producer_index)
      {
        const std::size_t assigned_jobs =
          jobs_per_producer + (producer_index < remainder ? 1u : 0u);

        producers.emplace_back([&, assigned_jobs] {
          for (std::size_t job = 0; job < assigned_jobs; ++job)
          {
            submit_one_job();
          }
        });
      }

      for (auto& producer : producers)
      {
        producer.join();
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
  result.scenario = scenario;
  result.mode = "thread_pool";
  result.workload = workload_name(config.workload);
  result.workers = config.workers;
  result.producers = config.producers;
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
    << " workload=" << std::setw(6) << result.workload
    << " workers=" << std::setw(3) << result.workers
    << " producers=" << std::setw(3) << result.producers
    << " jobs=" << std::setw(8) << result.jobs
    << " work=" << std::setw(8) << result.work_units
    << " best_ms=" << std::setw(10) << std::fixed << std::setprecision(3) << result.best_ms
    << " avg_ms=" << std::setw(10) << std::fixed << std::setprecision(3) << result.avg_ms
    << " throughput=" << std::setw(12) << std::fixed << std::setprecision(1)
    << result.throughput_jobs_per_sec
    << " speedup=" << std::setw(8) << std::fixed << std::setprecision(3)
    << result.speedup_vs_direct
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

  out << "scenario,mode,workload,workers,producers,jobs,work_units,iterations,best_ms,avg_ms,throughput_jobs_per_sec,speedup_vs_direct\n";
  out << std::fixed << std::setprecision(6);

  for (const auto& result : results)
  {
    out
      << result.scenario << ','
      << result.mode << ','
      << result.workload << ','
      << result.workers << ','
      << result.producers << ','
      << result.jobs << ','
      << result.work_units << ','
      << result.iterations << ','
      << result.best_ms << ','
      << result.avg_ms << ','
      << result.throughput_jobs_per_sec << ','
      << result.speedup_vs_direct << '\n';
  }
}

void run_one_scenario(std::vector<Result>& results, const std::string& scenario, const Config& config)
{
  const Result direct = benchmark_direct(scenario, config);
  const Result pool = benchmark_pool(scenario, config, direct.best_ms);

  results.push_back(direct);
  results.push_back(pool);

  print_result(direct);
  print_result(pool);
}

void append_scaling_scenarios(std::vector<Result>& results)
{
  const std::vector<std::size_t> workers = {1, 2, 4, 8};
  const std::vector<std::size_t> work_units = {1000, 10000};

  for (std::size_t work : work_units)
  {
    for (std::size_t worker_count : workers)
    {
      Config config;
      config.workers = worker_count;
      config.jobs = 10000;
      config.producers = 1;
      config.work_units = work;
      config.iterations = 5;
      config.workload = WorkloadType::Spin;

      const std::string scenario =
        "scaling_w" + std::to_string(worker_count) + "_work" + std::to_string(work);

      run_one_scenario(results, scenario, config);
    }
  }
}

void append_granularity_scenarios(std::vector<Result>& results)
{
  const std::vector<std::size_t> work_units = {10, 100, 1000, 10000, 100000};

  for (std::size_t work : work_units)
  {
    Config config;
    config.workers = 4;
    config.jobs = 10000;
    config.producers = 1;
    config.work_units = work;
    config.iterations = 5;
    config.workload = WorkloadType::Spin;

    const std::string scenario = "granularity_work" + std::to_string(work);
    run_one_scenario(results, scenario, config);
  }
}

void append_contention_scenarios(std::vector<Result>& results)
{
  const std::vector<std::size_t> producers = {1, 2, 4, 8};

  for (std::size_t producer_count : producers)
  {
    Config config;
    config.workers = 4;
    config.jobs = 10000;
    config.producers = producer_count;
    config.work_units = 1000;
    config.iterations = 5;
    config.workload = WorkloadType::Spin;

    const std::string scenario = "contention_p" + std::to_string(producer_count);
    run_one_scenario(results, scenario, config);
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

      if (options.preset == "contention" || options.preset == "all")
      {
        append_contention_scenarios(results);
      }
    }
    else
    {
      run_one_scenario(results, "single", options.config);
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
