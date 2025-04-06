#ifndef TOY_LANG_INTERPRETER
#define TOY_LANG_INTERPRETER

#include <istream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include "visitor.h"
#include "parser.h"
#include "tokenzier.h"


class Environment;


class Value {
public:
    virtual ~Value() = default;
    virtual int asInt() const = 0;
};

class IntValue : public Value {
    int value;
public:
    IntValue(int val) : value(val) {}
    int asInt() const override { return value; }
};


class Evaluator : public Visitor {
    Environment& env;
    std::unique_ptr<Value> result;

public:
    Evaluator(Environment& env);
    
    std::unique_ptr<Value> evaluate(NodeAST* node);
    
    void visit(ExprAST& expr) override;
    void visit(NumberAST& number) override;
    void visit(IdentifierAST& identifier) override;
    void visit(BinaryOpAST& binary) override;
    void visit(TernaryExprAST& ternary) override;
    void visit(FunctionCallAST& call) override;
    void visit(StatementAST& stmt) override;
    void visit(AssignmentAST& assignment) override;
    void visit(ReturnStmtAST& returnStmt) override;
    void visit(FunctionDefAST& functionDef) override;
};


class Environment {
    std::map<std::string, std::unique_ptr<Value>> variables;
    std::map<std::string, std::unique_ptr<FunctionDefAST>> functions;
    Environment* parent;
    
public:
    Environment(Environment* parent = nullptr);
    
    void defineVariable(const std::string& name, std::unique_ptr<Value> value);
    Value* getVariable(const std::string& name);
    
    void defineFunction(const std::string& name, std::unique_ptr<FunctionDefAST> func);
    FunctionDefAST* getFunction(const std::string& name);
    
    std::unique_ptr<Environment> createChildEnv();
};

class Interpreter {
    std::unique_ptr<Environment> global_env;
    std::vector<std::unique_ptr<FunctionDefAST>> functions;
    
public:
    Interpreter(std::istream& input);
    
    int run(const std::string& function_name, std::vector<int> args);
};

#endif