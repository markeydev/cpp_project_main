#include "parser.h"
#include "tokenzier.h"
#include "error.h"
#include <sstream>

Parser::Parser(Tokenizer* tokenizer) : tokenizer(tokenizer) {}

std::vector<std::unique_ptr<FunctionDefAST>> Parser::parseProgram() {
    std::vector<std::unique_ptr<FunctionDefAST>> functions;
    
    while (!tokenizer->IsEnd()) {
        while (!tokenizer->IsEnd() && 
               std::holds_alternative<UtilityTokens>(tokenizer->GetToken()) && 
               std::get<UtilityTokens>(tokenizer->GetToken()) == UtilityTokens::NEWLINE) {
            tokenizer->Next();
        }
        
        if (tokenizer->IsEnd()) {
            break;
        }
        
        Token token = tokenizer->GetToken();
        
        if (std::holds_alternative<UtilityTokens>(token) && 
            std::get<UtilityTokens>(token) == UtilityTokens::DEF) {
            functions.push_back(parseFunctionDef());
        } else {
            std::stringstream ss;
            ss << "Expected function definition or newline";
            throw SyntaxError(ss.str());
        }
    }
    
    return functions;
}

std::unique_ptr<FunctionDefAST> Parser::parseFunctionDef() {
    if (!std::holds_alternative<UtilityTokens>(tokenizer->GetToken()) || 
        std::get<UtilityTokens>(tokenizer->GetToken()) != UtilityTokens::DEF) {
        throw SyntaxError("Expected 'def' keyword");
    }
    tokenizer->Next();
    
    if (!std::holds_alternative<SymbolToken>(tokenizer->GetToken())) {
        throw SyntaxError("Expected function name after 'def'");
    }
    std::string name = std::get<SymbolToken>(tokenizer->GetToken()).name;
    tokenizer->Next();
    
    if (!std::holds_alternative<EmbracingToken>(tokenizer->GetToken()) || 
        std::get<EmbracingToken>(tokenizer->GetToken()) != EmbracingToken::LPAREN) {
        throw SyntaxError("Expected '(' after function name");
    }
    tokenizer->Next();
    
    std::vector<std::string> params;
    
    if (!std::holds_alternative<EmbracingToken>(tokenizer->GetToken()) || 
        std::get<EmbracingToken>(tokenizer->GetToken()) != EmbracingToken::RPAREN) {
        
        if (!std::holds_alternative<SymbolToken>(tokenizer->GetToken())) {
            throw SyntaxError("Expected parameter name");
        }
        params.push_back(std::get<SymbolToken>(tokenizer->GetToken()).name);
        tokenizer->Next();
        
        while (std::holds_alternative<EmbracingToken>(tokenizer->GetToken()) && 
               std::get<EmbracingToken>(tokenizer->GetToken()) == EmbracingToken::COMMA) {
            tokenizer->Next();
            
            if (!std::holds_alternative<SymbolToken>(tokenizer->GetToken())) {
                throw SyntaxError("Expected parameter name after ','");
            }
            params.push_back(std::get<SymbolToken>(tokenizer->GetToken()).name);
            tokenizer->Next();
        }
    }
    
    if (!std::holds_alternative<EmbracingToken>(tokenizer->GetToken()) || 
        std::get<EmbracingToken>(tokenizer->GetToken()) != EmbracingToken::RPAREN) {
        throw SyntaxError("Expected ')' after parameters");
    }
    tokenizer->Next();
    
    if (!std::holds_alternative<UtilityTokens>(tokenizer->GetToken()) || 
        std::get<UtilityTokens>(tokenizer->GetToken()) != UtilityTokens::NEWLINE) {
        throw SyntaxError("Expected newline after function declaration");
    }
    tokenizer->Next();
    
    std::vector<std::unique_ptr<StatementAST>> body;
    std::unique_ptr<ExprAST> return_expr = nullptr;
    
    bool foundReturn = false;
    
    while (!tokenizer->IsEnd()) {
        while (!tokenizer->IsEnd() && 
               std::holds_alternative<UtilityTokens>(tokenizer->GetToken()) && 
               std::get<UtilityTokens>(tokenizer->GetToken()) == UtilityTokens::NEWLINE) {
            tokenizer->Next();
        }
        
        if (tokenizer->IsEnd()) {
            break;
        }
        
        if (std::holds_alternative<UtilityTokens>(tokenizer->GetToken()) && 
            std::get<UtilityTokens>(tokenizer->GetToken()) == UtilityTokens::DEF) {
            auto nested_func = parseFunctionDef();
            body.push_back(std::move(nested_func));
            continue;
        }
        
        if (std::holds_alternative<UtilityTokens>(tokenizer->GetToken()) && 
            std::get<UtilityTokens>(tokenizer->GetToken()) == UtilityTokens::RETURN) {
            tokenizer->Next();
            return_expr = parseExpression();
            foundReturn = true;
            
            if (!std::holds_alternative<UtilityTokens>(tokenizer->GetToken()) || 
                (std::get<UtilityTokens>(tokenizer->GetToken()) != UtilityTokens::NEWLINE && 
                 std::get<UtilityTokens>(tokenizer->GetToken()) != UtilityTokens::EOFT)) {
                throw SyntaxError("Expected newline after return statement");
            }
            if (!tokenizer->IsEnd()) {
                tokenizer->Next();
            }
            break;
        }
        
        body.push_back(parseStatement());
        
        if (!std::holds_alternative<UtilityTokens>(tokenizer->GetToken()) || 
            std::get<UtilityTokens>(tokenizer->GetToken()) != UtilityTokens::NEWLINE) {
            throw SyntaxError("Expected newline after statement");
        }
        tokenizer->Next();
    }
    
    if (!foundReturn || !return_expr) {
        throw SyntaxError("Function must end with a return statement");
    }
    
    return std::make_unique<FunctionDefAST>(name, std::move(params), std::move(body), std::move(return_expr));
}

