#include "runtime/generator.hpp"

#include <limits>
#include <stdexcept>

namespace tinyserve {

GenerationResult generate_tokens(const Model& model, const TokenIds& prompt_tokens,
                                 const GenerationOptions& options) {
  if (prompt_tokens.empty()) {
    throw std::invalid_argument("generation requires at least one prompt token");
  }
  if (options.max_new_tokens > std::numeric_limits<std::size_t>::max() - prompt_tokens.size() ||
      prompt_tokens.size() + options.max_new_tokens > model.config().max_sequence_length) {
    throw std::out_of_range("prompt plus max_new_tokens exceeds max_sequence_length");
  }
  if (options.stop_token_id && *options.stop_token_id >= model.config().vocab_size) {
    throw std::out_of_range("stop token ID exceeds vocabulary size");
  }

  GenerationResult result;
  result.prompt_tokens = prompt_tokens;
  result.all_tokens = prompt_tokens;
  result.generated_tokens.reserve(options.max_new_tokens);
  Sampler sampler(options.sampling);
  if (options.decode_mode == DecodeMode::no_cache) {
    for (std::size_t step = 0; step < options.max_new_tokens; ++step) {
      const auto next = sampler.select(model.next_token_logits(result.all_tokens));
      result.generated_tokens.push_back(next);
      result.all_tokens.push_back(next);
      if (options.stop_token_id && next == *options.stop_token_id) {
        break;
      }
    }
    return result;
  }

  if (options.max_new_tokens == 0) {
    return result;
  }
  auto cache = model.create_kv_cache(prompt_tokens.size() + options.max_new_tokens);
  std::vector<float> logits;
  for (const auto token : prompt_tokens) {
    logits = model.next_token_logits_cached(token, cache);
  }
  for (std::size_t step = 0; step < options.max_new_tokens; ++step) {
    const auto next = sampler.select(logits);
    result.generated_tokens.push_back(next);
    result.all_tokens.push_back(next);
    if (options.stop_token_id && next == *options.stop_token_id) {
      break;
    }
    if (step + 1 < options.max_new_tokens) {
      logits = model.next_token_logits_cached(next, cache);
    }
  }
  result.kv_cache_tokens = cache.synchronized_size();
  return result;
}

}  // namespace tinyserve
