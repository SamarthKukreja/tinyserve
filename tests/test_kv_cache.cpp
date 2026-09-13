#include "runtime/kv_cache.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void require(bool condition, const std::string& message) {
  if (!condition) {
    throw std::runtime_error(message);
  }
}

template <typename Exception, typename Function>
void require_throws(Function&& function, const std::string& message) {
  try {
    function();
  } catch (const Exception&) {
    return;
  }
  throw std::runtime_error(message);
}

}  // namespace

int main() {
  try {
    tinyserve::KvCache cache(2, 1, 3, 2);
    require(cache.num_layers() == 2 && cache.capacity() == 3, "cache geometry is wrong");
    require(cache.memory_bytes() == 2 * 2 * 3 * 1 * 2 * sizeof(float),
            "cache byte accounting is wrong");
    require(cache.synchronized_size() == 0, "new cache is not empty");

    const tinyserve::Tensor keys({1, 1, 2}, {1.0F, 2.0F});
    const tinyserve::Tensor values({1, 1, 2}, {3.0F, 4.0F});
    cache.append(0, keys, values);
    require(cache.layer_size(0) == 1, "append did not grow layer zero");
    require_throws<std::logic_error>([&] { static_cast<void>(cache.synchronized_size()); },
                                     "unsynchronized layer sizes were accepted");
    cache.append(1, keys, values);
    require(cache.synchronized_size() == 1, "layers did not synchronize after append");
    require(cache.keys(0).values() == keys.values(), "retrieved keys differ from appended keys");
    require(cache.values(1).values() == values.values(),
            "retrieved values differ from appended values");

    require_throws<std::invalid_argument>(
        [&] { cache.append(0, tinyserve::Tensor({1, 2, 2}), values); },
        "invalid append shape was accepted");
    require_throws<std::out_of_range>([&] { static_cast<void>(cache.layer_size(2)); },
                                      "invalid layer was accepted");

    cache.append(0, keys, values);
    cache.append(0, keys, values);
    require_throws<std::out_of_range>([&] { cache.append(0, keys, values); },
                                      "cache capacity overflow was accepted");
    cache.reset();
    require(cache.synchronized_size() == 0, "reset did not clear logical sizes");
    require_throws<std::logic_error>([&] { static_cast<void>(cache.keys(0)); },
                                     "empty cache retrieval was accepted");
    cache.append(0, keys, values);
    cache.append(1, keys, values);
    require(cache.synchronized_size() == 1, "cache was not reusable after reset");

    require_throws<std::invalid_argument>([] { tinyserve::KvCache invalid(1, 1, 0, 2); },
                                          "zero cache capacity was accepted");
    std::cout << "TinyServe KV cache tests passed\n";
    return EXIT_SUCCESS;
  } catch (const std::exception& error) {
    std::cerr << "TinyServe KV cache tests failed: " << error.what() << '\n';
    return EXIT_FAILURE;
  }
}
