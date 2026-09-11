#include "runtime/attention.hpp"

#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

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

}  // namespace

int main() {
  try {
    const auto probabilities = tinyserve::stable_softmax({1000.0F, 1001.0F});
    require_close(probabilities[0], 0.26894143F, "stable softmax first value is wrong");
    require_close(probabilities[1], 0.73105860F, "stable softmax second value is wrong");

    tinyserve::Tensor scores({2, 2}, {1.0F, 2.0F, 3.0F, 4.0F});
    tinyserve::apply_causal_mask(scores);
    require(std::isinf(scores.at({0, 1})) && scores.at({0, 1}) < 0.0F,
            "future attention score was not masked");
    require_close(scores.at({1, 0}), 3.0F, "past attention score was changed");

    const tinyserve::Tensor queries({2, 2, 2}, 0.0F);
    const tinyserve::Tensor keys({2, 1, 2}, 0.0F);
    const tinyserve::Tensor values({2, 1, 2}, {2.0F, 4.0F, 6.0F, 8.0F});
    const auto attended = tinyserve::scaled_dot_product_attention(queries, keys, values);
    require(attended.shape() == std::vector<std::size_t>({2, 2, 2}),
            "GQA attention output shape is wrong");
    for (std::size_t head = 0; head < 2; ++head) {
      require_close(attended.at({0, head, 0}), 2.0F, "causal first-token value is wrong");
      require_close(attended.at({0, head, 1}), 4.0F, "causal first-token value is wrong");
      require_close(attended.at({1, head, 0}), 4.0F, "uniform attention average is wrong");
      require_close(attended.at({1, head, 1}), 6.0F, "uniform attention average is wrong");
    }

    require_failure([&] {
      const tinyserve::Tensor bad_queries({2, 3, 2}, 0.0F);
      const tinyserve::Tensor bad_keys({2, 2, 2}, 0.0F);
      const tinyserve::Tensor bad_values({2, 2, 2}, 0.0F);
      static_cast<void>(
          tinyserve::scaled_dot_product_attention(bad_queries, bad_keys, bad_values));
    });
    require_failure([] {
      static_cast<void>(tinyserve::stable_softmax(
          {-std::numeric_limits<float>::infinity()}));
    });

    std::cout << "TinyServe attention tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "TinyServe attention tests failed: " << error.what() << '\n';
    return 1;
  }
}
