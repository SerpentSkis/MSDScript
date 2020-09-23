
#pragma once

#include "pointer.h"

class Expression;

class Continuation;

class Environment;

class EmptyEnv;

class Value;

class Step {
public:
    typedef enum {
        InterpMode,
        ContinueMode
    } modeT;

    static modeT mode; /* chooses the mode */

    static PTR(Expression) expr; /* for interp_mode */

    static PTR(Environment) env; /* for interp_mode */

    static PTR(Value) val; /* for ContinueMode */

    static PTR(Continuation) cont; /* all modes */

    PTR(Value)

    static interpBySteps(PTR(Expression) e);
};

