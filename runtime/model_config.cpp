#include "runtime/model_config.hpp"

#include <array>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace tinyserve {
namespace {

std::string trim(std::string_view value) {
  const auto first = value.find_first_not_of(" \t\r");
  if (first == std::string_view::npos) {
    return {};
  }
  const auto last = value.find_last_not_of(" \t\r");
  return std::string(value.substr(first, last - first + 1));
}

std::uint32_t parse_positive_u32(const std::string& key, const std::string& value) {
  std::size_t consumed = 0;
  unsigned long long parsed = 0;
  try {
    parsed = std::stoull(value, &consumed, 10);
  } catch (const std::exception&) {
    throw std::invalid_argument("model config field '" + key + "' must be a positive integer");
  }
  if (consumed != value.size() || parsed == 0 ||
      parsed > std::numeric_limits<std::uint32_t>::max()) {
    throw std::invalid_argument("model config field '" + key + "' must be a positive 32-bit integer");
  }
  return static_cast<std::uint32_t>(parsed);
}

float parse_positive_float(const std::string& key, const std::string& value) {
  char* end = nullptr;
  errno = 0;
  const float parsed = std::strtof(value.c_str(), &end);
  if (errno == ERANGE || end == value.c_str() || *end != '\0' || !std::isfinite(parsed) ||
      parsed <= 0.0F) {
    throw std::invalid_argument("model config field '" + key + "' must be a positive finite number");
  }
  return parsed;
}

}  // namespace

DataType parse_data_type(std::string_view value) {
  if (value == "float32") {
    return DataType::float32;
  }
  if (value == "float16") {
    return DataType::float16;
  }
  throw std::invalid_argument("unsupported dtype '" + std::string(value) + "'");
}

std::string_view data_type_name(DataType dtype) {
  switch (dtype) {
    case DataType::float32:
      return "float32";
    case DataType::float16:
      return "float16";
  }
  throw std::invalid_argument("unknown dtype code");
}

std::size_t data_type_size(DataType dtype) {
  switch (dtype) {
    case DataType::float32:
      return 4;
    case DataType::float16:
      return 2;
  }
  throw std::invalid_argument("unknown dtype code");
}

ModelConfig parse_model_config(std::string_view text) {
  std::unordered_map<std::string, std::string> values;
  std::size_t offset = 0;
  std::size_t line_number = 0;
  while (offset <= text.size()) {
    const auto end = text.find('\n', offset);
    const auto line_end = end == std::string_view::npos ? text.size() : end;
    const std::string line = trim(text.substr(offset, line_end - offset));
    ++line_number;
    if (!line.empty() && line.front() != '#') {
      const auto separator = line.find('=');
      if (separator == std::string::npos) {
        throw std::invalid_argument("model config line " + std::to_string(line_number) +
                                    " must use key=value syntax");
      }
      const std::string key = trim(std::string_view(line).substr(0, separator));
      const std::string value = trim(std::string_view(line).substr(separator + 1));
      if (key.empty() || value.empty()) {
        throw std::invalid_argument("model config line " + std::to_string(line_number) +
                                    " has an empty key or value");
      }
      if (!values.emplace(key, value).second) {
        throw std::invalid_argument("duplicate model config field '" + key + "'");
      }
    }
    if (end == std::string_view::npos) {
      break;
    }
    offset = end + 1;
  }

  constexpr std::array<std::string_view, 10> required = {
      "vocab_size",          "hidden_size", "intermediate_size", "num_layers",
      "num_heads",           "num_kv_heads", "rope_theta",        "norm_epsilon",
      "max_sequence_length", "dtype"};
  for (const auto key : required) {
    if (values.find(std::string(key)) == values.end()) {
      throw std::invalid_argument("missing model config field '" + std::string(key) + "'");
    }
  }
  const std::unordered_set<std::string> allowed(required.begin(), required.end());
  for (const auto& entry : values) {
    if (allowed.find(entry.first) == allowed.end()) {
      throw std::invalid_argument("unknown model config field '" + entry.first + "'");
    }
  }

  ModelConfig config;
  config.vocab_size = parse_positive_u32("vocab_size", values.at("vocab_size"));
  config.hidden_size = parse_positive_u32("hidden_size", values.at("hidden_size"));
  config.intermediate_size =
      parse_positive_u32("intermediate_size", values.at("intermediate_size"));
  config.num_layers = parse_positive_u32("num_layers", values.at("num_layers"));
  config.num_heads = parse_positive_u32("num_heads", values.at("num_heads"));
  config.num_kv_heads = parse_positive_u32("num_kv_heads", values.at("num_kv_heads"));
  config.rope_theta = parse_positive_float("rope_theta", values.at("rope_theta"));
  config.norm_epsilon = parse_positive_float("norm_epsilon", values.at("norm_epsilon"));
  config.max_sequence_length =
      parse_positive_u32("max_sequence_length", values.at("max_sequence_length"));
  config.dtype = parse_data_type(values.at("dtype"));
  validate_model_config(config);
  return config;
}

void validate_model_config(const ModelConfig& config) {
  if (config.vocab_size == 0 || config.hidden_size == 0 || config.intermediate_size == 0 ||
      config.num_layers == 0 || config.num_heads == 0 || config.num_kv_heads == 0 ||
      config.max_sequence_length == 0) {
    throw std::invalid_argument("model config dimensions and counts must be positive");
  }
  if (!std::isfinite(config.rope_theta) || config.rope_theta <= 0.0F) {
    throw std::invalid_argument("rope_theta must be positive and finite");
  }
  if (!std::isfinite(config.norm_epsilon) || config.norm_epsilon <= 0.0F) {
    throw std::invalid_argument("norm_epsilon must be positive and finite");
  }
  if (config.hidden_size % config.num_heads != 0) {
    throw std::invalid_argument("hidden_size must be divisible by num_heads");
  }
  if (config.num_kv_heads > config.num_heads || config.num_heads % config.num_kv_heads != 0) {
    throw std::invalid_argument("num_kv_heads must divide num_heads and cannot exceed it");
  }
  static_cast<void>(data_type_size(config.dtype));
}

}  // namespace tinyserve
