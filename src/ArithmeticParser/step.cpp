#include "step.h"

#include "Environment.h"
#include "continuation.h"

Step::modeT Step::mode;
PTR(Expression) Step::expr;
PTR(Environment) Step::env;
PTR(Value) Step::val;
PTR(Continuation) Step::cont;

PTR(Continuation) Continuation::done;

PTR(Value) Step::interpBySteps(PTR(Expression) e) {
    Step::mode = Step::InterpMode;
    Step::expr = e; //this needs to be e and not be std::move(e)
    Step::env = NEW(EmptyEnv)();
    Step::val = nullptr;
    Step::cont = Continuation::done;

    while (true) {
        if (Step::mode == Step::InterpMode)
            Step::expr->stepInterpret();
        else {
            if (Step::cont == Continuation::done)
                return Step::val;
            else
                Step::cont->stepContinue();
        }
    }
}