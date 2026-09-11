#pragma once

#include "runtime/model.hpp"
#include "runtime/sampling.hpp"
#include "runtime/tokenizer.hpp"

#include <cstddef>
#include <optional>

namespace tinyserve {

struct GenerationOptions {
  std::size_t max_new_tokens = 1;
  SamplingOptions sampling;
  std::optional<TokenId> stop_token_id;
};

struct GenerationResult {
  TokenIds prompt_tokens;
  TokenIds generated_tokens;
  TokenIds all_tokens;
};

GenerationResult generate_tokens(const Model& model, const TokenIds& prompt_tokens,
                                 const GenerationOptions& options);

}  // namespace tinyserve
