#pragma once

#include <filesystem>
#include <string>
#include <functional>
#include <expected>
#include <system_error>
#include <iostream>
#include <chrono>
#include <type_traits>
#include <system_error>
#include <span>
#include <thread>
#include <algorithm>
#include <ranges>
#include <functional>
#include <concepts>

#define FMT_STATIC
#define FMT_UNICODE 0
#include <fmt/base.h>
#include <fmt/std.h>
#include <fmt/color.h>

#include <rfl.hpp>
#include <rfl/json.hpp>

#define BENCHMARK_STATIC_DEFINE
#include <benchmark/benchmark.h>

namespace fs = std::filesystem;

#pragma region BENCHCFG_MACROS

#define BENCHCFG_RETURN_EXPECTED(error_code, ...) \
return std::unexpected(fmt_errc_runtime_error(std::make_error_code(error_code), __VA_ARGS__));

#define BENCHCFG_RETURN_EXPECTED_COND(cond, error_code, ...) \
if(cond) { BENCHCFG_RETURN_EXPECTED(error_code, __VA_ARGS__) }

#define BENCHMARK_FROM_CONFIG(CONFIG)                                   		 \
  BENCHMARK_PRIVATE_DECLARE(_benchmark_) =                           				 \
      (::benchmark::internal::RegisterBenchmarkInternal(             				 \
						benchcfg::from_config(CONFIG)));	 

#define BENCHMARK_FROM_CONFIG_LOADER(CONFIG_LOADER, FUNCTION, NAME)														 \
  BENCHMARK_PRIVATE_DECLARE(_benchmark_) =                           													 \
      (::benchmark::internal::RegisterBenchmarkInternal(             													 \
				   benchcfg::from_config(																															 \
					   benchcfg::setConfigName(CONFIG_LOADER.getConfig(), NAME, FUNCTION))));	 					 \

//#define BENCHMARK_FROM_CONFIG_LOADER_2(CONFIG_LOADER, FUNCTION)		\
//BENCHMARK_FROM_CONFIG_LOADER(CONFIG_LOADER, FUNCTION, #FUNCTION)

#define BENCHCFG_WRAP(...) __VA_ARGS__

#define BENCHCFG_FIELD(NAME, DESCRIPTION, TYPE, DEFAULT_VALUE) \
rfl::Field<#NAME, rfl::Description<DESCRIPTION, TYPE>> NAME = DEFAULT_VALUE;

//BENCHCFG_FIELD(
//	 /*Name =*/ ...,
//	 /*Description =*/...,
//	 /*Type =*/ ...,
//	 /*Default value =*/ ...)

#pragma endregion 
// BENCHCFG_MACROS

// Config utilities
namespace benchcfg
{
	class fmt_runtime_error : public std::runtime_error
	{
	public:
		using base_type = std::runtime_error;

		template<fmt::formattable... Args>
		fmt_runtime_error(fmt::format_string<Args...> format, Args&&... args)
			: base_type(fmt::format(format, std::forward<Args>(args)...)) {
		}

		fmt_runtime_error(const std::string_view message) : base_type(message.data()) {}
	};

	class fmt_errc_runtime_error : public fmt_runtime_error
	{
	public:
		using base_type = fmt_runtime_error;

		template<fmt::formattable... Args>
		fmt_errc_runtime_error(std::error_code error_code, fmt::format_string<Args...> format, Args&&... args)
			:
			base_type(fmt::format(format, std::forward<Args>(args)...)),
			error_code(error_code) {
		}

		fmt_errc_runtime_error(const std::string_view message) : base_type(message.data()) {}

		std::error_code errc() const
		{
			return error_code;
		}

	private:
		std::error_code error_code;
	};

	class Json
	{
	public:
		template <typename T>
		[[nodiscard]] static auto read(const fs::path& path) -> std::expected<T, fmt_errc_runtime_error>
		{
			BENCHCFG_RETURN_EXPECTED_COND(!fs::exists(path), std::errc::no_such_file_or_directory, "File does not exist: {}", path)

			std::ifstream file(path, std::ios::in);

			BENCHCFG_RETURN_EXPECTED_COND(!file.is_open() || !file.good(), std::errc::bad_file_descriptor, "Failed to open file for reading: {}", path)

			try
			{
				std::stringstream file_data;
				file_data << file.rdbuf();

				auto read_result = rfl::json::read<T, rfl::DefaultIfMissing>(file_data);

				BENCHCFG_RETURN_EXPECTED_COND(!read_result, std::errc::bad_message, "Exception during JSON reading: {}", read_result.error().value().what())

				return read_result.value();
			}
			catch (const std::exception& ex)
			{
				BENCHCFG_RETURN_EXPECTED(std::errc::bad_message, "Exception during JSON reading: {}", ex.what())
			}
			catch (...)
			{
				BENCHCFG_RETURN_EXPECTED(std::errc::bad_message, "Unknown error occurred during JSON reading")
			}
		}

