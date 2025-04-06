#ifndef TOY_LANG_VISITOR
#define TOY_LANG_VISITOR

class NodeAST;
class ExprAST;
class StatementAST;
class FunctionDefAST;
class NumberAST;
class IdentifierAST;
class BinaryOpAST;
class TernaryExprAST;
class ReturnStmtAST;
class AssignmentAST;
class FunctionCallAST;


class Visitor {
public:
    virtual ~Visitor() = default;
    

    virtual void visit(ExprAST& expr) = 0;
    virtual void visit(NumberAST& number) = 0;
    virtual void visit(IdentifierAST& identifier) = 0;
    virtual void visit(BinaryOpAST& binary) = 0;
    virtual void visit(TernaryExprAST& ternary) = 0;
    virtual void visit(FunctionCallAST& call) = 0;
    

    virtual void visit(StatementAST& stmt) = 0;
    virtual void visit(AssignmentAST& assignment) = 0;
    virtual void visit(ReturnStmtAST& returnStmt) = 0;
    

    virtual void visit(FunctionDefAST& functionDef) = 0;
};

#endif