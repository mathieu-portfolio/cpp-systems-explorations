#include "arena.hpp"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <new>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace
{
enum class Scenario
{
  AllocOnly,
  AllocReset,
  MarkRewind
};

struct Config
{
  Scenario scenario = Scenario::AllocOnly;
  std::size_t allocation_size = 32;
  std::size_t alignment = 16;
  std::size_t count = 100000;
  std::size_t iterations = 5;
};

struct Result
{
  std::string scenario;
  std::string mode;
  std::size_t allocation_size = 0;
  std::size_t alignment = 0;
  std::size_t count = 0;
  std::size_t iterations = 0;
  double best_ms = 0.0;
  double avg_ms = 0.0;
  double throughput_ops_per_sec = 0.0;
  double speedup_vs_heap = 0.0;
};

struct Options
{
  bool sweep = false;
  std::string preset = "all";
  std::string csv_out;
  Config config;
};

std::string scenario_name(Scenario scenario)
{
  switch (scenario)
  {
    case Scenario::AllocOnly: return "alloc_only";
    case Scenario::AllocReset: return "alloc_reset";
    case Scenario::MarkRewind: return "mark_rewind";
  }

  return "unknown";
}

Scenario parse_scenario(std::string_view text)
{
  if (text == "alloc_only") return Scenario::AllocOnly;
  if (text == "alloc_reset") return Scenario::AllocReset;
  if (text == "mark_rewind") return Scenario::MarkRewind;
  throw std::invalid_argument("Invalid scenario. Expected alloc_only, alloc_reset, or mark_rewind");
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

bool is_power_of_two(std::size_t value)
{
  return value != 0 && (value & (value - 1)) == 0;
}

void validate_config(const Config& config)
{
  if (config.allocation_size == 0)
  {
    throw std::invalid_argument("--size must be greater than 0");
  }

  if (config.alignment == 0)
  {
    throw std::invalid_argument("--alignment must be greater than 0");
  }

  if (!is_power_of_two(config.alignment))
  {
    throw std::invalid_argument("--alignment must be a power of two");
  }

  if (config.count == 0)
  {
    throw std::invalid_argument("--count must be greater than 0");
  }

  if (config.iterations == 0)
  {
    throw std::invalid_argument("--iterations must be greater than 0");
  }
}

void print_usage(const char* program)
{
  std::cout
    << "Usage: " << program << " [options]\n"
    << '\n'
    << "Single-run options:\n"
    << "  --scenario NAME   alloc_only | alloc_reset | mark_rewind (default: alloc_only)\n"
    << "  --size N          Allocation size in bytes (default: 32)\n"
    << "  --alignment N     Allocation alignment in bytes (default: 16)\n"
    << "  --count N         Number of allocation operations (default: 100000)\n"
    << "  --iterations N    Number of repeated runs (default: 5)\n"
    << '\n'
    << "Sweep options:\n"
    << "  --sweep           Run a benchmark matrix instead of a single scenario\n"
    << "  --preset NAME     sizes | alignments | patterns | all (default: all)\n"
    << "  --csv-out PATH    Write CSV results to PATH\n"
    << '\n'
    << "Other:\n"
    << "  --help            Show this help message\n";
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

    if (arg == "--scenario")
    {
      options.config.scenario = parse_scenario(require_value("--scenario"));
    }
    else if (arg == "--size")
    {
      options.config.allocation_size = parse_size(require_value("--size"), "--size");
    }
    else if (arg == "--alignment")
    {
      options.config.alignment = parse_size(require_value("--alignment"), "--alignment");
    }
    else if (arg == "--count")
    {
      options.config.count = parse_size(require_value("--count"), "--count");
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
    options.preset != "sizes" &&
    options.preset != "alignments" &&
    options.preset != "patterns" &&
    options.preset != "all")
  {
    throw std::invalid_argument("Invalid preset. Expected sizes, alignments, patterns, or all");
  }

  validate_config(options.config);
  return options;
}

template <typename Fn>
double time_ms(Fn&& fn)
{
  const auto start = std::chrono::steady_clock::now();
  fn();
  const auto end = std::chrono::steady_clock::now();

  return std::chrono::duration<double, std::milli>(end - start).count();
}

std::size_t compute_arena_capacity(const Config& config)
{
  const std::size_t padded = config.allocation_size + config.alignment;
  const std::size_t extra = 1024;
  return padded * config.count + extra;
}

void benchmark_arena_alloc_only(const Config& config)
{
  Arena arena(compute_arena_capacity(config), config.alignment);

  std::vector<void*> ptrs;
  ptrs.reserve(config.count);

  for (std::size_t i = 0; i < config.count; ++i)
  {
    void* ptr = arena.allocate(config.allocation_size, config.alignment);
    if (ptr == nullptr)
    {
      throw std::runtime_error("Arena allocation failed during alloc_only");
    }

    ptrs.push_back(ptr);
    static_cast<unsigned char*>(ptr)[0] = static_cast<unsigned char>(i & 0xFF);
  }
}

void benchmark_heap_alloc_only(const Config& config)
{
  std::vector<void*> ptrs;
  ptrs.reserve(config.count);

  for (std::size_t i = 0; i < config.count; ++i)
  {
    void* ptr = ::operator new(config.allocation_size, std::align_val_t(config.alignment));
    ptrs.push_back(ptr);
    static_cast<unsigned char*>(ptr)[0] = static_cast<unsigned char>(i & 0xFF);
  }

  for (void* ptr : ptrs)
  {
    ::operator delete(ptr, std::align_val_t(config.alignment));
  }
}

void benchmark_arena_alloc_reset(const Config& config)
{
  Arena arena(compute_arena_capacity(config), config.alignment);

  for (std::size_t i = 0; i < config.count; ++i)
  {
    void* ptr = arena.allocate(config.allocation_size, config.alignment);
    if (ptr == nullptr)
    {
      throw std::runtime_error("Arena allocation failed during alloc_reset");
    }

    static_cast<unsigned char*>(ptr)[0] = static_cast<unsigned char>(i & 0xFF);
  }

  arena.reset();
}

void benchmark_heap_alloc_reset(const Config& config)
{
  std::vector<void*> ptrs;
  ptrs.reserve(config.count);

  for (std::size_t i = 0; i < config.count; ++i)
  {
    void* ptr = ::operator new(config.allocation_size, std::align_val_t(config.alignment));
    ptrs.push_back(ptr);
    static_cast<unsigned char*>(ptr)[0] = static_cast<unsigned char>(i & 0xFF);
  }

  for (void* ptr : ptrs)
  {
    ::operator delete(ptr, std::align_val_t(config.alignment));
  }
}

void benchmark_arena_mark_rewind(const Config& config)
{
  Arena arena(compute_arena_capacity(config), config.alignment);

  constexpr std::size_t batch_size = 64;
  const std::size_t batches = (config.count + batch_size - 1) / batch_size;

  for (std::size_t batch = 0; batch < batches; ++batch)
  {
    auto marker = arena.mark();

    const std::size_t start = batch * batch_size;
    const std::size_t end = std::min(start + batch_size, config.count);

    for (std::size_t i = start; i < end; ++i)
    {
      void* ptr = arena.allocate(config.allocation_size, config.alignment);
      if (ptr == nullptr)
      {
        throw std::runtime_error("Arena allocation failed during mark_rewind");
      }

      static_cast<unsigned char*>(ptr)[0] = static_cast<unsigned char>(i & 0xFF);
    }

    arena.rewind(marker);
  }
}

void benchmark_heap_mark_rewind(const Config& config)
{
  constexpr std::size_t batch_size = 64;
  const std::size_t batches = (config.count + batch_size - 1) / batch_size;

  for (std::size_t batch = 0; batch < batches; ++batch)
  {
    const std::size_t start = batch * batch_size;
    const std::size_t end = std::min(start + batch_size, config.count);

    std::vector<void*> ptrs;
    ptrs.reserve(end - start);

    for (std::size_t i = start; i < end; ++i)
    {
      void* ptr = ::operator new(config.allocation_size, std::align_val_t(config.alignment));
      ptrs.push_back(ptr);
      static_cast<unsigned char*>(ptr)[0] = static_cast<unsigned char>(i & 0xFF);
    }

    for (void* ptr : ptrs)
    {
      ::operator delete(ptr, std::align_val_t(config.alignment));
    }
  }
}

Result benchmark_mode(const std::string& mode, const Config& config)
{
  std::vector<double> samples;
  samples.reserve(config.iterations);

  for (std::size_t iteration = 0; iteration < config.iterations; ++iteration)
  {
    samples.push_back(time_ms([&] {
      if (config.scenario == Scenario::AllocOnly)
      {
        if (mode == "arena")
        {
          benchmark_arena_alloc_only(config);
        }
        else
        {
          benchmark_heap_alloc_only(config);
        }
      }
      else if (config.scenario == Scenario::AllocReset)
      {
        if (mode == "arena")
        {
          benchmark_arena_alloc_reset(config);
        }
        else
        {
          benchmark_heap_alloc_reset(config);
        }
      }
      else if (config.scenario == Scenario::MarkRewind)
      {
        if (mode == "arena")
        {
          benchmark_arena_mark_rewind(config);
        }
        else
        {
          benchmark_heap_mark_rewind(config);
        }
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
  result.mode = mode;
  result.allocation_size = config.allocation_size;
  result.alignment = config.alignment;
  result.count = config.count;
  result.iterations = config.iterations;
  result.best_ms = best_ms;
  result.avg_ms = avg_ms;
  result.throughput_ops_per_sec = static_cast<double>(config.count) * 1000.0 / best_ms;
  return result;
}

void print_result(const Result& result)
{
  std::cout
    << std::left
    << std::setw(14) << result.scenario
    << std::setw(10) << result.mode
    << " size=" << std::setw(8) << result.allocation_size
    << " align=" << std::setw(6) << result.alignment
    << " count=" << std::setw(10) << result.count
    << " best_ms=" << std::setw(10) << std::fixed << std::setprecision(3) << result.best_ms
    << " avg_ms=" << std::setw(10) << std::fixed << std::setprecision(3) << result.avg_ms
    << " throughput=" << std::setw(12) << std::fixed << std::setprecision(1) << result.throughput_ops_per_sec
    << " speedup=" << std::setw(8) << std::fixed << std::setprecision(3) << result.speedup_vs_heap
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

  out << "scenario,mode,allocation_size,alignment,count,iterations,best_ms,avg_ms,throughput_ops_per_sec,speedup_vs_heap\n";
  out << std::fixed << std::setprecision(6);

  for (const auto& result : results)
  {
    out
      << result.scenario << ','
      << result.mode << ','
      << result.allocation_size << ','
      << result.alignment << ','
      << result.count << ','
      << result.iterations << ','
      << result.best_ms << ','
      << result.avg_ms << ','
      << result.throughput_ops_per_sec << ','
      << result.speedup_vs_heap << '\n';
  }
}

void run_one_scenario(std::vector<Result>& results, const Config& config)
{
  Result heap_result = benchmark_mode("heap", config);
  Result arena_result = benchmark_mode("arena", config);

  heap_result.speedup_vs_heap = 1.0;
  arena_result.speedup_vs_heap = heap_result.best_ms / arena_result.best_ms;

  results.push_back(heap_result);
  results.push_back(arena_result);

  print_result(heap_result);
  print_result(arena_result);
}

void append_size_scenarios(std::vector<Result>& results)
{
  const std::vector<std::size_t> sizes = {8, 16, 32, 64, 128, 256, 512};

  for (std::size_t size : sizes)
  {
    Config config;
    config.scenario = Scenario::AllocOnly;
    config.allocation_size = size;
    config.alignment = 16;
    config.count = 100000;
    config.iterations = 5;
    run_one_scenario(results, config);
  }
}

void append_alignment_scenarios(std::vector<Result>& results)
{
  const std::vector<std::size_t> alignments = {1, 2, 4, 8, 16, 32, 64};

  for (std::size_t alignment : alignments)
  {
    Config config;
    config.scenario = Scenario::AllocOnly;
    config.allocation_size = 64;
    config.alignment = alignment;
    config.count = 100000;
    config.iterations = 5;
    run_one_scenario(results, config);
  }
}

void append_pattern_scenarios(std::vector<Result>& results)
{
  for (Scenario scenario : {Scenario::AllocOnly, Scenario::AllocReset, Scenario::MarkRewind})
  {
    Config config;
    config.scenario = scenario;
    config.allocation_size = 64;
    config.alignment = 16;
    config.count = 100000;
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
      if (options.preset == "sizes" || options.preset == "all")
      {
        append_size_scenarios(results);
      }

      if (options.preset == "alignments" || options.preset == "all")
      {
        append_alignment_scenarios(results);
      }

      if (options.preset == "patterns" || options.preset == "all")
      {
        append_pattern_scenarios(results);
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
