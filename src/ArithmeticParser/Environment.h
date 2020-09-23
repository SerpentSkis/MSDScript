#pragma once

#include "pointer.h"
#include "expression.hpp"

class Value;

class Expression;

class VariableExpression;

class Environment {
public:
    virtual PTR(Value) lookup(std::string findName) = 0;

    static PTR(Environment) empty;
};

class EmptyEnv : public Environment {
public:
    EmptyEnv();

    PTR(Value) lookup(std::string findName) override;
};

class ExtendedEnv : public Environment {
public:
    std::string name;
    PTR(Value) val;
    PTR(Environment) rest;

    PTR(Value) lookup(std::string findName) override;

    ExtendedEnv(std::string name, PTR(Value) val, PTR(Environment) rest);
};
