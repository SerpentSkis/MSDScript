#pragma once

#include "pointer.h"

class Expression;

class NumberExpression;

class Environment;

class Continuation;

class Value {
public:

    virtual bool equals(PTR(Value) val) = 0;

    virtual PTR(Value) addedTo(PTR(Value) otherVal) = 0;

    virtual PTR(Value) multipliedBy(PTR(Value) otherVal) = 0;

    virtual PTR(Expression) toExpression() = 0;

    virtual PTR(Value) call(PTR(Value) argument) = 0;

    virtual bool isTrue() = 0;

    virtual std::string toString() = 0;
};

class NumberValue : public Value {
public:
    int primitiveValue;

    explicit NumberValue(int rep);

    bool equals(PTR(Value) otherVal) override;

    PTR(Value) addedTo(PTR(Value) otherVal) override;

    PTR(Value) multipliedBy(PTR(Value) otherVal) override;

    PTR(Expression) toExpression() override;

    PTR(Value) call(PTR(Value) argument) override;

    bool isTrue() override;

    std::string toString() override;
};

class BooleanValue : public Value {
public:
    bool primitiveValue;

    explicit BooleanValue(bool rep);

    bool equals(PTR(Value) otherVal) override;

    PTR(Value) addedTo(PTR(Value) otherVal) override;

    PTR(Value) multipliedBy(PTR(Value) otherVal) override;

    PTR(Expression) toExpression() override;

    PTR(Value) call(PTR(Value) argument) override;

    bool isTrue() override;

    std::string toString() override;
};

class FunctionValue : public Value {
public:
    std::string formalArg;
    PTR(Expression) body;
    PTR(Environment) env;

    explicit FunctionValue(std::string formalArg,
                           PTR(Expression) body,
                           PTR(Environment) env);

    bool equals(PTR(Value) otherVal) override;

    PTR(Value) addedTo(PTR(Value) otherVal) override;

    PTR(Value) multipliedBy(PTR(Value) otherVal) override;

    PTR(Expression) toExpression() override;

    PTR(Value) call(PTR(Value) argument) override;

    bool isTrue() override;

    std::string toString() override;

    void callStep(PTR(Value) actualArgVal, PTR(Continuation) rest);

};