		template <typename T, typename Processor = rfl::Processors<>>
		[[nodiscard]] static auto write(const fs::path& path, const T& data, yyjson_write_flag write_flag = YYJSON_WRITE_PRETTY_TWO_SPACES) -> std::expected<void, fmt_errc_runtime_error>
		{
			std::ofstream file(path, std::ios::out | std::ios::trunc);

			BENCHCFG_RETURN_EXPECTED_COND(!file.is_open() || !file.good(), std::errc::bad_file_descriptor, "Failed to open file for writing: {}", path)

			try
			{
				file << rfl::json::write<Processor>(data, write_flag);

				BENCHCFG_RETURN_EXPECTED_COND(!file.good(), std::errc::bad_file_descriptor, "Failed to write data to file: {}", path)

				return {};
			}
			catch (const std::exception& ex)
			{
				BENCHCFG_RETURN_EXPECTED(std::errc::bad_message, "Exception during JSON writing: {}", ex.what())
			}
			catch (...)
			{
				BENCHCFG_RETURN_EXPECTED(std::errc::bad_message, "Unknown error occurred during JSON writing")
			}
		}
	};
}

// Config
namespace benchcfg
{
	struct BenchConfig
	{
		//struct Arg
		//{
		//	std::string name;
		//	int64_t value;
		//};

		struct ValueRange
		{
			int64_t start;
			int64_t limit;
			std::optional<int> step = std::nullopt;
		};

		struct ThreadRange
		{
			int min_threads;
			int max_threads;
			std::optional<int> stride = std::nullopt;
		};

		//BENCHCFG_FIELD(
		//	args,
		//	"Benchmark function args",
		//	std::optional<std::vector<Arg>>,
		//	std::nullopt)

		//BENCHCFG_FIELD(
		//	args,
		//	"Benchmark function args",
		//	std::optional<std::vector<int64_t>>,
		//	std::nullopt)

		BENCHCFG_FIELD(
			name,
			"Benchmark name",
			std::optional<std::string>,
			std::nullopt)

		BENCHCFG_FIELD(
			function,
			"Pointer to benchmark function",
			rfl::Skip<uintptr_t>,
			(uintptr_t)nullptr)

		BENCHCFG_FIELD(
			time_unit,
			"Time unit used for output reports",
			std::optional<benchmark::TimeUnit>,
			std::nullopt)

		BENCHCFG_FIELD(
			value_range,
			"Start and end values for benchmark range",
			ValueRange,
			ValueRange{})
		
		BENCHCFG_FIELD(
			value_range_list,
			"Start and end values for multiple benchmark parameter ranges",
			BENCHCFG_WRAP(std::optional<std::vector<std::pair<int64_t, int64_t>>>),
			std::nullopt)

		BENCHCFG_FIELD(
			range_multiplier,
			"Multiplier for range values",
			std::optional<int>,
			std::nullopt)

		BENCHCFG_FIELD(
			min_time,
			"Minimum benchmark execution time",
			std::optional<double>,
			std::nullopt)

		BENCHCFG_FIELD(
			min_warmup_time,
			"Minimum warmup time before benchmarking",
			std::optional<double>,
			std::nullopt)

		BENCHCFG_FIELD(
			iterations,
			"Exact number of iterations to run",
			std::optional<benchmark::IterationCount>,
			std::nullopt)

		BENCHCFG_FIELD(
			repetitions,
			"Number of times to repeat the benchmark",
			std::optional<int>, 
			std::nullopt)

		BENCHCFG_FIELD(
			report_aggregates_only,
			"Whether to report only aggregated results",
			std::optional<bool>,
			std::nullopt)

		BENCHCFG_FIELD(
			display_aggregates_only, 
			"Whether to display only aggregated results",
			std::optional<bool>,
			std::nullopt)

