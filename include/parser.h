#ifndef TOY_LANG_PARSER
#define TOY_LANG_PARSER

#include <memory>
#include <string>
#include <vector>
#include "visitor.h"
#include "error.h"

class NodeAST {
public:
  virtual ~NodeAST() = default;
  virtual void accept(Visitor &visitor) = 0;
};

class ExprAST : public NodeAST { 
public:
  void accept(Visitor &visitor) override {
    visitor.visit(*this);
  }
};

class NumberAST : public ExprAST {
  int value;
public:
  NumberAST(int val) : value(val) {}
  int getValue() const { return value; }
  
  void accept(Visitor &visitor) override {
    visitor.visit(*this);
  }
};

class IdentifierAST : public ExprAST {
  std::string name;
public:
  IdentifierAST(const std::string& id) : name(id) {}
  const std::string& getName() const { return name; }
  
  void accept(Visitor &visitor) override {
    visitor.visit(*this);
  }
};

class BinaryOpAST : public ExprAST {
  char op;
  std::unique_ptr<ExprAST> left, right;
public:
  BinaryOpAST(char op, std::unique_ptr<ExprAST> left, std::unique_ptr<ExprAST> right)
    : op(op), left(std::move(left)), right(std::move(right)) {}
  
  char getOp() const { return op; }
  ExprAST* getLeft() const { return left.get(); }
  ExprAST* getRight() const { return right.get(); }
  
  void accept(Visitor &visitor) override {
    visitor.visit(*this);
  }
};

class TernaryExprAST : public ExprAST {
  std::unique_ptr<ExprAST> condition;
  std::unique_ptr<ExprAST> then_expr;
  std::unique_ptr<ExprAST> else_expr;
public:
  TernaryExprAST(std::unique_ptr<ExprAST> condition,
                std::unique_ptr<ExprAST> then_expr,
                std::unique_ptr<ExprAST> else_expr)
    : condition(std::move(condition)), 
      then_expr(std::move(then_expr)),
      else_expr(std::move(else_expr)) {}
  
  ExprAST* getCondition() const { return condition.get(); }
  ExprAST* getThenExpr() const { return then_expr.get(); }
  ExprAST* getElseExpr() const { return else_expr.get(); }
  
  void accept(Visitor &visitor) override {
    visitor.visit(*this);
  }
};

class FunctionCallAST : public ExprAST {
  std::string callee;
  std::vector<std::unique_ptr<ExprAST>> args;
public:
  FunctionCallAST(const std::string& callee, 
                 std::vector<std::unique_ptr<ExprAST>> args)
    : callee(callee), args(std::move(args)) {}
  
  const std::string& getCallee() const { return callee; }
  const std::vector<std::unique_ptr<ExprAST>>& getArgs() const { return args; }
  
  void accept(Visitor &visitor) override {
    visitor.visit(*this);
  }
};

class StatementAST : public NodeAST { 
public:
  void accept(Visitor &visitor) override {
    visitor.visit(*this);
  }
};

class AssignmentAST : public StatementAST {
  std::string variable;
  std::unique_ptr<ExprAST> value;
public:
  AssignmentAST(const std::string& var, std::unique_ptr<ExprAST> val)
    : variable(var), value(std::move(val)) {}
  
  const std::string& getVariable() const { return variable; }
  ExprAST* getValue() const { return value.get(); }
  
  void accept(Visitor &visitor) override {
    visitor.visit(*this);
  }
};

class ReturnStmtAST : public StatementAST {
  std::unique_ptr<ExprAST> return_expr;
public:
  ReturnStmtAST(std::unique_ptr<ExprAST> expr)
    : return_expr(std::move(expr)) {}
  
  ExprAST* getReturnExpr() const { return return_expr.get(); }
  
  void accept(Visitor &visitor) override {
    visitor.visit(*this);
  }
};

class FunctionDefAST : public StatementAST { 
private:
    std::string name;
    std::vector<std::string> params;
    std::vector<std::unique_ptr<StatementAST>> body;
    std::unique_ptr<ExprAST> return_expr;
  
public:
    FunctionDefAST(const std::string& name, 
                  std::vector<std::string> params,
                  std::vector<std::unique_ptr<StatementAST>> body,
                  std::unique_ptr<ExprAST> return_expr)
        : name(name), params(std::move(params)), 
          body(std::move(body)), return_expr(std::move(return_expr)) {}
    
