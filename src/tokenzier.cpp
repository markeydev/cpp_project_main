#include "tokenzier.h"
#include "error.h"
#include <cctype>

Tokenizer::Tokenizer(std::istream* in) : in_(in), current_token_(UtilityTokens::EOFT) {
    Next();
}

bool Tokenizer::IsEnd() {
    return std::holds_alternative<UtilityTokens>(current_token_) && 
           std::get<UtilityTokens>(current_token_) == UtilityTokens::EOFT;
}

Token Tokenizer::GetToken() {
    return current_token_;
}

void Tokenizer::Next() {
    SkipWhitespace();
    
    if (in_->eof()) {
        current_token_ = UtilityTokens::EOFT;
        return;
    }
    
    int c = in_->peek();
    
    if (c == '\n') {
        in_->get();
        current_token_ = UtilityTokens::NEWLINE;
        return;
    }
    
    if (std::isdigit(c)) {
        current_token_ = ReadNumber();
        return;
    }
    
    if (std::isalpha(c) || c == '_') {
        std::string identifier = ReadIdentifier();
        
        if (identifier == "def") {
            current_token_ = UtilityTokens::DEF;
        } else if (identifier == "return") {
            current_token_ = UtilityTokens::RETURN;
        } else if (identifier == "if") {
            current_token_ = EmbracingToken::IF;
        } else if (identifier == "then") {
            current_token_ = EmbracingToken::THEN;
        } else if (identifier == "else") {
            current_token_ = EmbracingToken::ELSE;
        } else {
            current_token_ = SymbolToken{identifier};
        }
        
        return;
    }
    
    switch (c) {
        case '(':
            in_->get();
            current_token_ = EmbracingToken::LPAREN;
            return;
        case ')':
            in_->get();
            current_token_ = EmbracingToken::RPAREN;
            return;
        case ',':
            in_->get();
            current_token_ = EmbracingToken::COMMA;
            return;
        case '+':
            in_->get();
            current_token_ = OperatorToken::PLUS;
            return;
        case '-':
            in_->get();
            current_token_ = OperatorToken::MINUS;
            return;
        case '*':
            in_->get();
            current_token_ = OperatorToken::MULTIPLY;
            return;
        case '/':
            in_->get();
            current_token_ = OperatorToken::DIVIDE;
            return;
        case '<':
            in_->get();
            current_token_ = OperatorToken::LESS;
            return;
        case '=':
            in_->get();
            if (in_->peek() == '=') {
                in_->get();
                current_token_ = OperatorToken::EQ_EQ;
            } else {
                current_token_ = OperatorToken::EQ;
            }
            return;
        case '!':
            in_->get();
            if (in_->peek() == '=') {
                in_->get();
                current_token_ = OperatorToken::NOT_EQ;
            } else {
                throw SyntaxError("Expected '=' after '!'");
            }
            return;
        default:
            in_->get();
            throw SyntaxError("Unknown character: " + std::string(1, static_cast<char>(c)));
    }
}

void Tokenizer::SkipWhitespace() {
    while (!in_->eof() && (std::isspace(in_->peek()) && in_->peek() != '\n')) {
        in_->get();
    }
    
    if (in_->peek() == '#') {
        while (!in_->eof() && in_->peek() != '\n') {
            in_->get();
        }
    }
}

Token Tokenizer::ReadNumber() {
    std::string num;
    
    while (!in_->eof() && std::isdigit(in_->peek())) {
        num += static_cast<char>(in_->get());
    }
    
    try {
        int value = std::stoi(num);
        return ConstantToken{value};
    } catch (const std::exception&) {
        throw SyntaxError("Invalid number: " + num);
    }
}

std::string Tokenizer::ReadIdentifier() {
    std::string identifier;
    
    while (!in_->eof() && (std::isalnum(in_->peek()) || in_->peek() == '_')) {
        identifier += static_cast<char>(in_->get());
    }
    
    return identifier;
}