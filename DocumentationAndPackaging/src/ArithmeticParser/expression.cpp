#include "expression.hpp"#include "Environment.h"#include "continuation.h"#include <utility>NumberExpression::NumberExpression(int value) {    this->primitiveValue = value;}bool NumberExpression::equals(PTR(Expression) expression) {    PTR(NumberExpression) numberExpression = CAST(NumberExpression)(expression);    return numberExpression == nullptr ? false : this->primitiveValue == numberExpression->primitiveValue;}PTR(Value) NumberExpression::interpret(PTR(Environment) environment) {    return NEW(NumberValue)(this->primitiveValue);}PTR(Expression) NumberExpression::substitute(std::string variable, PTR(Value) variableEquals) {    return NEW(NumberExpression)(this->primitiveValue);}bool NumberExpression::hasVariable() {    return false;}std::string NumberExpression::toString() {    return std::to_string(this->primitiveValue);}PTR(Expression) NumberExpression::optimize() {    return NEW(NumberExpression)(this->primitiveValue);}void NumberExpression::stepInterpret() {    Step::mode = Step::ContinueMode;    Step::val = NEW(NumberValue)(primitiveValue);    Step::cont = Step::cont;}AddExpression::AddExpression(PTR(Expression) leftExpression, PTR(Expression) rightExpression) {    this->leftExpression = std::move(leftExpression);    this->rightExpression = std::move(rightExpression);}bool AddExpression::equals(PTR(Expression) expression) {    PTR(AddExpression) addExpression = CAST(AddExpression)(expression);    return addExpression == nullptr ? false : leftExpression->equals(addExpression->leftExpression)                                              && rightExpression->equals(addExpression->rightExpression);}PTR(Expression) AddExpression::substitute(std::string var, PTR(Value) val) {    return NEW(AddExpression)(leftExpression->substitute(var, val),                              rightExpression->substitute(var, val));}bool AddExpression::hasVariable() {    return this->leftExpression->hasVariable() || this->rightExpression->hasVariable();}std::string AddExpression::toString() {    return "(" + this->leftExpression->toString() + " + " + this->rightExpression->toString() + ")";}PTR(Expression) AddExpression::optimize() {    if (this->hasVariable()) {        return NEW(AddExpression)(this->leftExpression->optimize(),                                  this->rightExpression->optimize());    } else {        return this->interpret(NEW(EmptyEnv)())->toExpression();    }}PTR(Value) AddExpression::interpret(PTR(Environment) environment) {    return leftExpression->interpret(environment)->addedTo(rightExpression->interpret(environment));}void AddExpression::stepInterpret() {    Step::mode = Step::InterpMode;    Step::expr = leftExpression;    Step::cont = NEW(RightThenAddContinuation)(rightExpression, Step::env, Step::cont);}MultiplyExpression::MultiplyExpression(PTR(Expression) leftExpression, PTR(Expression) rightExpression) {    this->leftExpression = std::move(leftExpression);    this->rightExpression = std::move(rightExpression);}bool MultiplyExpression::equals(PTR(Expression) expression) {    PTR(MultiplyExpression) m = CAST(MultiplyExpression)(expression);    return m == nullptr ? false : leftExpression->equals(m->leftExpression) &&                                  rightExpression->equals(m->rightExpression);}PTR(Expression) MultiplyExpression::substitute(std::string var, PTR(Value) val) {    return NEW(MultiplyExpression)(leftExpression->substitute(var, val),                                   rightExpression->substitute(var, val));}bool MultiplyExpression::hasVariable() {    return leftExpression->hasVariable() || rightExpression->hasVariable();}std::string MultiplyExpression::toString() {    return "(" + this->leftExpression->toString() + " * " + this->rightExpression->toString() + ")";}PTR(Expression) MultiplyExpression::optimize() {    if (this->hasVariable()) {        return NEW(MultiplyExpression)(this->leftExpression->optimize(),                                       this->rightExpression->optimize());    } else {        return this->interpret(NEW(EmptyEnv)())->toExpression();    }}PTR(Value) MultiplyExpression::interpret(PTR(Environment) environment) {    return leftExpression->interpret(environment)->multipliedBy(rightExpression->interpret(environment));}void MultiplyExpression::stepInterpret() {    Step::mode = Step::InterpMode;    Step::expr = leftExpression;    Step::cont = NEW(RightThenMultiplyContinuation)(rightExpression, Step::env, Step::cont);}VariableExpression::VariableExpression(std::string name) {    this->name = std::move(name);}bool VariableExpression::equals(PTR(Expression) expression) {    PTR(VariableExpression) variableExpression = CAST(VariableExpression)(expression);    return variableExpression == nullptr ? false : this->name == variableExpression->name;}/** * warner said so * @param env * @return */PTR(Value) VariableExpression::interpret(PTR(Environment) env) {    return env->lookup(name);}bool VariableExpression::hasVariable() {    return true;}PTR(Expression) VariableExpression::substitute(std::string var, PTR(Value) val) {    if (this->name == var) {        return val->toExpression();    } else {        return NEW(VariableExpression)(this->name);    }}std::string VariableExpression::toString() {    return this->name;}PTR(Expression) VariableExpression::optimize() {    return NEW(VariableExpression)(this->name);}void VariableExpression::stepInterpret() {    Step::mode = Step::ContinueMode;    Step::val = Step::env->lookup(name);    Step::cont = Step::cont;}LetExpression::LetExpression(std::string var, PTR(Expression) rhs, PTR(Expression) body) {    this->var = std::move(var);    this->rhs = std::move(rhs);    this->body = std::move(body);}bool LetExpression::equals(PTR(Expression) pExpression) {    PTR(LetExpression) letExpression = CAST(LetExpression)(pExpression);    return letExpression == nullptr ? false : this->var == letExpression->var &&                                              this->rhs->equals(letExpression->rhs) &&                                              this->body->equals(letExpression->body);}PTR(Value) LetExpression::interpret(PTR(Environment) environment) {    PTR(Value) rhsVal = this->rhs->interpret(environment);    PTR(Environment) newEnv = NEW(ExtendedEnv)(this->var, rhsVal, environment);    return this->body->interpret(newEnv);}PTR(Expression)LetExpression::substitute(std::string newVar, PTR(Value) val) {    if (newVar == this->var) {        return NEW(LetExpression)(this->var,                                  this->rhs->substitute(newVar, val),                                  this->body);    } else {        return NEW(LetExpression)(this->var,                                  this->rhs->substitute(newVar, val),                                  this->body->substitute(newVar, val));    }}bool LetExpression::hasVariable() {    return this->rhs->hasVariable() || this->body->hasVariable();}std::string LetExpression::toString() {    return "_let " + this->var + " = " + this->rhs->toString() +           " _in (" + this->body->toString() + ")";}PTR(Expression)LetExpression::optimize() {    if (rhs->hasVariable()) {        return NEW(LetExpression)(this->var,                                  this->rhs->optimize(),                                  this->body->optimize());    } else {        return this->body->optimize()->substitute(                this->var,                this->rhs->interpret(NEW(EmptyEnv)())        )->optimize();    }}void LetExpression::stepInterpret() {    Step::mode = Step::InterpMode;    Step::expr = rhs;    Step::env = Step::env;    Step::cont = NEW(LetBodyCont)(var, body, Step::env, Step::cont);}BooleanExpression::BooleanExpression(bool truthValue) {    this->truthValue = truthValue;}bool BooleanExpression::equals(PTR(Expression) expression) {    PTR(BooleanExpression) boolean = CAST(BooleanExpression)(expression);    return boolean == nullptr ? false : boolean->truthValue == this->truthValue;}PTR(Value) BooleanExpression::interpret(PTR(Environment) environment) {    return NEW(BooleanValue)(this->truthValue);}PTR(Expression)BooleanExpression::substitute(std::string var, PTR(Value) val) {    return NEW(BooleanExpression)(this->truthValue);}bool BooleanExpression::hasVariable() {    return false;}std::string BooleanExpression::toString() {    return this->truthValue ? "_true" : "_false";}PTR(Expression)BooleanExpression::optimize() {    return NEW(BooleanExpression)(this->truthValue);}void BooleanExpression::stepInterpret() {    Step::mode = Step::ContinueMode;    Step::val = NEW(BooleanValue)(this->truthValue);    Step::cont = Step::cont; /* no-op */}IfExpression::IfExpression(PTR(Expression) ifCondition, PTR(Expression) thenResult, PTR(Expression) elseResult) {    this->testPart = std::move(ifCondition);    this->thenResult = std::move(thenResult);    this->elseResult = std::move(elseResult);}bool IfExpression::equals(PTR(Expression) expression) {    PTR(IfExpression) ifStatement = CAST(IfExpression)(expression);    return ifStatement == nullptr ? false : ifStatement->testPart->equals(this->testPart) &&                                            ifStatement->thenResult->equals(this->thenResult) &&                                            ifStatement->elseResult->equals(this->elseResult);}PTR(Value) IfExpression::interpret(PTR(Environment) environment) {    return this->testPart->interpret(environment)->isTrue() ? this->thenResult->interpret(environment)                                                            : this->elseResult->interpret(environment);}PTR(Expression)IfExpression::substitute(std::string var, PTR(Value) val) {    return NEW(IfExpression)(this->testPart->substitute(var, val),                             this->thenResult->substitute(var, val),                             this->elseResult->substitute(var, val));}bool IfExpression::hasVariable() {    return this->testPart->hasVariable() || this->thenResult->hasVariable() || this->elseResult->hasVariable();}std::string IfExpression::toString() {    return "_if " + this->testPart->toString() + " _then " + this->thenResult->toString() +           " _else " + this->elseResult->toString();}PTR(Expression)IfExpression::optimize() {    PTR(IfExpression) optimizedIfExpression = NEW(IfExpression)(this->testPart->optimize(),                                                                this->thenResult->optimize(),                                                                this->elseResult->optimize());    if (optimizedIfExpression->testPart->hasVariable()) {        return optimizedIfExpression;    } else if (optimizedIfExpression->testPart->interpret(NEW(EmptyEnv)())->equals(NEW(BooleanValue)(true))) {        return optimizedIfExpression->thenResult;    } else {        return optimizedIfExpression->elseResult;    }}void IfExpression::stepInterpret() {    Step::mode = Step::InterpMode;    Step::expr = testPart;    Step::env = Step::env;    Step::cont = NEW(IfBranchCont)(thenResult, elseResult, Step::env, Step::cont);}EqualsExpression::EqualsExpression(PTR(Expression) leftExpression, PTR(Expression) rightExpression) {    this->leftExpression = std::move(leftExpression);    this->rightExpression = std::move(rightExpression);}bool EqualsExpression::equals(PTR(Expression) expression) {    PTR(EqualsExpression) equalsExpression = CAST(EqualsExpression)(expression);    return equalsExpression == nullptr ? false : equalsExpression->leftExpression->equals(this->leftExpression) &&                                                 equalsExpression->rightExpression->equals(this->rightExpression);}PTR(Value) EqualsExpression::interpret(PTR(Environment) environment) {    return NEW(BooleanValue)(            this->leftExpression->interpret(environment)->equals(                    this->rightExpression->interpret(environment)));}PTR(Expression)EqualsExpression::substitute(std::string var, PTR(Value) val) {    return NEW(EqualsExpression)(this->leftExpression->substitute(var, val),                                 this->rightExpression->substitute(var, val));}bool EqualsExpression::hasVariable() {    return this->leftExpression->hasVariable() || this->rightExpression->hasVariable();}std::string EqualsExpression::toString() {    return leftExpression->toString() + " == " + rightExpression->toString();}PTR(Expression)EqualsExpression::optimize() {    PTR(EqualsExpression) optimizedEqualsExpression =            NEW(EqualsExpression)(leftExpression->optimize(), rightExpression->optimize());    if (optimizedEqualsExpression->leftExpression->equals(optimizedEqualsExpression->rightExpression)) {        return NEW(BooleanExpression)(true);    } else {        return optimizedEqualsExpression;    }}void EqualsExpression::stepInterpret() {    Step::mode = Step::InterpMode;    Step::expr = leftExpression;    Step::cont = NEW(RightThenCompContinuation)(rightExpression, Step::env, Step::cont);}/** * _fun (functionParameter) functionBody * @param formalArg * @param body */FunctionExpression::FunctionExpression(std::string formalArg, PTR(Expression) body) {    this->formalArg = std::move(formalArg);    this->body = std::move(body);}bool FunctionExpression::equals(PTR(Expression) expression) {    PTR(FunctionExpression) functionExpression = CAST(FunctionExpression)(expression);    if (functionExpression == nullptr) {        return false;    } else {        return functionExpression->formalArg == this->formalArg &&               functionExpression->body->equals(this->body);    }}PTR(Value) FunctionExpression::interpret(PTR(Environment) env) {    return NEW(FunctionValue)(this->formalArg, this->body, env);}PTR(Expression)FunctionExpression::substitute(std::string var, PTR(Value) val) {    return var == this->formalArg ?           NEW(FunctionExpression)(this->formalArg,                                   this->body) :           NEW(FunctionExpression)(this->formalArg,                                   this->body->substitute(var, val));}bool FunctionExpression::hasVariable() {    return true;}std::string FunctionExpression::toString() {    return "(_fun (" + this->formalArg + ") " + this->body->toString() + ')';}PTR(Expression)FunctionExpression::optimize() {    return NEW(FunctionExpression)(this->formalArg, this->body->optimize());}void FunctionExpression::stepInterpret() {    Step::mode = Step::ContinueMode;    Step::val = NEW(FunctionValue)(this->formalArg, this->body, Step::env);    Step::cont = Step::cont; /* no-op */}CallExpression::CallExpression(PTR(Expression) functionExpression, PTR(Expression) argumentExpression) {    this->toBeCalled = std::move(functionExpression);    this->actualArg = std::move(argumentExpression);}bool CallExpression::equals(PTR(Expression) expression) {    PTR(CallExpression) callExpression = CAST(CallExpression)(expression);    return callExpression == nullptr ? false : callExpression->toBeCalled->equals(this->toBeCalled) &&                                               callExpression->actualArg->equals(this->actualArg);}PTR(Value) CallExpression::interpret(PTR(Environment) environment) {    return this->toBeCalled->interpret(environment)->call(this->actualArg->interpret(environment));}PTR(Expression)CallExpression::substitute(std::string var, PTR(Value) val) {    return NEW(CallExpression)(this->toBeCalled->substitute(var, val),                               this->actualArg->substitute(var, val));}bool CallExpression::hasVariable() {    return this->actualArg->hasVariable() || this->toBeCalled->hasVariable();}std::string CallExpression::toString() {    return this->toBeCalled->toString() + '(' + this->actualArg->toString() + ')';}PTR(Expression)CallExpression::optimize() {    return NEW(CallExpression)(this->toBeCalled->optimize(),                               this->actualArg->optimize());}void CallExpression::stepInterpret() {    Step::mode = Step::InterpMode;    Step::expr = this->toBeCalled;    Step::cont = NEW(ArgThenCallCont)(actualArg, Step::env, Step::cont);}