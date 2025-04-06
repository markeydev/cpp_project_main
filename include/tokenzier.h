#ifndef TOY_LANG_TOKENIZER
#define TOY_LANG_TOKENIZER

#include <istream>
#include <string>
#include <variant>

struct SymbolToken {
  std::string name;
};

struct ConstantToken {
  int value;
};


enum class EmbracingToken { LPAREN, RPAREN, COMMA, IF, THEN, ELSE };

enum class OperatorToken {
  PLUS,
  MINUS,
  MULTIPLY,
  DIVIDE,
  EQ_EQ,
  NOT_EQ,
  LESS,
  EQ
};


enum class UtilityTokens { DEF, RETURN, NEWLINE, EOFT };

using Token = std::variant<SymbolToken, ConstantToken, EmbracingToken,
                           OperatorToken, UtilityTokens>;

class Tokenizer {
private:
  std::istream* in_;
  Token current_token_;

  void SkipWhitespace();
  Token ReadNumber();
  std::string ReadIdentifier();

public:
  Tokenizer(std::istream *in);

  bool IsEnd();

  void Next();

  Token GetToken();
};

#endif