		BENCHCFG_FIELD(
			complexity,
			"Asymptotic computational complexity",
			std::optional<benchmark::BigO>, 
			std::nullopt)

		BENCHCFG_FIELD(
			threads, 
			"Number of concurrent threads",
			std::optional<int>,
			std::nullopt)

		BENCHCFG_FIELD(
			thread_range,
			"Range of concurrent threads",
			std::optional<ThreadRange>,
			std::nullopt)
	};
}

//// Config reflector
//namespace rfl
//{
//	template <>
//	struct Reflector<benchmark::internal::Benchmark>
//	{
//		using BaseType = benchmark::internal::Benchmark;
//		using ReflType = benchcfg::BenchConfig;
//
//		static BaseType* to(const ReflType& config) noexcept
//		{
//			//BaseType* base = new benchmark::internal::FunctionBenchmark(config.name, config.function.value().target<benchmark::internal::Function>());
//			BaseType* base = new benchmark::internal::FunctionBenchmark(config.name.get().get().value(), reinterpret_cast<benchmark::internal::Function*>(config.function.get().get().value()));
//
//			if (config.time_unit.get().get().has_value())
//				base->Unit(config.time_unit.get().get().value());
//
//			{
//				auto& value_range = config.value_range.get().get();
//
//				if (value_range.step.has_value())
//					base->DenseRange(value_range.start, value_range.limit, value_range.step.value());
//				else
//					base->Range(value_range.start, value_range.limit);
//			}
//
//			if (config.value_ranges.get().get().has_value())
//				base->Ranges(config.value_ranges.get().get().value());
//
//			if (config.range_multiplier.get().get().has_value())
//				base->RangeMultiplier(config.range_multiplier.get().get().value());
//
//			if (config.min_time.get().get().has_value())
//				base->MinTime(config.min_time.get().get().value());
//
//			if (config.min_warmup_time.get().get().has_value())
//				base->MinWarmUpTime(config.min_warmup_time.get().get().value());
//
//			if (config.iterations.get().get().has_value())
//				base->Iterations(config.iterations.get().get().value());
//
//			if (config.repetitions.get().get().has_value())
//				base->Repetitions(config.repetitions.get().get().value());
//
//			if (config.report_aggregates_only.get().get().has_value())
//				base->ReportAggregatesOnly(config.report_aggregates_only.get().get().value());
//
//			if (config.display_aggregates_only.get().get().has_value())
//				base->DisplayAggregatesOnly(config.display_aggregates_only.get().get().value());
//
//			if (config.complexity.get().get().has_value())
//				base->Complexity(config.complexity.get().get().value());
//
//			if (config.threads.get().get().has_value())
//				base->Threads(config.threads.get().get().value());
//
//			if (config.thread_range.get().get().has_value())
//			{
//				auto& thread_range = config.thread_range.get().get().value();
//
//				if (thread_range.stride.has_value())
//					base->DenseThreadRange(thread_range.min_threads, thread_range.max_threads, thread_range.stride.value());
//				else
//					base->ThreadRange(thread_range.min_threads, thread_range.max_threads);
//			}
//
//
//			return base;
//		}
//
//		//static ReflType from(const BaseType& v)
//		//{
//		//	return {};
//		//}
//	};
//}

// Config reflector
namespace rfl
{
	template <>
	struct Reflector<benchmark::internal::Benchmark>
	{
		using BaseType = benchmark::internal::Benchmark;
		using ReflType = benchcfg::BenchConfig;

