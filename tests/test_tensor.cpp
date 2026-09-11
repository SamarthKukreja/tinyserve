#include "runtime/tensor.hpp"

#include <cmath>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void require(bool condition, const std::string& message) {
  if (!condition) {
    throw std::runtime_error(message);
  }
}

void require_close(float actual, float expected, float tolerance, const std::string& message) {
  if (std::fabs(actual - expected) > tolerance) {
    throw std::runtime_error(message);
  }
}

void require_failure(const std::function<void()>& action, const std::string& label) {
  try {
    action();
  } catch (const std::exception&) {
    return;
  }
  throw std::runtime_error("expected operation to fail: " + label);
}

}  // namespace

int main() {
  try {
    tinyserve::Tensor tensor({2, 3}, {0.0F, 1.0F, 2.0F, 3.0F, 4.0F, 5.0F});
    require(tensor.rank() == 2 && tensor.size() == 6, "tensor metadata is wrong");
    require(tensor.offset({1, 2}) == 5, "row-major offset is wrong");
    require_close(tensor.at({1, 1}), 4.0F, 0.0F, "tensor indexing is wrong");
    tensor.at({0, 2}) = 7.0F;
    require_close(tensor.values()[2], 7.0F, 0.0F, "mutable indexing is wrong");
    require_failure([&] { static_cast<void>(tensor.at({2, 0})); }, "out-of-range index");
    require_failure([] { tinyserve::Tensor invalid(std::vector<std::size_t>{2, 0}); },
                    "zero dimension");
    require_failure(
        [] {
          tinyserve::Tensor invalid(std::vector<std::size_t>{2, 2},
                                    std::vector<float>{1.0F});
        },
        "value count mismatch");

    require_close(tinyserve::dot_product({1.0F, 2.0F, 3.0F}, {4.0F, 5.0F, 6.0F}),
                  32.0F, 1.0e-6F, "dot product is wrong");
    const tinyserve::Tensor matrix({2, 2}, {1.0F, 2.0F, 3.0F, 4.0F});
    const auto product = tinyserve::matrix_vector_multiply(matrix, {2.0F, 1.0F});
    require(product == std::vector<float>({4.0F, 10.0F}), "matrix-vector product is wrong");

    const tinyserve::Tensor left({2}, {1.0F, 2.0F});
    const tinyserve::Tensor right({2}, {3.0F, 4.0F});
    require(tinyserve::elementwise_add(left, right).values() ==
                std::vector<float>({4.0F, 6.0F}),
            "elementwise add is wrong");
    require(tinyserve::elementwise_multiply(left, right).values() ==
                std::vector<float>({3.0F, 8.0F}),
            "elementwise multiply is wrong");
    require(tinyserve::multiply_scalar(left, 2.0F).values() ==
                std::vector<float>({2.0F, 4.0F}),
            "scalar multiply is wrong");

    std::cout << "TinyServe tensor tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "TinyServe tensor tests failed: " << error.what() << '\n';
    return 1;
  }
}
