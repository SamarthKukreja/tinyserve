#pragma once

#include "runtime/model.hpp"
#include "runtime/sampling.hpp"
#include "runtime/tokenizer.hpp"

#include <cstddef>
#include <optional>

namespace tinyserve {

enum class DecodeMode {
  no_cache,
  kv_cache,
};

struct GenerationOptions {
  std::size_t max_new_tokens = 1;
  SamplingOptions sampling;
  std::optional<TokenId> stop_token_id;
  DecodeMode decode_mode = DecodeMode::no_cache;
};

struct GenerationResult {
  TokenIds prompt_tokens;
  TokenIds generated_tokens;
  TokenIds all_tokens;
  std::size_t kv_cache_tokens = 0;
};

GenerationResult generate_tokens(const Model& model, const TokenIds& prompt_tokens,
                                 const GenerationOptions& options);

}  // namespace tinyserve
