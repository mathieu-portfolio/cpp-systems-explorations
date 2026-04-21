#include "vector.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace
{
enum class Scenario
{
  Growth,
  ReserveGrowth,
  Copy,
  Move
};

enum class ElementType
{
  Int,
  String,
  Heavy
};

struct Heavy
{
  std::uint64_t a = 0;
  std::uint64_t b = 0;
  std::uint64_t c = 0;
  std::uint64_t d = 0;
  std::string payload;

  Heavy() : payload(64, 'x') {}
  explicit Heavy(std::size_t i)
    : a(i), b(i + 1), c(i + 2), d(i + 3), payload(64, static_cast<char>('a' + (i % 26))) {}
};

struct Config
{
  Scenario scenario = Scenario::Growth;
  ElementType element_type = ElementType::Int;
  std::size_t size = 100000;
  std::size_t iterations = 5;
};

struct Result
{
  std::string scenario;
  std::string mode;
  std::string element_type;
  std::size_t size = 0;
  std::size_t iterations = 0;
  double best_ms = 0.0;
  double avg_ms = 0.0;
  double throughput_ops_per_sec = 0.0;
  double speedup_vs_std = 0.0;
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
    << "  --scenario NAME   growth | reserve_growth | copy | move (default: growth)\n"
    << "  --type NAME       int | string | heavy (default: int)\n"
    << "  --size N          Number of elements (default: 100000)\n"
    << "  --iterations N    Number of repeated runs (default: 5)\n"
    << '\n'
    << "Sweep options:\n"
    << "  --sweep           Run a benchmark matrix instead of a single scenario\n"
    << "  --preset NAME     growth | copy_move | types | all (default: all)\n"
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
  if (text == "growth") return Scenario::Growth;
  if (text == "reserve_growth") return Scenario::ReserveGrowth;
  if (text == "copy") return Scenario::Copy;
  if (text == "move") return Scenario::Move;
  throw std::invalid_argument("Invalid scenario. Expected growth, reserve_growth, copy, or move");
}

ElementType parse_element_type(std::string_view text)
{
  if (text == "int") return ElementType::Int;
  if (text == "string") return ElementType::String;
  if (text == "heavy") return ElementType::Heavy;
  throw std::invalid_argument("Invalid type. Expected int, string, or heavy");
}

std::string scenario_name(Scenario scenario)
{
  switch (scenario)
  {
    case Scenario::Growth: return "growth";
    case Scenario::ReserveGrowth: return "reserve_growth";
    case Scenario::Copy: return "copy";
    case Scenario::Move: return "move";
  }

  return "unknown";
}

std::string element_type_name(ElementType type)
{
  switch (type)
  {
    case ElementType::Int: return "int";
    case ElementType::String: return "string";
    case ElementType::Heavy: return "heavy";
  }

  return "unknown";
}

void validate_config(const Config& config)
{
  if (config.size == 0)
  {
    throw std::invalid_argument("--size must be greater than 0");
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

    if (arg == "--scenario")
    {
      options.config.scenario = parse_scenario(require_value("--scenario"));
    }
    else if (arg == "--type")
    {
      options.config.element_type = parse_element_type(require_value("--type"));
    }
    else if (arg == "--size")
    {
      options.config.size = parse_size(require_value("--size"), "--size");
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
    options.preset != "growth" &&
    options.preset != "copy_move" &&
    options.preset != "types" &&
    options.preset != "all")
  {
    throw std::invalid_argument("Invalid preset. Expected growth, copy_move, types, or all");
  }

  validate_config(options.config);
  return options;
}

template <typename T>
T make_value(std::size_t i);

template <>
int make_value<int>(std::size_t i)
{
  return static_cast<int>(i);
}

template <>
std::string make_value<std::string>(std::size_t i)
{
  return "value_" + std::to_string(i);
}

template <>
Heavy make_value<Heavy>(std::size_t i)
{
  return Heavy(i);
}

template <typename Fn>
double time_ms(Fn&& fn)
{
  const auto start = std::chrono::steady_clock::now();
  fn();
  const auto end = std::chrono::steady_clock::now();
  return std::chrono::duration<double, std::milli>(end - start).count();
}

template <template <typename> class Vec, typename T>
void execute_scenario(Scenario scenario, std::size_t size)
{
  if (scenario == Scenario::Growth)
  {
    Vec<T> v;
    for (std::size_t i = 0; i < size; ++i)
    {
      v.push_back(make_value<T>(i));
    }
  }
  else if (scenario == Scenario::ReserveGrowth)
  {
    Vec<T> v;
    v.reserve(size);
    for (std::size_t i = 0; i < size; ++i)
    {
      v.push_back(make_value<T>(i));
    }
  }
  else if (scenario == Scenario::Copy)
  {
    Vec<T> src;
    src.reserve(size);
    for (std::size_t i = 0; i < size; ++i)
    {
      src.push_back(make_value<T>(i));
    }

    Vec<T> copy = src;
    volatile std::size_t sink = copy.size();
    (void)sink;
  }
  else if (scenario == Scenario::Move)
  {
    Vec<T> src;
    src.reserve(size);
    for (std::size_t i = 0; i < size; ++i)
    {
      src.push_back(make_value<T>(i));
    }

    Vec<T> moved = std::move(src);
    volatile std::size_t sink = moved.size();
    (void)sink;
  }
}

template <typename T>
Result benchmark_mode(const std::string& mode, const Config& config)
{
  std::vector<double> samples;
  samples.reserve(config.iterations);

  for (std::size_t iteration = 0; iteration < config.iterations; ++iteration)
  {
    if (mode == "custom_vector")
    {
      samples.push_back(time_ms([&] {
        execute_scenario<Vector, T>(config.scenario, config.size);
      }));
    }
    else if (mode == "std_vector")
    {
      samples.push_back(time_ms([&] {
        execute_scenario<std::vector, T>(config.scenario, config.size);
      }));
    }
    else
    {
      throw std::invalid_argument("Unknown benchmark mode");
    }
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
  result.element_type = element_type_name(config.element_type);
  result.size = config.size;
  result.iterations = config.iterations;
  result.best_ms = best_ms;
  result.avg_ms = avg_ms;
  result.throughput_ops_per_sec = 1000.0 / best_ms;
  return result;
}

Result run_std_result(const Config& config)
{
  switch (config.element_type)
  {
    case ElementType::Int:
      return benchmark_mode<int>("std_vector", config);
    case ElementType::String:
      return benchmark_mode<std::string>("std_vector", config);
    case ElementType::Heavy:
      return benchmark_mode<Heavy>("std_vector", config);
  }

  throw std::invalid_argument("Unsupported element type");
}

Result run_custom_result(const Config& config, double std_best_ms)
{
  Result result;

  switch (config.element_type)
  {
    case ElementType::Int:
      result = benchmark_mode<int>("custom_vector", config);
      break;
    case ElementType::String:
      result = benchmark_mode<std::string>("custom_vector", config);
      break;
    case ElementType::Heavy:
      result = benchmark_mode<Heavy>("custom_vector", config);
      break;
  }

  result.speedup_vs_std = std_best_ms / result.best_ms;
  return result;
}

void print_result(const Result& result)
{
  std::cout
    << std::left
    << std::setw(16) << result.scenario
    << std::setw(14) << result.mode
    << " type=" << std::setw(8) << result.element_type
    << " size=" << std::setw(10) << result.size
    << " best_ms=" << std::setw(10) << std::fixed << std::setprecision(3) << result.best_ms
    << " avg_ms=" << std::setw(10) << std::fixed << std::setprecision(3) << result.avg_ms
    << " throughput=" << std::setw(12) << std::fixed << std::setprecision(1) << result.throughput_ops_per_sec
    << " speedup=" << std::setw(8) << std::fixed << std::setprecision(3) << result.speedup_vs_std
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

  out << "scenario,mode,element_type,size,iterations,best_ms,avg_ms,throughput_ops_per_sec,speedup_vs_std\n";
  out << std::fixed << std::setprecision(6);

  for (const auto& result : results)
  {
    out
      << result.scenario << ','
      << result.mode << ','
      << result.element_type << ','
      << result.size << ','
      << result.iterations << ','
      << result.best_ms << ','
      << result.avg_ms << ','
      << result.throughput_ops_per_sec << ','
      << result.speedup_vs_std << '\n';
  }
}

void run_one_scenario(std::vector<Result>& results, const Config& config)
{
  const Result std_result = run_std_result(config);
  Result custom_result = run_custom_result(config, std_result.best_ms);

  Result std_copy = std_result;
  std_copy.speedup_vs_std = 1.0;

  results.push_back(std_copy);
  results.push_back(custom_result);

  print_result(std_copy);
  print_result(custom_result);
}

void append_growth_scenarios(std::vector<Result>& results)
{
  const std::vector<std::size_t> sizes = {1000, 10000, 100000, 1000000};

  for (Scenario scenario : {Scenario::Growth, Scenario::ReserveGrowth})
  {
    for (std::size_t size : sizes)
    {
      Config config;
      config.scenario = scenario;
      config.element_type = ElementType::Int;
      config.size = size;
      config.iterations = 5;
      run_one_scenario(results, config);
    }
  }
}

void append_copy_move_scenarios(std::vector<Result>& results)
{
  const std::vector<std::size_t> sizes = {1000, 10000, 100000};

  for (Scenario scenario : {Scenario::Copy, Scenario::Move})
  {
    for (std::size_t size : sizes)
    {
      Config config;
      config.scenario = scenario;
      config.element_type = ElementType::Int;
      config.size = size;
      config.iterations = 5;
      run_one_scenario(results, config);
    }
  }
}

void append_type_scenarios(std::vector<Result>& results)
{
  const std::vector<std::size_t> sizes = {1000, 10000, 100000};

  for (ElementType type : {ElementType::Int, ElementType::String, ElementType::Heavy})
  {
    for (std::size_t size : sizes)
    {
      Config config;
      config.scenario = Scenario::ReserveGrowth;
      config.element_type = type;
      config.size = size;
      config.iterations = 5;
      run_one_scenario(results, config);
    }
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
      if (options.preset == "growth" || options.preset == "all")
      {
        append_growth_scenarios(results);
      }

      if (options.preset == "copy_move" || options.preset == "all")
      {
        append_copy_move_scenarios(results);
      }

      if (options.preset == "types" || options.preset == "all")
      {
        append_type_scenarios(results);
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
