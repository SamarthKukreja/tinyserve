#include "runtime/generate.hpp"

#include "runtime/generator.hpp"
#include "runtime/model.hpp"
#include "runtime/tokenizer.hpp"

#include <cctype>
#include <cstdint>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <limits>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

namespace tinyserve {
namespace {

struct CliOptions {
  std::filesystem::path model_path;
  std::optional<std::string> prompt;
  std::optional<TokenIds> token_ids;
  GenerationOptions generation;
};

void print_usage(std::ostream& output) {
  output << "Usage:\n"
         << "  tinyserve generate --model FILE (--prompt TEXT | --tokens IDS) "
            "[--max-tokens N] [--temperature T] [--top-k K] [--seed N]\n"
         << "\nIDS is a comma-separated list such as 1,2,3. Temperature 0 selects greedy "
            "decoding.\n";
}

std::string require_value(int& index, int argc, char** argv, const std::string& option) {
  if (index + 1 >= argc) {
    throw std::invalid_argument("missing value for " + option);
  }
  ++index;
  return argv[index];
}

std::uint64_t parse_u64(const std::string& text, const std::string& option) {
  std::size_t consumed = 0;
  unsigned long long value = 0;
  try {
    value = std::stoull(text, &consumed, 10);
  } catch (const std::exception&) {
    throw std::invalid_argument(option + " requires an unsigned integer");
  }
  if (consumed != text.size()) {
    throw std::invalid_argument(option + " requires an unsigned integer");
  }
  return static_cast<std::uint64_t>(value);
}

float parse_float(const std::string& text, const std::string& option) {
  std::size_t consumed = 0;
  float value = 0.0F;
  try {
    value = std::stof(text, &consumed);
  } catch (const std::exception&) {
    throw std::invalid_argument(option + " requires a number");
  }
  if (consumed != text.size()) {
    throw std::invalid_argument(option + " requires a number");
  }
  return value;
}

TokenIds parse_token_ids(const std::string& text) {
  if (text.empty()) {
    throw std::invalid_argument("--tokens must not be empty");
  }
  TokenIds tokens;
  std::size_t offset = 0;
  while (offset <= text.size()) {
    const auto comma = text.find(',', offset);
    const auto end = comma == std::string::npos ? text.size() : comma;
    const auto item = text.substr(offset, end - offset);
    const auto value = parse_u64(item, "--tokens");
    if (value > std::numeric_limits<TokenId>::max()) {
      throw std::out_of_range("--tokens contains an ID larger than uint32");
    }
    tokens.push_back(static_cast<TokenId>(value));
    if (comma == std::string::npos) {
      break;
    }
    offset = comma + 1;
  }
  return tokens;
}

CliOptions parse_options(int argc, char** argv) {
  CliOptions options;
  options.generation.max_new_tokens = 8;
  for (int index = 2; index < argc; ++index) {
    const std::string argument = argv[index];
    if (argument == "--model") {
      options.model_path = require_value(index, argc, argv, argument);
    } else if (argument == "--prompt") {
      options.prompt = require_value(index, argc, argv, argument);
    } else if (argument == "--tokens") {
      options.token_ids = parse_token_ids(require_value(index, argc, argv, argument));
    } else if (argument == "--max-tokens") {
      const auto value = parse_u64(require_value(index, argc, argv, argument), argument);
      if (value == 0 || value > std::numeric_limits<std::size_t>::max()) {
        throw std::out_of_range("--max-tokens must be a positive size_t value");
      }
      options.generation.max_new_tokens = static_cast<std::size_t>(value);
    } else if (argument == "--temperature") {
      options.generation.sampling.temperature =
          parse_float(require_value(index, argc, argv, argument), argument);
    } else if (argument == "--top-k") {
      const auto value = parse_u64(require_value(index, argc, argv, argument), argument);
      if (value > std::numeric_limits<std::size_t>::max()) {
        throw std::out_of_range("--top-k exceeds size_t");
      }
      options.generation.sampling.top_k = static_cast<std::size_t>(value);
    } else if (argument == "--seed") {
      options.generation.sampling.seed =
          parse_u64(require_value(index, argc, argv, argument), argument);
    } else if (argument == "--help" || argument == "-h") {
      print_usage(std::cout);
      throw std::runtime_error("help requested");
    } else {
      throw std::invalid_argument("unknown option '" + argument + "'");
    }
  }
  if (options.model_path.empty()) {
    throw std::invalid_argument("--model is required");
  }
  if (options.prompt.has_value() == options.token_ids.has_value()) {
    throw std::invalid_argument("provide exactly one of --prompt or --tokens");
  }
  if (options.prompt && options.prompt->empty()) {
    throw std::invalid_argument("--prompt must not be empty");
  }
  return options;
}

std::string token_list(const TokenIds& tokens) {
  std::ostringstream output;
  for (std::size_t index = 0; index < tokens.size(); ++index) {
    if (index != 0) {
      output << ',';
    }
    output << tokens[index];
  }
  return output.str();
}

std::string escaped_bytes(const TokenIds& tokens) {
  std::ostringstream output;
  output << '"';
  for (const auto token : tokens) {
    if (token > 255) {
      output << "<token:" << token << ">";
      continue;
    }
    const auto byte = static_cast<unsigned char>(token);
    if (byte == '\\') {
      output << "\\\\";
    } else if (byte == '"') {
      output << "\\\"";
    } else if (std::isprint(byte) != 0) {
      output << static_cast<char>(byte);
    } else {
      output << "\\x" << std::hex << std::setw(2) << std::setfill('0')
             << static_cast<unsigned int>(byte) << std::dec;
    }
  }
  output << '"';
  return output.str();
}

}  // namespace

int run_cli(int argc, char** argv) {
  if (argc < 2) {
    print_usage(std::cout);
    return 0;
  }
  if (std::string(argv[1]) != "generate") {
    std::cerr << "error: unknown command '" << argv[1] << "'\n";
    print_usage(std::cerr);
    return 2;
  }
  try {
    const auto options = parse_options(argc, argv);
    const auto model = Model::load(options.model_path);
    const ByteTokenizer tokenizer;
    const auto prompt_tokens = options.prompt ? tokenizer.encode(*options.prompt) : *options.token_ids;
    const auto result = generate_tokens(model, prompt_tokens, options.generation);
    std::cout << "validation_status=synthetic_fixture_unverified\n"
              << "prompt_token_ids=" << token_list(result.prompt_tokens) << '\n'
              << "generated_token_ids=" << token_list(result.generated_tokens) << '\n'
              << "generated_text=" << escaped_bytes(result.generated_tokens) << '\n';
    return 0;
  } catch (const std::runtime_error& error) {
    if (std::string(error.what()) == "help requested") {
      return 0;
    }
    std::cerr << "error: " << error.what() << '\n';
    return 2;
  } catch (const std::exception& error) {
    std::cerr << "error: " << error.what() << '\n';
    return 2;
  }
}

}  // namespace tinyserve
