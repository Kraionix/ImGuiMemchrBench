#include <cstdlib>

#include <string>
#include <string_view>
#include <functional>
#include <filesystem>
#include <ranges>
#include <charconv>
#include <random>
#include <memory>
#include <memory_resource>
#include <codecvt>
#include <algorithm>
#include <execution>
#include <span>

#define FMT_STATIC
#define FMT_UNICODE 0
#include <fmt/base.h>
#include <fmt/std.h>
#include <fmt/color.h>

#define BENCHMARK_STATIC_DEFINE
#include <benchmark/benchmark.h>

#include "bench-config.h"

enum class States : int
{
  NONE = 0,
  HELP,
  EXIT,
  SHOW_CONFIG,
  SHOW_SCHEMA,
  RELOAD,
  _COUNT
};

class StateMachine 
{
public:
  StateMachine() 
    : current_state(States::NONE) {}

  void run()
  {
    std::string input;

    while (true) 
    {
      fmt::print("Input Command ('help' for help): ");
      std::getline(std::cin, input);

      current_state = parseInput(input);
      handleState();
    }
  }

private:
  States parseInput(std::string_view input)
  {
    for (size_t i = 0; i < commands.size(); ++i)
    {
      if (input == commands[i])
        return static_cast<States>(i);
    }
    return States::NONE;
  }

  void handleState()
  {
    switch (current_state)
    {
    case States::SHOW_CONFIG:
      showConfig();
      break;
    case States::SHOW_SCHEMA:
      showSchema();
      break;
    case States::RELOAD:
      reloadConfig();
      break;
    case States::HELP:
      printHelp();
      break;
    case States::EXIT:
      std::exit(0);
      break;
    case States::NONE:
    default:
      fmt::println("Unknown command. Repeat the entry.");
      break;
    }
  }

  void printHelp()
  {
    fmt::println("Commands:");
    for (size_t i = 0; i < commands.size(); i++)
    {
      fmt::println("{}", commands[i]);
    }
  }

  void showConfig() 
  {
    //benchcfg::BenchConfig config{};
    fmt::println("Config: {}", defaultConfig);
  }

  void showSchema() 
  {
    std::string schema = rfl::json::to_schema<benchcfg::BenchConfig>(YYJSON_WRITE_PRETTY_TWO_SPACES);
    fmt::println("Schema: {}", schema);
  }

  void reloadConfig()
  {
    fs::path cfg_path = fs::current_path() / "bench_config.json";
    fs::path schema_path = fs::current_path() / "bench_config_schema.json";

    //benchcfg::BenchConfig config{};
    std::string schema = rfl::json::to_schema<benchcfg::BenchConfig>(YYJSON_WRITE_PRETTY_TWO_SPACES);

    benchcfg::Json::write<benchcfg::BenchConfig, rfl::NoOptionals>(cfg_path, defaultConfig);

    std::ofstream file(schema_path, std::ios::out | std::ios::trunc);
    file << schema;

    fmt::println("Successful config update");
  }

private:
  inline static const std::array<std::string_view, static_cast<size_t>(States::_COUNT)> commands =
  {
    "none",        // States::NONE
    "help",         // States::HELP
    "exit",        // States::EXIT
    "show_config", // States::SHOW_CONFIG
    "show_schema", // States::SHOW_SCHEMA
    "reload"       // States::RELOAD
  };

  inline static const benchcfg::BenchConfig defaultConfig = {
    //.args = std::vector<benchcfg::BenchConfig::Arg>{ {.name ="line_length", .value = 131 } },
    //.args = std::vector<int64_t>{ 131 },
    .time_unit = benchmark::kMillisecond,
    .value_range = benchcfg::BenchConfig::ValueRange{ .start = 1 << 20, .limit = 1 << 30 }
  };

private:
  States current_state;
};

int main()
{
  StateMachine stateChecker;
  stateChecker.run();
}

//int main()
//{
//  fs::path cfg_path = fs::current_path() / "bench_config.json";
//  fs::path schema_path = fs::current_path() / "bench_config_schema.json";
//
//  benchcfg::ConfigLoader config_loader(cfg_path);
//  benchcfg::BenchConfig config = config_loader.getConfig();
//  std::string schema = rfl::json::to_schema<benchcfg::BenchConfig>(YYJSON_WRITE_PRETTY_TWO_SPACES);
//
//  fmt::println("Config: {}", config);
//  fmt::println("Schema: {}", schema);
//
//  benchcfg::Json::write(cfg_path, config);
//  benchcfg::Json::write(schema_path, schema);
//
//  system("pause");
//  return 0;
//}
