#include "fiber_job_system.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
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
enum class Mode
{
    NoYield,
    YieldOnce,
    YieldMany
};

struct Config
{
    std::size_t workers = 4;
    std::size_t fibers = 10000;
    std::size_t work_units = 1000;
    std::size_t yield_count = 10;
    std::size_t iterations = 5;
};

struct Result
{
    std::string mode;
    std::size_t workers = 0;
    std::size_t fibers = 0;
    std::size_t work_units = 0;
    std::size_t yield_count = 0;
    std::size_t iterations = 0;
    double best_ms = 0.0;
    double avg_ms = 0.0;
    double throughput_fibers_per_sec = 0.0;
    double avg_ns_per_fiber = 0.0;
};

struct Options
{
    bool sweep = false;
    std::string preset = "all";
    std::string csv_out;
    Mode mode = Mode::NoYield;
    Config config;
};

void print_usage(const char* program)
{
    std::cout
        << "Usage: " << program << " [options]\n\n"
        << "Single-run options:\n"
        << "  --mode NAME       no_yield | yield_once | yield_many (default: no_yield)\n"
        << "  --workers N       Number of worker threads (default: 4)\n"
        << "  --fibers N        Total number of submitted fibers (default: 10000)\n"
        << "  --work N          CPU work units per phase (default: 1000)\n"
        << "  --yields N        Yields per fiber in yield_many mode (default: 10)\n"
        << "  --iterations N    Number of repeated runs (default: 5)\n\n"
        << "Sweep options:\n"
        << "  --sweep           Run a benchmark matrix instead of one scenario\n"
        << "  --preset NAME     workers | granularity | yield_cost | all (default: all)\n"
        << "  --csv-out PATH    Write CSV results to PATH\n\n"
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

Mode parse_mode(std::string_view text)
{
    if (text == "no_yield") return Mode::NoYield;
    if (text == "yield_once") return Mode::YieldOnce;
    if (text == "yield_many") return Mode::YieldMany;
    throw std::invalid_argument("Invalid mode. Expected no_yield, yield_once, or yield_many");
}

std::string mode_name(Mode mode)
{
    switch (mode)
    {
        case Mode::NoYield: return "no_yield";
        case Mode::YieldOnce: return "yield_once";
        case Mode::YieldMany: return "yield_many";
    }
    return "unknown";
}

void validate_config(const Config& config)
{
    if (config.workers == 0) throw std::invalid_argument("--workers must be greater than 0");
    if (config.fibers == 0) throw std::invalid_argument("--fibers must be greater than 0");
    if (config.work_units == 0) throw std::invalid_argument("--work must be greater than 0");
    if (config.yield_count == 0) throw std::invalid_argument("--yields must be greater than 0");
    if (config.iterations == 0) throw std::invalid_argument("--iterations must be greater than 0");
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

        if (arg == "--mode")
            options.mode = parse_mode(require_value("--mode"));
        else if (arg == "--workers")
            options.config.workers = parse_size(require_value("--workers"), "--workers");
        else if (arg == "--fibers")
            options.config.fibers = parse_size(require_value("--fibers"), "--fibers");
        else if (arg == "--work")
            options.config.work_units = parse_size(require_value("--work"), "--work");
        else if (arg == "--yields")
            options.config.yield_count = parse_size(require_value("--yields"), "--yields");
        else if (arg == "--iterations")
            options.config.iterations = parse_size(require_value("--iterations"), "--iterations");
        else if (arg == "--sweep")
            options.sweep = true;
        else if (arg == "--preset")
            options.preset = std::string(require_value("--preset"));
        else if (arg == "--csv-out")
            options.csv_out = std::string(require_value("--csv-out"));
        else if (arg == "--help")
        {
            print_usage(argv[0]);
            std::exit(0);
        }
        else
            throw std::invalid_argument("Unknown argument: " + arg);
    }

    if (
        options.preset != "workers" &&
        options.preset != "granularity" &&
        options.preset != "yield_cost" &&
        options.preset != "all")
    {
        throw std::invalid_argument("Invalid preset. Expected workers, granularity, yield_cost, or all");
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

double run_benchmark(const Config& config, Mode mode)
{
    return time_ms([&] {
        FiberJobSystem scheduler(config.workers);

        std::atomic<std::size_t> completed{0};
        std::mutex completion_mutex;
        std::condition_variable completion_cv;

        for (std::size_t i = 0; i < config.fibers; ++i)
        {
            scheduler.submit([&, mode] {
                switch (mode)
                {
                    case Mode::NoYield:
                    {
                        do_spin_work(config.work_units);
                        break;
                    }

                    case Mode::YieldOnce:
                    {
                        do_spin_work(config.work_units);
                        scheduler.yield_current();
                        do_spin_work(config.work_units);
                        break;
                    }

                    case Mode::YieldMany:
                    {
                        for (std::size_t y = 0; y < config.yield_count; ++y)
                        {
                            do_spin_work(config.work_units);
                            scheduler.yield_current();
                        }
                        do_spin_work(config.work_units);
                        break;
                    }
                }

                const std::size_t done = completed.fetch_add(1, std::memory_order_acq_rel) + 1;
                if (done == config.fibers)
                {
                    std::lock_guard<std::mutex> lock(completion_mutex);
                    completion_cv.notify_one();
                }
            });
        }

        while (completed.load(std::memory_order_acquire) < config.fibers)
        {
            scheduler.resume_all();
            std::this_thread::sleep_for(std::chrono::microseconds(50));
        }

        std::unique_lock<std::mutex> lock(completion_mutex);
        completion_cv.wait(lock, [&] {
            return completed.load(std::memory_order_acquire) == config.fibers;
        });
    });
}

Result benchmark_mode(const Config& config, Mode mode)
{
    std::vector<double> samples;
    samples.reserve(config.iterations);

    for (std::size_t it = 0; it < config.iterations; ++it)
    {
        samples.push_back(run_benchmark(config, mode));
    }

    const double best_ms = *std::min_element(samples.begin(), samples.end());
    double avg_ms = 0.0;
    for (double s : samples) avg_ms += s;
    avg_ms /= static_cast<double>(samples.size());

    Result r;
    r.mode = mode_name(mode);
    r.workers = config.workers;
    r.fibers = config.fibers;
    r.work_units = config.work_units;
    r.yield_count = config.yield_count;
    r.iterations = config.iterations;
    r.best_ms = best_ms;
    r.avg_ms = avg_ms;
    r.throughput_fibers_per_sec = (static_cast<double>(config.fibers) * 1000.0) / best_ms;
    r.avg_ns_per_fiber = (best_ms * 1000000.0) / static_cast<double>(config.fibers);
    return r;
}

void print_result(const Result& result)
{
    std::cout
        << std::left
        << std::setw(14) << result.mode
        << " workers=" << std::setw(3) << result.workers
        << " fibers=" << std::setw(8) << result.fibers
        << " work=" << std::setw(8) << result.work_units
        << " yields=" << std::setw(6) << result.yield_count
        << " best_ms=" << std::setw(10) << std::fixed << std::setprecision(3) << result.best_ms
        << " avg_ms=" << std::setw(10) << std::fixed << std::setprecision(3) << result.avg_ms
        << " throughput=" << std::setw(12) << std::fixed << std::setprecision(1) << result.throughput_fibers_per_sec
        << " ns/fiber=" << std::setw(12) << std::fixed << std::setprecision(1) << result.avg_ns_per_fiber
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

    out << "mode,workers,fibers,work_units,yield_count,iterations,best_ms,avg_ms,throughput_fibers_per_sec,avg_ns_per_fiber\n";
    out << std::fixed << std::setprecision(6);

    for (const auto& result : results)
    {
        out
            << result.mode << ','
            << result.workers << ','
            << result.fibers << ','
            << result.work_units << ','
            << result.yield_count << ','
            << result.iterations << ','
            << result.best_ms << ','
            << result.avg_ms << ','
            << result.throughput_fibers_per_sec << ','
            << result.avg_ns_per_fiber << '\n';
    }
}

void run_config(std::vector<Result>& results, const Config& config, const std::vector<Mode>& modes)
{
    for (Mode mode : modes)
    {
        const Result result = benchmark_mode(config, mode);
        results.push_back(result);
        print_result(result);
    }
}

void append_worker_scenarios(std::vector<Result>& results)
{
    const std::vector<std::size_t> workers = {1, 2, 4, 8};

    for (std::size_t worker_count : workers)
    {
        Config config;
        config.workers = worker_count;
        config.fibers = 10000;
        config.work_units = 1000;
        config.yield_count = 10;
        config.iterations = 5;
        run_config(results, config, {Mode::NoYield, Mode::YieldOnce});
    }
}

void append_granularity_scenarios(std::vector<Result>& results)
{
    const std::vector<std::size_t> work_units = {10, 100, 1000, 10000, 100000};

    for (std::size_t work : work_units)
    {
        Config config;
        config.workers = 4;
        config.fibers = 10000;
        config.work_units = work;
        config.yield_count = 10;
        config.iterations = 5;
        run_config(results, config, {Mode::NoYield, Mode::YieldOnce});
    }
}

void append_yield_cost_scenarios(std::vector<Result>& results)
{
    const std::vector<std::size_t> yield_counts = {1, 2, 5, 10, 20};

    for (std::size_t yields : yield_counts)
    {
        Config config;
        config.workers = 4;
        config.fibers = 5000;
        config.work_units = 1000;
        config.yield_count = yields;
        config.iterations = 5;
        run_config(results, config, {Mode::YieldMany});
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
            if (options.preset == "workers" || options.preset == "all")
            {
                append_worker_scenarios(results);
            }
            if (options.preset == "granularity" || options.preset == "all")
            {
                append_granularity_scenarios(results);
            }
            if (options.preset == "yield_cost" || options.preset == "all")
            {
                append_yield_cost_scenarios(results);
            }
        }
        else
        {
            run_config(results, options.config, {options.mode});
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