std::unique_ptr<StatementAST> Parser::parseStatement() {
    if (std::holds_alternative<UtilityTokens>(tokenizer->GetToken()) && 
        std::get<UtilityTokens>(tokenizer->GetToken()) == UtilityTokens::RETURN) {
        tokenizer->Next();
        auto expr = parseExpression();
        return std::make_unique<ReturnStmtAST>(std::move(expr));
    }
    
    if (std::holds_alternative<SymbolToken>(tokenizer->GetToken())) {
        std::string name = std::get<SymbolToken>(tokenizer->GetToken()).name;
        tokenizer->Next();
        
        if (!std::holds_alternative<OperatorToken>(tokenizer->GetToken()) || 
            std::get<OperatorToken>(tokenizer->GetToken()) != OperatorToken::EQ) {
            throw SyntaxError("Expected '=' after variable name");
        }
        tokenizer->Next();
        
        auto expr = parseExpression();
        return std::make_unique<AssignmentAST>(name, std::move(expr));
    }
    
    throw SyntaxError("Expected statement");
}

std::unique_ptr<ExprAST> Parser::parseExpression() {
    return parseTernaryExpr();
}

std::unique_ptr<ExprAST> Parser::parseTernaryExpr() {
    if (std::holds_alternative<EmbracingToken>(tokenizer->GetToken()) && 
        std::get<EmbracingToken>(tokenizer->GetToken()) == EmbracingToken::IF) {
        tokenizer->Next();
        
        auto condition = parseLogicalExpr();
        
        if (!std::holds_alternative<EmbracingToken>(tokenizer->GetToken()) || 
            std::get<EmbracingToken>(tokenizer->GetToken()) != EmbracingToken::THEN) {
            throw SyntaxError("Expected 'then' after condition");
        }
        tokenizer->Next();
        
        auto then_expr = parseLogicalExpr();
        
        if (!std::holds_alternative<EmbracingToken>(tokenizer->GetToken()) || 
            std::get<EmbracingToken>(tokenizer->GetToken()) != EmbracingToken::ELSE) {
            throw SyntaxError("Expected 'else' after then expression");
        }
        tokenizer->Next();
        
        auto else_expr = parseLogicalExpr();
        
        return std::make_unique<TernaryExprAST>(
            std::move(condition), std::move(then_expr), std::move(else_expr));
    }
    
    return parseLogicalExpr();
}

std::unique_ptr<ExprAST> Parser::parseLogicalExpr() {
    auto expr = parseAddExpr();
    
    while (std::holds_alternative<OperatorToken>(tokenizer->GetToken()) && 
           (std::get<OperatorToken>(tokenizer->GetToken()) == OperatorToken::EQ_EQ || 
            std::get<OperatorToken>(tokenizer->GetToken()) == OperatorToken::NOT_EQ || 
            std::get<OperatorToken>(tokenizer->GetToken()) == OperatorToken::LESS)) {
        
        char op;
        if (std::get<OperatorToken>(tokenizer->GetToken()) == OperatorToken::EQ_EQ) {
            op = '=';
        } else if (std::get<OperatorToken>(tokenizer->GetToken()) == OperatorToken::NOT_EQ) {
            op = '!';
        } else {
            op = '<';
        }
        
        tokenizer->Next();
        auto right = parseAddExpr();
        expr = std::make_unique<BinaryOpAST>(op, std::move(expr), std::move(right));
    }
    
    return expr;
}