    FunctionDefAST(const FunctionDefAST&) = delete;
    FunctionDefAST& operator=(const FunctionDefAST&) = delete;
    
    FunctionDefAST(FunctionDefAST&& other) noexcept 
        : name(std::move(other.name)),
          params(std::move(other.params)),
          body(std::move(other.body)),
          return_expr(std::move(other.return_expr)) {}
        
    FunctionDefAST& operator=(FunctionDefAST&& other) noexcept {
        if (this != &other) {
            name = std::move(other.name);
            params = std::move(other.params);
            body = std::move(other.body);
            return_expr = std::move(other.return_expr);
        }
        return *this;
    }
    
    const std::string& getName() const { return name; }
    const std::vector<std::string>& getParams() const { return params; }
    const std::vector<std::unique_ptr<StatementAST>>& getBody() const { return body; }
    ExprAST* getReturnExpr() const { return return_expr.get(); }
    
    void accept(Visitor &visitor) override {
        visitor.visit(*this);
    }
    
    std::unique_ptr<FunctionDefAST> clone() const {
        std::vector<std::unique_ptr<StatementAST>> cloned_body;
        cloned_body.reserve(body.size());
        
        for (const auto& stmt : body) {
            if (auto assignment = dynamic_cast<AssignmentAST*>(stmt.get())) {
                auto cloned_value = cloneExpr(assignment->getValue());
                cloned_body.push_back(
                    std::make_unique<AssignmentAST>(
                        assignment->getVariable(),
                        std::move(cloned_value)
                    )
                );
            } else if (auto return_stmt = dynamic_cast<ReturnStmtAST*>(stmt.get())) {
                auto cloned_expr = cloneExpr(return_stmt->getReturnExpr());
                cloned_body.push_back(
                    std::make_unique<ReturnStmtAST>(std::move(cloned_expr))
                );
            } else if (auto nested_func = dynamic_cast<FunctionDefAST*>(stmt.get())) {
                cloned_body.push_back(nested_func->clone());
            }
        }
        
        std::unique_ptr<ExprAST> cloned_return = nullptr;
        if (return_expr) {
            cloned_return = cloneExpr(return_expr.get());
        }
        
        return std::make_unique<FunctionDefAST>(
            name, 
            params, 
            std::move(cloned_body),
            std::move(cloned_return)
        );
    }
    
private:
    std::unique_ptr<ExprAST> cloneExpr(ExprAST* expr) const {
        if (!expr) return nullptr;
        
        if (auto number = dynamic_cast<NumberAST*>(expr)) {
            return std::make_unique<NumberAST>(number->getValue());
        } else if (auto id = dynamic_cast<IdentifierAST*>(expr)) {
            return std::make_unique<IdentifierAST>(id->getName());
        } else if (auto binary = dynamic_cast<BinaryOpAST*>(expr)) {
            return std::make_unique<BinaryOpAST>(
                binary->getOp(),
                cloneExpr(binary->getLeft()),
                cloneExpr(binary->getRight())
            );
        } else if (auto ternary = dynamic_cast<TernaryExprAST*>(expr)) {
            return std::make_unique<TernaryExprAST>(
                cloneExpr(ternary->getCondition()),
                cloneExpr(ternary->getThenExpr()),
                cloneExpr(ternary->getElseExpr())
            );
        } else if (auto call = dynamic_cast<FunctionCallAST*>(expr)) {
            std::vector<std::unique_ptr<ExprAST>> cloned_args;
            for (const auto& arg : call->getArgs()) {
                cloned_args.push_back(cloneExpr(arg.get()));
            }
            return std::make_unique<FunctionCallAST>(
                call->getCallee(),
                std::move(cloned_args)
            );
        }
        return nullptr;
    }
};

class Parser {
private:
    class Tokenizer* tokenizer;

public:
    Parser(class Tokenizer* tokenizer);
    
    std::vector<std::unique_ptr<FunctionDefAST>> parseProgram();
    
private:
    std::unique_ptr<FunctionDefAST> parseFunctionDef();
    std::unique_ptr<StatementAST> parseStatement();
    std::unique_ptr<ExprAST> parseExpression();
    std::unique_ptr<ExprAST> parseTernaryExpr();
    std::unique_ptr<ExprAST> parseLogicalExpr();
    std::unique_ptr<ExprAST> parseAddExpr();
    std::unique_ptr<ExprAST> parseMulExpr();
    std::unique_ptr<ExprAST> parsePrimary();
};

#endif