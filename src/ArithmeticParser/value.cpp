#include <stdexcept>
#include <utility>
#include "../catch/catch.hpp"
#include "value.h"
#include "step.h"
#include "Environment.h"
#include "expression.hpp"
#include "continuation.h"

NumberValue::NumberValue(int rep) {
    this->primitiveValue = rep;
}

bool NumberValue::equals(PTR(Value) otherVal) {
    PTR(NumberValue) otherNumVal = CAST(NumberValue)(otherVal);
    return otherNumVal == nullptr ? false : primitiveValue == otherNumVal->primitiveValue;
}

PTR(Value)NumberValue::addedTo(PTR(Value) otherVal) {
    PTR(NumberValue) otherNumVal = CAST(NumberValue)(otherVal);
    return otherNumVal == nullptr ? throw std::runtime_error("not a number") :
           NEW(NumberValue)(primitiveValue + otherNumVal->primitiveValue);
}

PTR(Value)NumberValue::multipliedBy(PTR(Value) otherVal) {
    PTR(NumberValue) otherNumVal = CAST(NumberValue)(otherVal);
    return otherNumVal == nullptr ? throw std::runtime_error("not a number") :
           NEW(NumberValue)(this->primitiveValue * otherNumVal->primitiveValue);
}

PTR(Expression)NumberValue::toExpression() {
    return NEW(NumberExpression)(this->primitiveValue);
}

PTR(Value)NumberValue::call(PTR(Value) argument) {
    throw std::runtime_error("no composing numbers");
}

bool NumberValue::isTrue() {
    if (this->primitiveValue == 1) {
        return true;
    } else if (this->primitiveValue == 0) {
        return false;
    } else {
        throw std::runtime_error("number asked to be true");
    }
}

std::string NumberValue::toString() {
    return std::to_string(this->primitiveValue);
}

BooleanValue::BooleanValue(bool rep) {
    this->primitiveValue = rep;
}

bool BooleanValue::equals(PTR(Value) otherVal) {
    PTR(BooleanValue) otherBoolVal = CAST(BooleanValue)(otherVal);
    return otherBoolVal == nullptr ? false : primitiveValue == otherBoolVal->primitiveValue;
}

PTR(Value)BooleanValue::addedTo(PTR(Value) otherVal) {
    throw std::runtime_error("no adding booleans");
}

PTR(Value)BooleanValue::multipliedBy(PTR(Value) otherVal) {
    throw std::runtime_error("no multiplying booleans");
}

PTR(Expression)BooleanValue::toExpression() {
    return NEW(BooleanExpression)(primitiveValue);
}

bool BooleanValue::isTrue() {
    return this->primitiveValue;
}

std::string BooleanValue::toString() {
    return (std::string) (primitiveValue ? "_true" : "_false");
}

PTR(Value)BooleanValue::call(PTR(Value) argument) {
    throw std::runtime_error("no composing booleans");
}

FunctionValue::FunctionValue(std::string formalArg,
                             PTR(Expression) body,
                             PTR(Environment) env) {
    this->formalArg = std::move(formalArg);
    this->body = std::move(body);
    this->env = std::move(env);
}

bool FunctionValue::equals(PTR(Value) otherVal) {
    PTR(FunctionValue) otherFunctionValue = CAST(FunctionValue)(otherVal);
    return otherFunctionValue == nullptr ? false :
           this->formalArg == otherFunctionValue->formalArg &&
           this->body->equals(otherFunctionValue->body);
}

PTR(Value)FunctionValue::addedTo(PTR(Value) otherVal) {
    throw std::runtime_error("no adding functions");
}

PTR(Value)FunctionValue::multipliedBy(PTR(Value) otherVal) {
    throw std::runtime_error("no multiplying functions");
}

PTR(Expression)FunctionValue::toExpression() {
    return NEW(FunctionExpression)(this->formalArg, this->body);
}

PTR(Value)FunctionValue::call(PTR(Value) argument) {
    return this->body->interpret(NEW(ExtendedEnv)(this->formalArg, argument, this->env));
}

bool FunctionValue::isTrue() {
    throw std::runtime_error("function was asked if true");
}

std::string FunctionValue::toString() {
    return "_fun (" + this->formalArg + ") " + this->body->toString();
}

void FunctionValue::callStep(PTR(Value) actualArgVal, PTR(Continuation) rest) {
    Step::mode = Step::InterpMode;
    Step::expr = body;
    Step::env = NEW(ExtendedEnv)(formalArg, actualArgVal, env);
    Step::cont = std::move(rest);
}

TEST_CASE("values equals") {
    CHECK(NumberValue(5).equals(NEW(NumberValue)(5)));
    CHECK(!NumberValue(7).equals(NEW(NumberValue)(5)));

    CHECK(BooleanValue(true).equals(NEW(BooleanValue)(true)));
    CHECK(!BooleanValue(true).equals(NEW(BooleanValue)(false)));
    CHECK(!BooleanValue(false).equals(NEW(BooleanValue)(true)));

    CHECK(!(NEW(NumberValue)(7))->equals(NEW(BooleanValue)(false)));
    CHECK(!(NEW(BooleanValue)(false))->equals(NEW(NumberValue)(8)));
}

TEST_CASE("add_to") {
    CHECK ((NEW(NumberValue)(5))->addedTo(NEW(NumberValue)(8))->equals(NEW(NumberValue)(13)));

    CHECK_THROWS_WITH ((NEW(NumberValue)(5))->addedTo(NEW(BooleanValue)(false)), "not a number");
    CHECK_THROWS_WITH ((NEW(BooleanValue)(false))->addedTo(NEW(BooleanValue)(false)),
                       "no adding booleans");
}

TEST_CASE("mult_with") {
    CHECK ((NEW(NumberValue)(5))->multipliedBy(NEW(NumberValue)(8))->equals(NEW(NumberValue)(40)));

    CHECK_THROWS_WITH ((NEW(NumberValue)(5))->multipliedBy(NEW(BooleanValue)(false)), "not a number");
    CHECK_THROWS_WITH ((NEW(BooleanValue)(false))->multipliedBy(NEW(BooleanValue)(false)),
                       "no multiplying booleans");
}

TEST_CASE("value to_expr") {
    CHECK((NEW(NumberValue)(5))->toExpression()->equals(NEW(NumberExpression)(5)));
    CHECK((NEW(BooleanValue)(true))->toExpression()->equals(NEW(BooleanExpression)(true)));
    CHECK((NEW(BooleanValue)(false))->toExpression()->equals(NEW(BooleanExpression)(false)));
}

TEST_CASE("value to_string") {
    CHECK(NumberValue(5).toString() == "5");
    CHECK(BooleanValue(true).toString() == "_true");
    CHECK(BooleanValue(false).toString() == "_false");
}


