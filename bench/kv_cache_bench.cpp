#include "runtime/model.hpp"
#include "runtime/sampling.hpp"
#include "runtime/tokenizer.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <unistd.h>
#endif

#define TINYSERVE_STRINGIFY_IMPL(value) #value
#define TINYSERVE_STRINGIFY(value) TINYSERVE_STRINGIFY_IMPL(value)

namespace {

using Clock = std::chrono::steady_clock;

struct Options {
  std::filesystem::path model;
  std::filesystem::path output;
  std::filesystem::path raw_output;
  std::size_t prompt_tokens = 4;
  std::size_t generated_tokens = 8;
  std::size_t warmups = 3;
  std::size_t repeats = 9;
  std::size_t inner_iterations = 100;
  std::string source_revision = "unversioned-workspace";
  std::string command;
};

struct Sample {
  double ttft_ms = 0.0;
  double decode_ms = 0.0;
  std::uint64_t checksum = 0;
  tinyserve::TokenIds generated;
};

std::size_t parse_size(const std::string& text, const std::string& option) {
  std::size_t consumed = 0;
  const auto value = std::stoull(text, &consumed);
  if (consumed != text.size() || value == 0 || value > std::numeric_limits<std::size_t>::max()) {
    throw std::invalid_argument(option + " must be a positive size_t value");
  }
  return static_cast<std::size_t>(value);
}

Options parse_options(int argc, char** argv) {
  Options options;
  std::ostringstream command;
  for (int index = 0; index < argc; ++index) {
    if (index != 0) {
      command << ' ';
    }
    command << argv[index];
  }
  options.command = command.str();
  for (int index = 1; index < argc; ++index) {
    const std::string argument = argv[index];
    const auto value = [&]() -> std::string {
      if (++index >= argc) {
        throw std::invalid_argument("missing value for " + argument);
      }
      return argv[index];
    };
    if (argument == "--model") {
      options.model = value();
    } else if (argument == "--output") {
      options.output = value();
    } else if (argument == "--raw-output") {
      options.raw_output = value();
    } else if (argument == "--prompt-tokens") {
      options.prompt_tokens = parse_size(value(), argument);
    } else if (argument == "--generated-tokens") {
      options.generated_tokens = parse_size(value(), argument);
    } else if (argument == "--warmups") {
      options.warmups = parse_size(value(), argument);
    } else if (argument == "--repeats") {
      options.repeats = parse_size(value(), argument);
    } else if (argument == "--inner-iterations") {
      options.inner_iterations = parse_size(value(), argument);
    } else if (argument == "--source-revision") {
      options.source_revision = value();
    } else {
      throw std::invalid_argument("unknown benchmark option '" + argument + "'");
    }
  }
  if (options.model.empty() || options.output.empty() || options.raw_output.empty()) {
    throw std::invalid_argument("--model, --output, and --raw-output are required");
  }
  if (options.generated_tokens < 2) {
    throw std::invalid_argument("--generated-tokens must be at least 2 for decode timing");
  }
  return options;
}

tinyserve::TokenIds fixture_prompt(std::size_t count, std::size_t vocab_size) {
  tinyserve::TokenIds tokens;
  tokens.reserve(count);
  for (std::size_t index = 0; index < count; ++index) {
    tokens.push_back(static_cast<tinyserve::TokenId>((65 + index) % vocab_size));
  }
  return tokens;
}

Sample run_once(const tinyserve::Model& model, const tinyserve::TokenIds& prompt,
                std::size_t generated_count, bool cached) {
  Sample sample;
  sample.generated.reserve(generated_count);
  tinyserve::Sampler sampler({});
  tinyserve::TokenIds all_tokens = prompt;
  std::vector<float> logits;
  const auto ttft_start = Clock::now();
  if (cached) {
    auto cache = model.create_kv_cache(prompt.size() + generated_count);
    for (const auto token : prompt) {
      logits = model.next_token_logits_cached(token, cache);
    }
    const auto next = sampler.select(logits);
    sample.generated.push_back(next);
    all_tokens.push_back(next);
    const auto ttft_end = Clock::now();
    const auto decode_start = ttft_end;
    for (std::size_t step = 1; step < generated_count; ++step) {
      logits = model.next_token_logits_cached(all_tokens.back(), cache);
      const auto generated = sampler.select(logits);
      sample.generated.push_back(generated);
      all_tokens.push_back(generated);
    }
    const auto decode_end = Clock::now();
    sample.ttft_ms = std::chrono::duration<double, std::milli>(ttft_end - ttft_start).count();
    sample.decode_ms =
        std::chrono::duration<double, std::milli>(decode_end - decode_start).count();
  } else {
    logits = model.next_token_logits(all_tokens);
    const auto next = sampler.select(logits);
    sample.generated.push_back(next);
    all_tokens.push_back(next);
    const auto ttft_end = Clock::now();
    const auto decode_start = ttft_end;
    for (std::size_t step = 1; step < generated_count; ++step) {
      logits = model.next_token_logits(all_tokens);
      const auto generated = sampler.select(logits);
      sample.generated.push_back(generated);
      all_tokens.push_back(generated);
    }
    const auto decode_end = Clock::now();
    sample.ttft_ms = std::chrono::duration<double, std::milli>(ttft_end - ttft_start).count();
    sample.decode_ms =
        std::chrono::duration<double, std::milli>(decode_end - decode_start).count();
  }
  for (const auto token : sample.generated) {
    sample.checksum = sample.checksum * 1315423911ULL + token;
  }
  return sample;
}

Sample run_batched_sample(const tinyserve::Model& model, const tinyserve::TokenIds& prompt,
                          const Options& options, bool cached) {
  Sample averaged;
  for (std::size_t iteration = 0; iteration < options.inner_iterations; ++iteration) {
    const auto sample = run_once(model, prompt, options.generated_tokens, cached);
    averaged.ttft_ms += sample.ttft_ms;
    averaged.decode_ms += sample.decode_ms;
    averaged.checksum ^= sample.checksum + iteration;
    averaged.generated = sample.generated;
  }
  averaged.ttft_ms /= static_cast<double>(options.inner_iterations);
  averaged.decode_ms /= static_cast<double>(options.inner_iterations);
  return averaged;
}

double median(std::vector<double> values) {
  std::sort(values.begin(), values.end());
  const auto middle = values.size() / 2;
  return values.size() % 2 == 0 ? (values[middle - 1] + values[middle]) / 2.0
                               : values[middle];
}

std::string csv(const std::string& value) {
  std::string escaped = value;
  std::size_t position = 0;
  while ((position = escaped.find('"', position)) != std::string::npos) {
    escaped.insert(position, 1, '"');
    position += 2;
  }
  return '"' + escaped + '"';
}

std::string environment_value(const char* name, const std::string& fallback) {
#if defined(_WIN32)
  char* value = nullptr;
  std::size_t size = 0;
  if (_dupenv_s(&value, &size, name) != 0 || value == nullptr || size <= 1) {
    std::free(value);
    return fallback;
  }
  const std::string result(value);
  std::free(value);
  return result;
#else
  const auto* value = std::getenv(name);
  return value == nullptr || std::string(value).empty() ? fallback : value;
#endif
}

std::string operating_system() {
#if defined(_WIN32)
  return "Windows";
#elif defined(__linux__)
  return "Linux";
#elif defined(__APPLE__)
  return "macOS";
#else
  return "unknown";
#endif
}

std::string physical_ram_bytes() {
#if defined(_WIN32)
  MEMORYSTATUSEX status{};
  status.dwLength = sizeof(status);
  if (GlobalMemoryStatusEx(&status) != 0) {
    return std::to_string(status.ullTotalPhys);
  }
#elif defined(_SC_PHYS_PAGES) && defined(_SC_PAGE_SIZE)
  const auto pages = sysconf(_SC_PHYS_PAGES);
  const auto page_size = sysconf(_SC_PAGE_SIZE);
  if (pages > 0 && page_size > 0) {
    return std::to_string(static_cast<unsigned long long>(pages) *
                          static_cast<unsigned long long>(page_size));
  }
#endif
  return "not-reported";
}

std::string compiler() {
#if defined(_MSC_VER)
  return "MSVC " TINYSERVE_STRINGIFY(_MSC_FULL_VER);
#elif defined(__clang__)
  return "Clang " __clang_version__;
#elif defined(__GNUC__)
  return "GCC " __VERSION__;
#else
  return "unknown";
#endif
}

std::string timestamp_utc() {
  const auto now = std::chrono::system_clock::now();
  const auto time = std::chrono::system_clock::to_time_t(now);
  std::tm utc{};
#if defined(_WIN32)
  gmtime_s(&utc, &time);
#else
  gmtime_r(&time, &utc);
#endif
  std::ostringstream output;
  output << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
  return output.str();
}

std::string fixture_checksum(const std::filesystem::path& path) {
  std::ifstream input(path, std::ios::binary);
  if (!input) {
    throw std::runtime_error("cannot read model for checksum");
  }
  std::uint64_t hash = 14695981039346656037ULL;
  char byte = 0;
  while (input.get(byte)) {
    hash ^= static_cast<unsigned char>(byte);
    hash *= 1099511628211ULL;
  }
  std::ostringstream output;
  output << "fnv1a64:" << std::hex << std::setw(16) << std::setfill('0') << hash;
  return output.str();
}

}  // namespace

