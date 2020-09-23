#pragma once

#include "pointer.h"
#include "value.h"
#include "step.h"

class Environment;

class Expression;

class Expression ENABLE_THIS(Expression) {
public:
    virtual bool equals(PTR(Expression) expression) = 0;

    virtual PTR(Value) interpret(PTR(Environment) environment) = 0;

    virtual PTR(Expression) substitute(std::string input, PTR(Value) value) = 0;

    virtual bool hasVariable() = 0;

    virtual std::string toString() = 0;

    virtual PTR(Expression) optimize() = 0;

    /**
     * To take one env in the computation starting
       with this expression, reading from the registers
       in `Step` and updating them to indicate the next
       env. The `Step::expr` register will contain
       this expression (so it's uninteresting), and
       the `Step::cont` register will contain the
       continuation that expect this expression's
       result. The `Step::val` register is unspecified
       (i.e., must not be used by this method).
     */
    virtual void stepInterpret() = 0;

};

class NumberExpression : public Expression {
public:
    int primitiveValue;

    explicit NumberExpression(int value);

    bool equals(PTR(Expression) expression) override;

    PTR(Value) interpret(PTR(Environment) environment) override;

    PTR(Expression) substitute(std::string variable, PTR(Value) variableEquals) override;

    bool hasVariable() override;

    std::string toString() override;

    PTR(Expression) optimize() override;

    void stepInterpret() override;
};

class AddExpression : public Expression {
public:
    PTR(Expression) leftExpression;
    PTR(Expression) rightExpression;

    explicit AddExpression(PTR(Expression) leftExpression, PTR(Expression) rightExpression);

    bool equals(PTR(Expression) expression) override;

    PTR(Value) interpret(PTR(Environment) environment) override;

    PTR(Expression) substitute(std::string var, PTR(Value) val) override;

    bool hasVariable() override;

    std::string toString() override;

    PTR(Expression) optimize() override;

    void stepInterpret() override;
};

class MultiplyExpression : public Expression {
public:
    PTR(Expression) leftExpression;
    PTR(Expression) rightExpression;

    explicit MultiplyExpression(PTR(Expression) leftExpression, PTR(Expression) rightExpression);

    bool equals(PTR(Expression) expression) override;

    PTR(Value) interpret(PTR(Environment) environment) override;

    PTR(Expression) substitute(std::string var, PTR(Value) val) override;

    bool hasVariable() override;

    std::string toString() override;

    PTR(Expression) optimize() override;

    void stepInterpret() override;
};

class VariableExpression : public Expression {
public:
    std::string name;

    explicit VariableExpression(std::string name);

    bool equals(PTR(Expression) expression) override;

    PTR(Value) interpret(PTR(Environment) environment) override;

    PTR(Expression) substitute(std::string var, PTR(Value) val) override;

    bool hasVariable() override;

    std::string toString() override;

    PTR(Expression) optimize() override;

    void stepInterpret() override;
};

class LetExpression : public Expression {
public:
    std::string var;
    PTR(Expression) rhs;
    PTR(Expression) body;

    explicit LetExpression(std::string var, PTR(Expression) rhs, PTR(Expression) body);

    bool equals(PTR(Expression) pExpression) override;

    PTR(Value) interpret(PTR(Environment) environment) override;

    PTR(Expression) substitute(std::string newVar, PTR(Value) val) override;

    bool hasVariable() override;

    std::string toString() override;

    PTR(Expression) optimize() override;

    void stepInterpret() override;
};

class BooleanExpression : public Expression {
public:
    bool truthValue;

    explicit BooleanExpression(bool truthValue);

    bool equals(PTR(Expression) expression) override;

    PTR(Value) interpret(PTR(Environment) environment) override;

    PTR(Expression) substitute(std::string var, PTR(Value) val) override;

    bool hasVariable() override;

    std::string toString() override;

    PTR(Expression) optimize() override;

    void stepInterpret() override;
};

class IfExpression : public Expression {
public:
    PTR(Expression) testPart;
    PTR(Expression) thenResult;
    PTR(Expression) elseResult;

    IfExpression(PTR(Expression) ifCondition, PTR(Expression) thenResult, PTR(Expression) elseResult);

    bool equals(PTR(Expression) expression) override;

    PTR(Value) interpret(PTR(Environment) environment) override;

    PTR(Expression) substitute(std::string var, PTR(Value) val) override;

    bool hasVariable() override;

    std::string toString() override;

    PTR(Expression) optimize() override;

    void stepInterpret() override;
};

class EqualsExpression : public Expression {
public:
    PTR(Expression) leftExpression;
    PTR(Expression) rightExpression;

    EqualsExpression(PTR(Expression) leftExpression, PTR(Expression) rightExpression);

    bool equals(PTR(Expression) expression) override;

    PTR(Value) interpret(PTR(Environment) environment) override;

    PTR(Expression) substitute(std::string var, PTR(Value) val) override;

    bool hasVariable() override;

    std::string toString() override;

    PTR(Expression) optimize() override;

    void stepInterpret() override;
};

class FunctionExpression : public Expression {
public:
    std::string formalArg;
    PTR(Expression) body;

    FunctionExpression(std::string formalArg, PTR(Expression) body);

    bool equals(PTR(Expression) expression) override;

    PTR(Value) interpret(PTR(Environment) env) override;

    PTR(Expression) substitute(std::string var, PTR(Value) val) override;

    bool hasVariable() override;

    std::string toString() override;

    PTR(Expression) optimize() override;

    void stepInterpret() override;
};

class CallExpression : public Expression {
public:
    PTR(Expression) toBeCalled;
    PTR(Expression) actualArg;

    CallExpression(PTR(Expression) functionExpression, PTR(Expression) argumentExpression);

    bool equals(PTR(Expression) expression) override;

    PTR(Value) interpret(PTR(Environment) environment) override;

    PTR(Expression) substitute(std::string var, PTR(Value) val) override;

    bool hasVariable() override;

    std::string toString() override;

    PTR(Expression) optimize() override;

    void stepInterpret() override;
};