		static BaseType* to(const ReflType& config) noexcept
		{
			//auto& args = config.args.get().get();
			auto& name = config.name.get().get();
			auto& function = config.function.get().get();
			auto& time_unit = config.time_unit.get().get();
			auto& value_range = config.value_range.get().get();
			auto& value_range_list = config.value_range_list.get().get();
			auto& range_multiplier = config.range_multiplier.get().get();
			auto& min_time = config.min_time.get().get();
			auto& min_warmup_time = config.min_warmup_time.get().get();
			auto& iterations = config.iterations.get().get();
			auto& repetitions = config.repetitions.get().get();
			auto& report_aggregates_only = config.report_aggregates_only.get().get();
			auto& display_aggregates_only = config.display_aggregates_only.get().get();
			auto& complexity = config.complexity.get().get();
			auto& threads = config.threads.get().get();
			auto& thread_range = config.thread_range.get().get();

			BaseType* base = new benchmark::internal::FunctionBenchmark(name.value(), reinterpret_cast<benchmark::internal::Function*>(function.value()));

			//if (args.has_value())
			//	base->Args(args.value());

			//if (args.has_value())
			//{
			//	auto& args_value = args.value();
			//
			//	//for (size_t i = 0; i < args_value.size(); i++)
			//	//	fmt::println("name {}, value {}", args_value[i].name, args_value[i].value);
			//
			//	auto values = args_value
			//		| std::views::transform(&benchcfg::BenchConfig::Arg::value) 
			//		| std::ranges::to<std::vector<int64_t>>();
			//
			//	auto names = args_value 
			//		| std::views::transform(&benchcfg::BenchConfig::Arg::name) 
			//		| std::ranges::to<std::vector<std::string>>();
			//
			//	base->Args(values);
			//	base->ArgNames(names);
			//}

			//if (args.has_value())
			//{
			//	auto& args_value = args.value();
			//
			//	std::vector<int64_t> values(args_value.size());
			//	std::vector<std::string> names(args_value.size());
			//
			//	for (auto [idx, arg] : std::views::enumerate(args_value))
			//	{
			//		values[idx] = arg.value;
			//
			//		if (arg.name.has_value())
			//			names[idx] = arg.name.value();					
			//	}
			//
			//
			//	base->Args(values);
			//	base->ArgNames(names);
			//}

			if (time_unit.has_value())
				base->Unit(time_unit.value());

			if (value_range.step.has_value())
				base->DenseRange(value_range.start, value_range.limit, value_range.step.value());
			else
				base->Range(value_range.start, value_range.limit);

			if (value_range_list.has_value())
				base->Ranges(value_range_list.value());

			if (range_multiplier.has_value())
				base->RangeMultiplier(range_multiplier.value());

			if (min_time.has_value())
				base->MinTime(min_time.value());

			if (min_warmup_time.has_value())
				base->MinWarmUpTime(min_warmup_time.value());

			if (iterations.has_value())
				base->Iterations(iterations.value());

			if (repetitions.has_value())
				base->Repetitions(repetitions.value());

			if (report_aggregates_only.has_value())
				base->ReportAggregatesOnly(report_aggregates_only.value());

			if (display_aggregates_only.has_value())
				base->DisplayAggregatesOnly(display_aggregates_only.value());

			if (complexity.has_value())
				base->Complexity(complexity.value());

			if (threads.has_value())
				base->Threads(threads.value());

			if (thread_range.has_value())
			{
				if (thread_range.value().stride.has_value())
					base->DenseThreadRange(thread_range.value().min_threads, thread_range.value().max_threads, thread_range.value().stride.value());
				else
					base->ThreadRange(thread_range.value().min_threads, thread_range.value().max_threads);
			}

			return base;
		}

		//static ReflType from(const BaseType& v)
		//{
		//	return {};
		//}
	};
}


// Config formatter
namespace fmt
{
	template <>
	struct formatter<benchcfg::BenchConfig>
	{
		constexpr auto parse(format_parse_context& ctx) -> format_parse_context::iterator
		{
			return ctx.end();
		}

		auto format(const benchcfg::BenchConfig config, format_context& ctx) const -> format_context::iterator
		{
			std::string json = rfl::json::write<rfl::NoOptionals>(config, YYJSON_WRITE_PRETTY_TWO_SPACES);
			return fmt::format_to(ctx.out(), "{}", json);
		}
	};
}

// Config loader helper
namespace benchcfg
{
	class ConfigLoader
	{
	public:
		ConfigLoader(const fs::path& path) : path(path)
		{
			auto result = Json::read<BenchConfig>(path);

			if (result.has_value())
			{
				config = std::move(result.value());
			}
			else
			{
				fmt::println("Error: {}. Code: {}", result.error().what(), result.error().errc().message());
				system("pause");
				std::terminate();
			}
		}

		BenchConfig getConfig() const
		{
			return config;
		}

	private:
		fs::path path;
		BenchConfig config;
	};

	auto from_config(const BenchConfig& config) -> benchmark::internal::Benchmark*
	{
		return rfl::Reflector<benchmark::internal::Benchmark>::to(config);
	}

	BenchConfig setConfigName(BenchConfig config, const std::string& name, benchmark::internal::Function* function)
	{
		config.name.set(name);
		config.function = (uintptr_t)function;

		return config;
	}
}