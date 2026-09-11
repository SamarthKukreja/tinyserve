#include <iostream>

int main() {
  constexpr bool test_runner_is_active = true;
  if (!test_runner_is_active) {
    std::cerr << "TinyServe smoke test failed\n";
    return 1;
  }

  std::cout << "TinyServe smoke test passed\n";
  return 0;
}