std::unique_ptr<ExprAST> Parser::parseAddExpr() {
    auto expr = parseMulExpr();
    
    while (std::holds_alternative<OperatorToken>(tokenizer->GetToken()) && 
           (std::get<OperatorToken>(tokenizer->GetToken()) == OperatorToken::PLUS || 
            std::get<OperatorToken>(tokenizer->GetToken()) == OperatorToken::MINUS)) {
        
        char op = (std::get<OperatorToken>(tokenizer->GetToken()) == OperatorToken::PLUS) ? '+' : '-';
        tokenizer->Next();
        auto right = parseMulExpr();
        expr = std::make_unique<BinaryOpAST>(op, std::move(expr), std::move(right));
    }
    
    return expr;
}

std::unique_ptr<ExprAST> Parser::parseMulExpr() {
    auto expr = parsePrimary();
    
    while (std::holds_alternative<OperatorToken>(tokenizer->GetToken()) && 
           (std::get<OperatorToken>(tokenizer->GetToken()) == OperatorToken::MULTIPLY || 
            std::get<OperatorToken>(tokenizer->GetToken()) == OperatorToken::DIVIDE)) {
        
        char op = (std::get<OperatorToken>(tokenizer->GetToken()) == OperatorToken::MULTIPLY) ? '*' : '/';
        tokenizer->Next();
        auto right = parsePrimary();
        expr = std::make_unique<BinaryOpAST>(op, std::move(expr), std::move(right));
    }
    
    return expr;
}

std::unique_ptr<ExprAST> Parser::parsePrimary() {
    if (std::holds_alternative<ConstantToken>(tokenizer->GetToken())) {
        int value = std::get<ConstantToken>(tokenizer->GetToken()).value;
        tokenizer->Next();
        return std::make_unique<NumberAST>(value);
    }
    
    if (std::holds_alternative<SymbolToken>(tokenizer->GetToken())) {
        std::string name = std::get<SymbolToken>(tokenizer->GetToken()).name;
        tokenizer->Next();
        
        if (std::holds_alternative<EmbracingToken>(tokenizer->GetToken()) && 
            std::get<EmbracingToken>(tokenizer->GetToken()) == EmbracingToken::LPAREN) {
            tokenizer->Next();
            
            std::vector<std::unique_ptr<ExprAST>> args;
            
            if (!std::holds_alternative<EmbracingToken>(tokenizer->GetToken()) || 
                std::get<EmbracingToken>(tokenizer->GetToken()) != EmbracingToken::RPAREN) {
                
                args.push_back(parseExpression());
                
                while (std::holds_alternative<EmbracingToken>(tokenizer->GetToken()) && 
                       std::get<EmbracingToken>(tokenizer->GetToken()) == EmbracingToken::COMMA) {
                    tokenizer->Next();
                    args.push_back(parseExpression());
                }
            }
            
            if (!std::holds_alternative<EmbracingToken>(tokenizer->GetToken()) || 
                std::get<EmbracingToken>(tokenizer->GetToken()) != EmbracingToken::RPAREN) {
                throw SyntaxError("Expected ')' after function arguments");
            }
            tokenizer->Next();
            
            return std::make_unique<FunctionCallAST>(name, std::move(args));
        }
        
        return std::make_unique<IdentifierAST>(name);
    }
    
    if (std::holds_alternative<EmbracingToken>(tokenizer->GetToken()) && 
        std::get<EmbracingToken>(tokenizer->GetToken()) == EmbracingToken::LPAREN) {
        tokenizer->Next();
        
        auto expr = parseExpression();
        
        if (!std::holds_alternative<EmbracingToken>(tokenizer->GetToken()) || 
            std::get<EmbracingToken>(tokenizer->GetToken()) != EmbracingToken::RPAREN) {
            throw SyntaxError("Expected ')' after expression");
        }
        tokenizer->Next();
        
        return expr;
    }
    
    throw SyntaxError("Expected expression");
}