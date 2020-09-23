#include "continuation.h"

#include <utility>

#include "value.h"
#include "Environment.h"

RightThenAddContinuation::RightThenAddContinuation(PTR(Expression) rhs,
                                                   PTR(Environment) env,
                                                   PTR(Continuation) rest) {
    this->rhs = std::move(rhs);
    this->env = std::move(env);
    this->rest = std::move(rest);
}

void RightThenAddContinuation::stepContinue() {
    PTR(Value) lhsVal = Step::val;
    Step::mode = Step::InterpMode;
    Step::expr = rhs;
    Step::env = env;
    Step::cont = NEW(AddContinuation)(lhsVal, rest);

}

AddContinuation::AddContinuation(PTR(Value) lhsVal,
                                 PTR(Continuation) rest) {
    this->lhsVal = std::move(lhsVal);
    this->rest = std::move(rest);

}

void AddContinuation::stepContinue() {
    PTR(Value) rhsVal = Step::val;
    Step::mode = Step::ContinueMode; //todo always interp mode?
    Step::val = lhsVal->addedTo(rhsVal);
    Step::cont = rest;
}

RightThenMultiplyContinuation::RightThenMultiplyContinuation(PTR(Expression) rhs,
                                                             PTR(Environment) env,
                                                             PTR(Continuation) rest) {
    this->rhs = std::move(rhs);
    this->env = std::move(env);
    this->rest = std::move(rest);
}

void RightThenMultiplyContinuation::stepContinue() {
    PTR(Value) lhsVal = Step::val;
    Step::mode = Step::InterpMode;
    Step::expr = rhs;
    Step::env = env;
    Step::cont = NEW(MultiplyContinuation)(lhsVal, rest);
}

MultiplyContinuation::MultiplyContinuation(PTR(Value) lhsVal,
                                           PTR(Continuation) rest) {
    this->lhsVal = std::move(lhsVal);
    this->rest = std::move(rest);

}

void MultiplyContinuation::stepContinue() {
    PTR(Value) rhsVal = Step::val;
    Step::mode = Step::ContinueMode;
    Step::val = lhsVal->multipliedBy(rhsVal);
    Step::cont = rest;
}

void IfBranchCont::stepContinue() {
    PTR(Value) testVal = Step::val;
    Step::mode = Step::InterpMode;
    if (testVal->isTrue())
        Step::expr = thenPart;
    else
        Step::expr = elsePart;
    Step::env = env;
    Step::cont = rest;

}

IfBranchCont::IfBranchCont(PTR(Expression) thenPart,
                           PTR(Expression) elsePart,
                           PTR(Environment) env,
                           PTR(Continuation) rest) {
    this->thenPart = std::move(thenPart);
    this->elsePart = std::move(elsePart);
    this->env = std::move(env);
    this->rest = std::move(rest);

}

void LetBodyCont::stepContinue() {
    Step::mode = Step::InterpMode;
    Step::expr = body;
    Step::env = NEW(ExtendedEnv)(var,
                                 Step::val,
                                 env);
    Step::cont = rest;

}

LetBodyCont::LetBodyCont(std::string var,
                         PTR(Expression) body,
                         PTR(Environment) env,
                         PTR(Continuation) rest) {
    this->var = std::move(var);
    this->body = std::move(body);
    this->env = std::move(env);
    this->rest = std::move(rest);

}

ArgThenCallCont::ArgThenCallCont(PTR(Expression) actualArg,
                                 PTR(Environment) env,
                                 PTR(Continuation) rest) {
    this->actualArg = std::move(actualArg);
    this->env = std::move(env);
    this->rest = std::move(rest);
}

void ArgThenCallCont::stepContinue() {
    Step::mode = Step::InterpMode;
    Step::expr = actualArg;
    Step::env = env;
    Step::cont = NEW(CallCont)(Step::val, rest);
}

CallCont::CallCont(PTR(Value) toBeCalledVal,
                   PTR(Continuation) rest) {
    this->toBeCalledVal = std::move(toBeCalledVal);
    this->rest = std::move(rest);
}

void CallCont::stepContinue() {
    Step::mode = Step::ContinueMode;
    PTR(FunctionValue) functionValueToBeCalledVal = CAST(FunctionValue)(toBeCalledVal);
    functionValueToBeCalledVal->callStep(Step::val, rest);
}

RightThenCompContinuation::RightThenCompContinuation(PTR(Expression) rhs,
                                                     PTR(Environment) env,
                                                     PTR(Continuation) rest) {
    this->rhs = std::move(rhs);
    this->env = std::move(env);
    this->rest = std::move(rest);

}

void RightThenCompContinuation::stepContinue() {
    PTR(Value) lhsVal = Step::val;
    Step::mode = Step::InterpMode;
    Step::expr = rhs;
    Step::env = env;
    Step::cont = NEW(CompContinuation)(lhsVal, rest);
}

CompContinuation::CompContinuation(PTR(Value) lhsVal,
                                   PTR(Continuation) rest) {
    this->lhsVal = std::move(lhsVal);
    this->rest = std::move(rest);

}

void CompContinuation::stepContinue() {
    PTR(Value) rhsVal = Step::val;
    Step::mode = Step::ContinueMode;
    Step::val = NEW(BooleanValue)(lhsVal->equals(rhsVal));
    Step::cont = rest;
}
