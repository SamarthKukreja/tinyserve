#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace tinyserve {

enum class DataType : std::uint32_t {
  float32 = 1,
  float16 = 2,
};

struct ModelConfig {
  std::uint32_t vocab_size = 0;
  std::uint32_t hidden_size = 0;
  std::uint32_t intermediate_size = 0;
  std::uint32_t num_layers = 0;
  std::uint32_t num_heads = 0;
  std::uint32_t num_kv_heads = 0;
  float rope_theta = 0.0F;
  float norm_epsilon = 0.0F;
  std::uint32_t max_sequence_length = 0;
  DataType dtype = DataType::float32;
};

DataType parse_data_type(std::string_view value);
std::string_view data_type_name(DataType dtype);
std::size_t data_type_size(DataType dtype);

ModelConfig parse_model_config(std::string_view text);
void validate_model_config(const ModelConfig& config);

}  // namespace tinyserve