int main(int argc, char** argv) {
  try {
    const auto options = parse_options(argc, argv);
    const auto model = tinyserve::Model::load(options.model);
    if (options.prompt_tokens + options.generated_tokens > model.config().max_sequence_length) {
      throw std::out_of_range("prompt plus generated tokens exceeds model capacity");
    }
    const auto prompt = fixture_prompt(options.prompt_tokens, model.config().vocab_size);
    const auto no_cache_check = run_once(model, prompt, options.generated_tokens, false);
    const auto cached_check = run_once(model, prompt, options.generated_tokens, true);
    if (no_cache_check.generated != cached_check.generated) {
      throw std::runtime_error("benchmark refused to compare non-equivalent decode outputs");
    }
    for (std::size_t warmup = 0; warmup < options.warmups; ++warmup) {
      static_cast<void>(run_once(model, prompt, options.generated_tokens, false));
      static_cast<void>(run_once(model, prompt, options.generated_tokens, true));
    }

    std::vector<Sample> no_cache_samples;
    std::vector<Sample> cached_samples;
    for (std::size_t repeat = 0; repeat < options.repeats; ++repeat) {
      no_cache_samples.push_back(run_batched_sample(model, prompt, options, false));
      cached_samples.push_back(run_batched_sample(model, prompt, options, true));
    }

    const auto timestamp = timestamp_utc();
    const auto cpu = environment_value("PROCESSOR_IDENTIFIER", "not-reported");
    const auto ram_bytes = physical_ram_bytes();
    const auto model_checksum = fixture_checksum(options.model);
    const auto write_metadata = [&](std::ostream& output, const std::string& mode) {
      output << csv(timestamp) << ',' << csv(options.source_revision) << ','
             << csv(TINYSERVE_BUILD_TYPE) << ',' << csv(compiler()) << ','
             << csv(operating_system()) << ',' << csv(cpu) << ',' << csv(ram_bytes) << ','
             << csv("single-threaded-scalar") << ",1,0," << csv("greedy-temperature-0-top-k-0")
             << ','
             << csv(options.model.filename().string()) << ',' << csv(model_checksum) << ','
             << csv("float32") << ',' << csv(mode) << ',' << options.prompt_tokens << ','
             << options.generated_tokens << ',' << options.generated_tokens - 1 << ','
             << options.warmups << ',' << options.repeats << ',' << options.inner_iterations;
    };

    options.raw_output.parent_path().empty()
        ? static_cast<void>(0)
        : static_cast<void>(std::filesystem::create_directories(options.raw_output.parent_path()));
    std::ofstream raw(options.raw_output);
    raw << "timestamp_utc,source_revision,build_type,compiler,os,cpu,ram_bytes,thread_settings,"
           "batch_size,seed,sampling,model,model_checksum,"
           "precision,mode,prompt_tokens,generated_tokens,decode_tokens,warmups,repeats,"
           "inner_iterations,repeat,ttft_ms,decode_ms,checksum\n";
    const auto write_raw = [&](const std::string& mode, const std::vector<Sample>& samples) {
      for (std::size_t repeat = 0; repeat < samples.size(); ++repeat) {
        write_metadata(raw, mode);
        raw << ',' << repeat << ',' << std::fixed << std::setprecision(6)
            << samples[repeat].ttft_ms << ',' << samples[repeat].decode_ms << ','
            << samples[repeat].checksum << '\n';
      }
    };
    write_raw("no-cache", no_cache_samples);
    write_raw("kv-cache", cached_samples);

    options.output.parent_path().empty()
        ? static_cast<void>(0)
        : static_cast<void>(std::filesystem::create_directories(options.output.parent_path()));
    std::ofstream summary(options.output);
    summary << "timestamp_utc,source_revision,build_type,compiler,os,cpu,ram_bytes,"
               "thread_settings,batch_size,seed,sampling,model,model_checksum,"
               "precision,mode,prompt_tokens,generated_tokens,decode_tokens,warmups,repeats,"
               "inner_iterations,median_ttft_ms,median_decode_ms,decode_tokens_per_sec,"
               "persistent_kv_bytes,process_peak_memory,command\n";
    const auto write_summary = [&](const std::string& mode, const std::vector<Sample>& samples,
                                   std::size_t cache_bytes) {
      std::vector<double> ttft;
      std::vector<double> decode;
      for (const auto& sample : samples) {
        ttft.push_back(sample.ttft_ms);
        decode.push_back(sample.decode_ms);
      }
      const auto median_ttft = median(ttft);
      const auto median_decode = median(decode);
      const auto tokens_per_second =
          static_cast<double>(options.generated_tokens - 1) / (median_decode / 1000.0);
      write_metadata(summary, mode);
      summary << ',' << std::fixed << std::setprecision(6) << median_ttft << ','
              << median_decode << ',' << tokens_per_second << ',' << cache_bytes << ','
              << csv("not-measured") << ',' << csv(options.command) << '\n';
    };
    write_summary("no-cache", no_cache_samples, 0);
    const auto cache_bytes = model.create_kv_cache(options.prompt_tokens + options.generated_tokens)
                                 .memory_bytes();
    write_summary("kv-cache", cached_samples, cache_bytes);
    std::cout << "wrote fixture KV benchmark summary: " << options.output << '\n'
              << "wrote raw samples: " << options.raw_output << '\n';
    return EXIT_SUCCESS;
  } catch (const std::exception& error) {
    std::cerr << "KV benchmark failed: " << error.what() << '\n';
    return EXIT_FAILURE;
  }
}
