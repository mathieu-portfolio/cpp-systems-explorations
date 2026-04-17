#include "arena.hpp"

#include <cctype>
#include <cstddef>
#include <cstring>
#include <iostream>

enum class TokenKind {
  Number,
  Plus,
  Minus
};

struct Token {
  TokenKind kind;
  int value;
};

const char* token_kind_name(TokenKind kind)
{
  switch (kind)
  {
    case TokenKind::Number: return "Number";
    case TokenKind::Plus:   return "Plus";
    case TokenKind::Minus:  return "Minus";
  }

  return "Unknown";
}

// Temporary request buffer:
// copies the input without spaces so later stages work on a compact buffer.
char* normalize_expression(Arena& arena, const char* input)
{
  const size_t input_len = std::strlen(input);

  char* buffer = static_cast<char*>(arena.allocate(input_len + 1, alignof(char)));
  if (buffer == nullptr)
  {
    return nullptr;
  }

  size_t out = 0;
  for (size_t i = 0; i < input_len; ++i)
  {
    unsigned char c = static_cast<unsigned char>(input[i]);
    if (!std::isspace(c))
    {
      buffer[out++] = static_cast<char>(c);
    }
  }

  buffer[out] = '\0';
  return buffer;
}

bool count_tokens(const char* expr, size_t& out_count)
{
  out_count = 0;

  if (expr[0] == '\0')
  {
    return false;
  }

  size_t i = 0;
  while (expr[i] != '\0')
  {
    unsigned char c = static_cast<unsigned char>(expr[i]);

    if (std::isdigit(c))
    {
      ++out_count;
      while (std::isdigit(static_cast<unsigned char>(expr[i])))
      {
        ++i;
      }
      continue;
    }

    if (expr[i] == '+' || expr[i] == '-')
    {
      ++out_count;
      ++i;
      continue;
    }

    return false;
  }

  return true;
}

// Temporary request buffer:
// stores a tokenized view of the expression for evaluation.
Token* tokenize_expression(Arena& arena, const char* expr, size_t& out_count)
{
  if (!count_tokens(expr, out_count))
  {
    out_count = 0;
    return nullptr;
  }

  Token* tokens = static_cast<Token*>(arena.allocate<Token>(out_count));
  if (tokens == nullptr)
  {
    out_count = 0;
    return nullptr;
  }

  size_t token_index = 0;
  size_t i = 0;

  while (expr[i] != '\0')
  {
    unsigned char c = static_cast<unsigned char>(expr[i]);

    if (std::isdigit(c))
    {
      int value = 0;
      while (std::isdigit(static_cast<unsigned char>(expr[i])))
      {
        value = value * 10 + (expr[i] - '0');
        ++i;
      }

      tokens[token_index++] = Token{TokenKind::Number, value};
      continue;
    }

    if (expr[i] == '+')
    {
      tokens[token_index++] = Token{TokenKind::Plus, 0};
      ++i;
      continue;
    }

    if (expr[i] == '-')
    {
      tokens[token_index++] = Token{TokenKind::Minus, 0};
      ++i;
      continue;
    }

    out_count = 0;
    return nullptr;
  }

  return tokens;
}

bool evaluate_tokens(const Token* tokens, size_t count, int& result)
{
  if (count == 0 || tokens[0].kind != TokenKind::Number)
  {
    return false;
  }

  result = tokens[0].value;

  for (size_t i = 1; i + 1 < count; i += 2)
  {
    const Token& op = tokens[i];
    const Token& rhs = tokens[i + 1];

    if (rhs.kind != TokenKind::Number)
    {
      return false;
    }

    if (op.kind == TokenKind::Plus)
    {
      result += rhs.value;
    }
    else if (op.kind == TokenKind::Minus)
    {
      result -= rhs.value;
    }
    else
    {
      return false;
    }
  }

  return (count % 2) == 1;
}

void print_tokens(const Token* tokens, size_t count)
{
  for (size_t i = 0; i < count; ++i)
  {
    std::cout << "  [" << i << "] " << token_kind_name(tokens[i].kind);
    if (tokens[i].kind == TokenKind::Number)
    {
      std::cout << " (" << tokens[i].value << ")";
    }
    std::cout << '\n';
  }
}

bool process_request(Arena& arena, const char* input)
{
  std::cout << "Input: \"" << input << "\"\n";
  std::cout << "Arena used before request: " << arena.used() << " bytes\n";

  auto marker = arena.mark();

  char* normalized = normalize_expression(arena, input);
  if (normalized == nullptr)
  {
    std::cout << "Failed: could not allocate normalized buffer\n";
    return false;
  }

  std::cout << "Normalized buffer allocated in arena\n";
  std::cout << "Arena used after normalize: " << arena.used() << " bytes\n";

  size_t token_count = 0;
  Token* tokens = tokenize_expression(arena, normalized, token_count);
  if (tokens == nullptr)
  {
    std::cout << "Failed: invalid expression or token allocation failure\n";
    arena.rewind(marker);
    return false;
  }

  std::cout << "Token buffer allocated in arena\n";
  std::cout << "Arena used after tokenize: " << arena.used() << " bytes\n";

  int result = 0;
  if (!evaluate_tokens(tokens, token_count, result))
  {
    std::cout << "Failed: invalid token sequence\n";
    arena.rewind(marker);
    return false;
  }

  std::cout << "Normalized: \"" << normalized << "\"\n";
  std::cout << "Token count: " << token_count << '\n';
  std::cout << "Tokens:\n";
  print_tokens(tokens, token_count);
  std::cout << "Result: " << result << '\n';
  std::cout << "Arena used before rewind: " << arena.used() << " bytes\n";

  arena.rewind(marker);

  std::cout << "Arena used after rewind:  " << arena.used() << " bytes\n";
  std::cout << "All temporary memory for this request was reclaimed in one step.\n";

  return true;
}

int main()
{
  Arena arena(1024, alignof(std::max_align_t));

  const char* requests[] = {
    "12 + 7 - 3 + 20",
    "100 - 50 + 25",
    "1 + 2 + 3 + 4"
  };

  std::cout << "=== Arena demo: request-scoped scratch memory ===\n\n";
  std::cout << "This demo uses the arena as temporary working memory.\n";
  std::cout << "Each request allocates scratch buffers, computes a result,\n";
  std::cout << "then releases all request memory with a single rewind.\n\n";

  for (size_t i = 0; i < sizeof(requests) / sizeof(requests[0]); ++i)
  {
    std::cout << "=== Request " << (i + 1) << " ===\n";
    process_request(arena, requests[i]);
    std::cout << '\n';
  }

  return 0;
}