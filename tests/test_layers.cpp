#include "runtime/layers.hpp"

#include <cmath>
#include <functional>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr float kTolerance = 1.0e-5F;

void require(bool condition, const std::string& message) {
  if (!condition) {
    throw std::runtime_error(message);
  }
}

void require_close(float actual, float expected, const std::string& message) {
  if (std::fabs(actual - expected) > kTolerance) {
    throw std::runtime_error(message);
  }
}

void require_failure(const std::function<void()>& action) {
  try {
    action();
  } catch (const std::exception&) {
    return;
  }
  throw std::runtime_error("expected operation to fail");
}

tinyserve::Tensor identity(std::size_t size) {
  tinyserve::Tensor result({size, size});
  for (std::size_t index = 0; index < size; ++index) {
    result.at({index, index}) = 1.0F;
  }
  return result;
}

}  // namespace

int main() {
  try {
    const tinyserve::Tensor norm_input({2, 2}, {3.0F, 4.0F, 0.0F, 0.0F});
    const tinyserve::Tensor norm_weight({2}, {1.0F, 1.0F});
    const auto normalized = tinyserve::rms_norm(norm_input, norm_weight, 0.0F);
    require_close(normalized.at({0, 0}), 0.84852814F, "RMSNorm first value is wrong");
    require_close(normalized.at({0, 1}), 1.13137085F, "RMSNorm second value is wrong");
    require_close(normalized.at({1, 0}), 0.0F, "RMSNorm zero row is wrong");

    tinyserve::Tensor queries({2, 1, 2}, {1.0F, 2.0F, 1.0F, 0.0F});
    tinyserve::Tensor keys({2, 1, 2}, {3.0F, 4.0F, 1.0F, 0.0F});
    tinyserve::apply_rope(queries, keys, 0, 10000.0F);
    require_close(queries.at({0, 0, 0}), 1.0F, "RoPE position zero must be unchanged");
    require_close(queries.at({0, 0, 1}), 2.0F, "RoPE position zero must be unchanged");
    require_close(queries.at({1, 0, 0}), std::cos(1.0F), "RoPE cosine value is wrong");
    require_close(queries.at({1, 0, 1}), std::sin(1.0F), "RoPE sine value is wrong");
    tinyserve::Tensor split_queries({1, 1, 4}, {1.0F, 2.0F, 3.0F, 4.0F});
    tinyserve::Tensor split_keys({1, 1, 4}, {1.0F, 2.0F, 3.0F, 4.0F});
    tinyserve::apply_rope(split_queries, split_keys, 1, 10000.0F);
    require_close(split_queries.at({0, 0, 0}),
                  std::cos(1.0F) - 3.0F * std::sin(1.0F),
                  "RoPE split-half first frequency is wrong");
    require_close(split_queries.at({0, 0, 2}),
                  3.0F * std::cos(1.0F) + std::sin(1.0F),
                  "RoPE split-half rotated coordinate is wrong");
    require_close(split_queries.at({0, 0, 1}),
                  2.0F * std::cos(0.01F) - 4.0F * std::sin(0.01F),
                  "RoPE split-half second frequency is wrong");
    require_close(split_queries.at({0, 0, 3}),
                  4.0F * std::cos(0.01F) + 2.0F * std::sin(0.01F),
                  "RoPE split-half rotated coordinate is wrong");
    require_failure([] {
      tinyserve::Tensor odd_queries({1, 1, 3}, 0.0F);
      tinyserve::Tensor odd_keys({1, 1, 3}, 0.0F);
      tinyserve::apply_rope(odd_queries, odd_keys, 0, 10000.0F);
    });

    const tinyserve::Tensor linear_input({1, 2}, {2.0F, 3.0F});
    const tinyserve::Tensor linear_weights({2, 2}, {1.0F, 0.0F, 0.0F, 2.0F});
    require(tinyserve::linear(linear_input, linear_weights).values() ==
                std::vector<float>({2.0F, 6.0F}),
            "linear projection is wrong");
    const tinyserve::Tensor linear_bias({2}, {0.5F, -1.0F});
    require(tinyserve::linear(linear_input, linear_weights, &linear_bias).values() ==
                std::vector<float>({2.5F, 5.0F}),
            "linear projection bias is wrong");
    const auto activated = tinyserve::swiglu(tinyserve::Tensor({1}, {1.0F}),
                                              tinyserve::Tensor({1}, {2.0F}));
    require_close(activated.values()[0], 1.4621172F, "SwiGLU output is wrong");
    const auto mlp = tinyserve::feed_forward(linear_input, identity(2), identity(2), identity(2));
    require(mlp.shape() == linear_input.shape(), "MLP output shape is wrong");
    require_close(mlp.at({0, 0}), tinyserve::silu(2.0F) * 2.0F,
                  "MLP first value is wrong");
    require_close(mlp.at({0, 1}), tinyserve::silu(3.0F) * 3.0F,
                  "MLP second value is wrong");
    require(tinyserve::residual_add(tinyserve::Tensor({2}, {1.0F, 2.0F}),
                                    tinyserve::Tensor({2}, {3.0F, 4.0F}))
                .values() == std::vector<float>({4.0F, 6.0F}),
            "residual add is wrong");

    tinyserve::ModelConfig config;
    config.vocab_size = 8;
    config.hidden_size = 2;
    config.intermediate_size = 2;
    config.num_layers = 1;
    config.num_heads = 1;
    config.num_kv_heads = 1;
    config.rope_theta = 10000.0F;
    config.norm_epsilon = 1.0e-5F;
    config.max_sequence_length = 8;
    config.dtype = tinyserve::DataType::float32;

    // Deterministic dummy block fixture: zero updates make both residual paths explicit.
    const tinyserve::DecoderBlockWeights fixture_weights{
        tinyserve::Tensor({2}, {1.0F, 1.0F}),
        identity(2),
        identity(2),
        identity(2),
        std::nullopt,
        std::nullopt,
        std::nullopt,
        tinyserve::Tensor({2, 2}, 0.0F),
        tinyserve::Tensor({2}, {1.0F, 1.0F}),
        tinyserve::Tensor({2, 2}, 0.0F),
        tinyserve::Tensor({2, 2}, 0.0F),
        tinyserve::Tensor({2, 2}, 0.0F),
    };
    const tinyserve::Tensor block_input({2, 2}, {1.0F, 0.5F, -0.5F, 1.0F});
    const auto first = tinyserve::decoder_block(block_input, fixture_weights, config);
    const auto second = tinyserve::decoder_block(block_input, fixture_weights, config);
    require(first.shape() == block_input.shape(), "decoder block output shape is wrong");
    require(first.values() == block_input.values(), "zero-update residual block changed its input");
    require(first.values() == second.values(), "decoder block is not deterministic");
    require_failure([&] {
      static_cast<void>(tinyserve::decoder_block(block_input, fixture_weights, config, 7));
    });

    std::cout << "TinyServe layer tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "TinyServe layer tests failed: " << error.what() << '\n';
    return 1;
  }
}
