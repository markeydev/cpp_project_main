#include "interpreter.h"
#include "error.h"
#include <sstream>

Evaluator::Evaluator(Environment& env) : env(env), result(nullptr) {}

std::unique_ptr<Value> Evaluator::evaluate(NodeAST* node) {
    if (!node) {
        return nullptr;
    }
    
    result = nullptr;
    
    node->accept(*this);
    
    return std::move(result);
}

void Evaluator::visit(ExprAST& expr) {
    (void)expr;
}

void Evaluator::visit(NumberAST& number) {
    result = std::make_unique<IntValue>(number.getValue());
}

void Evaluator::visit(IdentifierAST& identifier) {
    Value* val = env.getVariable(identifier.getName());
    if (!val) {
        throw NameError("Undefined variable: " + identifier.getName());
    }
    result = std::make_unique<IntValue>(val->asInt());
}

void Evaluator::visit(BinaryOpAST& binary) {
    auto leftEval = evaluate(binary.getLeft());
    auto rightEval = evaluate(binary.getRight());
    
    if (!leftEval || !rightEval) {
        throw RuntimeError("Invalid operands in binary operation");
    }
    
    int left = leftEval->asInt();
    int right = rightEval->asInt();
    int value = 0;
    

    switch (binary.getOp()) {
        case '+': value = left + right; break;
        case '-': value = left - right; break;
        case '*': value = left * right; break;
        case '/': 
            if (right == 0) {
                throw RuntimeError("Division by zero");
            }
            value = left / right; 
            break;
        case '=': value = (left == right) ? 1 : 0; break; 
        case '!': value = (left != right) ? 1 : 0; break; 
        case '<': value = (left < right) ? 1 : 0; break;
        default:
            throw RuntimeError("Unknown binary operator");
    }
    
    result = std::make_unique<IntValue>(value);
}

void Evaluator::visit(FunctionCallAST& call) {
    const std::string& callee = call.getCallee();
    auto func = env.getFunction(callee);
    
    if (!func) {
        throw NameError("Undefined function: " + callee);
    }
    
    const auto& params = func->getParams();
    const auto& args = call.getArgs();
    

    if (params.size() != args.size()) {
        throw RuntimeError("Function " + callee + " called with incorrect number of arguments");
    }
    
    auto funcEnv = env.createChildEnv();
    
 
    for (size_t i = 0; i < params.size(); i++) {
        auto argValue = evaluate(args[i].get());
        funcEnv->defineVariable(params[i], std::move(argValue));
    }
    

    Evaluator funcEvaluator(*funcEnv);
    

    for (const auto& stmt : func->getBody()) {
        funcEvaluator.evaluate(stmt.get());
    }
    

    result = funcEvaluator.evaluate(func->getReturnExpr());
}

void Evaluator::visit(StatementAST& stmt) {
    (void)stmt;
}

void Evaluator::visit(AssignmentAST& assignment) {
    auto value = evaluate(assignment.getValue());
    if (!value) {
        throw RuntimeError("Invalid expression in assignment");
    }
    
    env.defineVariable(assignment.getVariable(), std::move(value));
    result = nullptr; 
}

void Evaluator::visit(ReturnStmtAST& returnStmt) {
    result = evaluate(returnStmt.getReturnExpr());
}

void Evaluator::visit(FunctionDefAST& functionDef) {
    env.defineFunction(functionDef.getName(), functionDef.clone());
    result = nullptr; 
}


Environment::Environment(Environment* parent) : parent(parent) {}

void Environment::defineVariable(const std::string& name, std::unique_ptr<Value> value) {
    variables[name] = std::move(value);
}
void Evaluator::visit(TernaryExprAST& ternary) {
    auto conditionValue = evaluate(ternary.getCondition());
    if (!conditionValue) {
        throw RuntimeError("Invalid condition in ternary expression");
    }
    
    bool condition = conditionValue->asInt() != 0;
    
    if (condition) {
        result = evaluate(ternary.getThenExpr());
    } else {
        result = evaluate(ternary.getElseExpr());
    }
}
Value* Environment::getVariable(const std::string& name) {
    auto it = variables.find(name);
    if (it != variables.end()) {
        return it->second.get();
    }
    
    if (parent) {
        return parent->getVariable(name);
    }
    
    return nullptr;
}

void Environment::defineFunction(const std::string& name, std::unique_ptr<FunctionDefAST> func) {
    functions[name] = std::move(func); 
}

FunctionDefAST* Environment::getFunction(const std::string& name) {
    auto it = functions.find(name);
    if (it != functions.end()) {
        return it->second.get();
    }
    
    if (parent) {
        return parent->getFunction(name);
    }
    
    return nullptr;
}

std::unique_ptr<Environment> Environment::createChildEnv() {
    return std::make_unique<Environment>(this);
}


Interpreter::Interpreter(std::istream& input) : global_env(std::make_unique<Environment>()) {

    Tokenizer tokenizer(&input);
    

    Parser parser(&tokenizer);
    

    functions = parser.parseProgram();
    

    for (auto& func : functions) {
 
        std::string name = func->getName();
        std::vector<std::string> params = func->getParams(); 
        

        auto new_func = std::make_unique<FunctionDefAST>(
            name,
            std::move(params),
            std::vector<std::unique_ptr<StatementAST>>(), 
            nullptr  
        );
        
        global_env->defineFunction(func->getName(), func->clone());
    }
}

int Interpreter::run(const std::string& function_name, std::vector<int> args) {
    auto func = global_env->getFunction(function_name);
    if (!func) {
        throw NameError("Function not found: " + function_name);
    }
    
    if (func->getParams().size() != args.size()) {
        throw RuntimeError("Incorrect number of arguments for function: " + function_name);
    }
    
    auto funcEnv = global_env->createChildEnv();
    
    for (size_t i = 0; i < args.size(); i++) {
        funcEnv->defineVariable(func->getParams()[i], 
            std::make_unique<IntValue>(args[i]));
    }
    
    Evaluator evaluator(*funcEnv);
    
    for (const auto& stmt : func->getBody()) {
        evaluator.evaluate(stmt.get());
    }
    
    auto result = evaluator.evaluate(func->getReturnExpr());
    if (!result) {
        throw RuntimeError("Function did not return a value");
    }
    
    return result->asInt();
}