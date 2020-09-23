#include "continuation.h"

#include "value.h"
#include "Environment.h"

RightThenAddContinuation::RightThenAddContinuation(PTR(Expression) rhs,
                                                   PTR(Environment) env,
                                                   PTR(Continuation) rest) {
    this->rhs = rhs;
    this->env = env;
    this->rest = rest;
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
    this->lhsVal = lhsVal;
    this->rest = rest;

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
    this->rhs = rhs;
    this->env = env;
    this->rest = rest;
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
    this->lhsVal = lhsVal;
    this->rest = rest;

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
    this->thenPart = thenPart;
    this->elsePart = elsePart;
    this->env = env;
    this->rest = rest;

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
    this->var = var;
    this->body = body;
    this->env = env;
    this->rest = rest;

}

ArgThenCallCont::ArgThenCallCont(PTR(Expression) actualArg,
                                 PTR(Environment) env,
                                 PTR(Continuation) rest) {
    this->actualArg = actualArg;
    this->env = env;
    this->rest = rest;
}

void ArgThenCallCont::stepContinue() {
    Step::mode = Step::InterpMode;
    Step::expr = actualArg;
    Step::env = env;
    Step::cont = NEW(CallCont)(Step::val, rest);
}

CallCont::CallCont(PTR(Value) toBeCalledVal,
                   PTR(Continuation) rest) {
    this->toBeCalledVal = toBeCalledVal;
    this->rest = rest;
}

void CallCont::stepContinue() {
    Step::mode = Step::ContinueMode;
    PTR(FunctionValue) functionValueToBeCalledVal = CAST(FunctionValue)(toBeCalledVal);
    functionValueToBeCalledVal->callStep(Step::val, rest);
}

RightThenCompContinuation::RightThenCompContinuation(PTR(Expression) rhs,
                                                     PTR(Environment) env,
                                                     PTR(Continuation) rest) {
    this->rhs = rhs;
    this->env = env;
    this->rest = rest;

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
    this->lhsVal = lhsVal;
    this->rest = rest;

}

void CompContinuation::stepContinue() {
    PTR(Value) rhsVal = Step::val;
    Step::mode = Step::ContinueMode;
    Step::val = NEW(BooleanValue)(lhsVal->equals(rhsVal));
    Step::cont = rest;
}
