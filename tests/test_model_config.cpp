#include "runtime/model_config.hpp"

#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

constexpr const char* kValidConfig = R"(vocab_size=8
hidden_size=4
intermediate_size=8
num_layers=1
num_heads=2
num_kv_heads=1
rope_theta=10000
norm_epsilon=0.00001
max_sequence_length=16
dtype=float32
)";

void require(bool condition, const std::string& message) {
  if (!condition) {
    throw std::runtime_error(message);
  }
}

void require_invalid(const std::function<void()>& action, const std::string& expected_text) {
  try {
    action();
  } catch (const std::invalid_argument& error) {
    require(std::string(error.what()).find(expected_text) != std::string::npos,
            "error did not contain '" + expected_text + "': " + error.what());
    return;
  }
  throw std::runtime_error("expected invalid_argument containing '" + expected_text + "'");
}

}  // namespace

int main() {
  try {
    const auto config = tinyserve::parse_model_config(kValidConfig);
    require(config.vocab_size == 8, "vocab_size was not parsed");
    require(config.hidden_size == 4, "hidden_size was not parsed");
    require(config.intermediate_size == 8, "intermediate_size was not parsed");
    require(config.num_layers == 1, "num_layers was not parsed");
    require(config.num_heads == 2 && config.num_kv_heads == 1, "head counts were not parsed");
    require(config.rope_theta == 10000.0F, "rope_theta was not parsed");
    require(config.max_sequence_length == 16, "max_sequence_length was not parsed");
    require(config.dtype == tinyserve::DataType::float32, "dtype was not parsed");

    require_invalid(
        [] { tinyserve::parse_model_config("vocab_size=8\n"); }, "missing model config field");

    std::string bad_heads = kValidConfig;
    bad_heads.replace(bad_heads.find("hidden_size=4"), std::string("hidden_size=4").size(),
                      "hidden_size=5");
    require_invalid([&] { tinyserve::parse_model_config(bad_heads); },
                    "hidden_size must be divisible");

    std::string duplicate = kValidConfig;
    duplicate += "vocab_size=9\n";
    require_invalid([&] { tinyserve::parse_model_config(duplicate); }, "duplicate");

    std::string unknown = kValidConfig;
    unknown += "architecture=fixture\n";
    require_invalid([&] { tinyserve::parse_model_config(unknown); }, "unknown model config field");

    std::cout << "TinyServe model config tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "TinyServe model config tests failed: " << error.what() << '\n';
    return 1;
  }
}
