#include "runtime/generator.hpp"
#include "runtime/model.hpp"
#include "runtime/sampling.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void require(bool condition, const std::string& message) {
  if (!condition) {
    throw std::runtime_error(message);
  }
}

}  // namespace

int main(int argc, char** argv) {
  try {
    require(argc == 2, "expected fixture path argument");
    require(tinyserve::greedy_argmax({1.0F, 3.0F, 3.0F}) == 1,
            "greedy argmax must choose the lowest ID on a tie");

    const auto cold = tinyserve::sampling_probabilities({0.0F, 1.0F}, 0.5F);
    const auto warm = tinyserve::sampling_probabilities({0.0F, 1.0F}, 2.0F);
    require(cold[1] > warm[1], "higher temperature did not flatten probabilities");
    const auto top_one = tinyserve::sampling_probabilities({1.0F, 3.0F, 2.0F}, 1.0F, 1);
    require(top_one == std::vector<float>({0.0F, 1.0F, 0.0F}), "top-k filtering is wrong");

    tinyserve::Sampler first_sampler({0.8F, 3, 42});
    tinyserve::Sampler second_sampler({0.8F, 3, 42});
    for (int sample = 0; sample < 16; ++sample) {
      require(first_sampler.select({0.0F, 1.0F, 2.0F, 3.0F}) ==
                  second_sampler.select({0.0F, 1.0F, 2.0F, 3.0F}),
              "seeded sampling is not deterministic");
    }

    const auto model = tinyserve::Model::load(argv[1]);
    tinyserve::GenerationOptions greedy_options;
    greedy_options.max_new_tokens = 3;
    const auto first = tinyserve::generate_tokens(model, {65}, greedy_options);
    const auto second = tinyserve::generate_tokens(model, {65}, greedy_options);
    require(first.generated_tokens.size() == 3, "decode loop ignored max_new_tokens");
    require(first.generated_tokens == second.generated_tokens, "greedy generation is not stable");
    require(first.all_tokens.size() == 4, "generated sequence length is wrong");

    tinyserve::GenerationOptions cached_options = greedy_options;
    cached_options.decode_mode = tinyserve::DecodeMode::kv_cache;
    const auto cached = tinyserve::generate_tokens(model, {65}, cached_options);
    require(cached.generated_tokens == first.generated_tokens,
            "cached greedy output differs from no-cache output");
    require(cached.kv_cache_tokens == cached.prompt_tokens.size() +
                                          cached.generated_tokens.size() - 1,
            "cached generation did not grow once per processed decode token");
    require(tinyserve::generate_tokens(model, {65}, cached_options).generated_tokens ==
                cached.generated_tokens,
            "repeated cached generation did not reset state");

    const tinyserve::TokenIds prefix = {65, 66, 67};
    auto cache = model.create_kv_cache(prefix.size());
    std::vector<float> cached_logits;
    for (std::size_t position = 0; position < prefix.size(); ++position) {
      cached_logits = model.next_token_logits_cached(prefix[position], cache);
      require(cache.synchronized_size() == position + 1,
              "model cache did not grow by one token");
    }
    const auto no_cache_logits = model.next_token_logits(prefix);
    require(cached_logits.size() == no_cache_logits.size(), "cached logit shape is wrong");
    for (std::size_t index = 0; index < cached_logits.size(); ++index) {
      require(std::fabs(cached_logits[index] - no_cache_logits[index]) <= 2.0e-5F,
              "cached logits differ from no-cache logits");
    }
    cache.reset();
    for (const auto token : prefix) {
      cached_logits = model.next_token_logits_cached(token, cache);
    }
    require(cache.synchronized_size() == prefix.size(), "reset cache did not refill correctly");

    tinyserve::GenerationOptions sampled_options;
    sampled_options.max_new_tokens = 3;
    sampled_options.sampling = {0.9F, 5, 1234};
    require(tinyserve::generate_tokens(model, {65}, sampled_options).generated_tokens ==
                tinyserve::generate_tokens(model, {65}, sampled_options).generated_tokens,
            "seeded model generation is not deterministic");

    bool capacity_failed = false;
    try {
      tinyserve::GenerationOptions too_long;
      too_long.max_new_tokens = model.config().max_sequence_length;
      static_cast<void>(tinyserve::generate_tokens(model, {65}, too_long));
    } catch (const std::out_of_range&) {
      capacity_failed = true;
    }
    require(capacity_failed, "generation capacity validation did not fail");

    std::cout << "TinyServe generation tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "TinyServe generation tests failed: " << error.what() << '\n';
    return 1;
  }
}
