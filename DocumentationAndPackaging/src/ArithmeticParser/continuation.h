#pragma once

#include "pointer.h"
#include "step.h"
#include <utility>

class Expression;

class Value;

class Continuation;

class Environment;

class Continuation ENABLE_THIS(Continuation) {
public:
    /* To take one env in the computation starting
       with this continuation, reading from the registers
       in `Step` and updating them to indicate the next
       env. The `Step::cont` register will contain
       this continuation (so it's uninteresting), and
       the `Step::val` register will contain the value
       that this continuation was waiting form.
       The `Step::expr` register is unspecified
       (i.e., must not be used by this method). */
    virtual void stepContinue() = 0;

    static PTR(Continuation) done;
};

class RightThenAddContinuation : public Continuation {
public:
    PTR(Expression) rhs;
    PTR(Environment) env;
    PTR(Continuation) rest;

    RightThenAddContinuation(PTR(Expression) rhs, PTR(Environment) env, PTR(Continuation) rest);

    void stepContinue() override;
};

class AddContinuation : public Continuation {
public:
    PTR(Value) lhsVal;
    PTR(Continuation) rest;

    AddContinuation(PTR(Value) lhsVal, PTR(Continuation) rest);

    void stepContinue() override;
};

class RightThenMultiplyContinuation : public Continuation {
public:
    PTR(Expression) rhs;
    PTR(Environment) env;
    PTR(Continuation) rest;

    RightThenMultiplyContinuation(PTR(Expression) rhs, PTR(Environment) env, PTR(Continuation) rest);

    void stepContinue() override;
};

class MultiplyContinuation : public Continuation {
public:
    PTR(Value) lhsVal;
    PTR(Continuation) rest;

    MultiplyContinuation(PTR(Value) lhsVal, PTR(Continuation) rest);

    void stepContinue() override;
};

class RightThenCompContinuation : public Continuation {
public:
    PTR(Expression) rhs;
    PTR(Environment) env;
    PTR(Continuation) rest;

    RightThenCompContinuation(PTR(Expression) rhs, PTR(Environment) env, PTR(Continuation) rest);

    void stepContinue() override;
};

class CompContinuation : public Continuation {
public:
    PTR(Value) lhsVal;
    PTR(Continuation) rest;

    CompContinuation(PTR(Value) lhsVal, PTR(Continuation) rest);

    void stepContinue() override;
};

class IfBranchCont : public Continuation {
public:
    PTR(Expression) thenPart;
    PTR(Expression) elsePart;
    PTR(Environment) env;
    PTR(Continuation) rest;

    void stepContinue() override;

    IfBranchCont(PTR(Expression) thenPart,
                 PTR(Expression) elsePart,
                 PTR(Environment) env,
                 PTR(Continuation) rest);
};

class LetBodyCont : public Continuation {
public:
    std::string var;
    PTR(Expression) body;
    PTR(Environment) env;
    PTR(Continuation) rest;

    LetBodyCont(std::string var,
                PTR(Expression) body,
                PTR(Environment) env,
                PTR(Continuation) rest);

    void stepContinue() override;
};

class ArgThenCallCont : public Continuation {
public:
    PTR(Expression) actualArg;
    PTR(Environment) env;
    PTR(Continuation) rest;

    void stepContinue() override;

    ArgThenCallCont(PTR(Expression) actualArg,
                    PTR(Environment) env,
                    PTR(Continuation) rest);
};

class CallCont : public Continuation {
public:
    PTR(Value) toBeCalledVal;
    PTR(Continuation) rest;

    CallCont(PTR(Value) toBeCalledVal, PTR(Continuation) rest);

    void stepContinue() override;
};

