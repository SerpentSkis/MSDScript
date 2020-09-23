#include <iostream>
#include <sstream>
#include "parse.h"
#include "Environment.h"
#include "../catch/catch.hpp"

/**
 * @param in input stream that contains an Expression.
 * @return the parsed representation of that Expression
 * @throws Throws `runtime_error` for parse errors.
 */
PTR(Expression) parse(std::istream &in) {
    PTR(Expression) e = parseExpr(in);

    char c = peekAfterSpaces(in);
    if (!in.eof())
        throw std::runtime_error((std::string) "expected end of file at " + c);

    return e;
}

/**
 * Takes an input stream that starts with an Expression,
 * consuming the largest initial Expression possible.
 * @param in
 * @return
 */
static PTR(Expression) parseExpr(std::istream &in) {
    PTR(Expression) e = parseAddend(in);

    char c = peekAfterSpaces(in);
    if (c == '+') {
        in >> c;
        PTR(Expression) rhs = parseExpr(in);
        e = NEW(AddExpression)(e, rhs);
    } else if (c == '=') {
        in.get();
        in >> c;
        PTR(Expression) rhs = parseExpr(in);
        e = NEW(EqualsExpression)(e, rhs);
    }

    return e;
}

/**
 * Takes an input stream that starts with an addend,
 * consuming the largest initial addend possible, where
 * an addend is an Expression that does not have `+`
 * except within nested expressions (like parentheses).
 * @param in input stream to be parsed.
 * @return Expression parsed from input stream.
 */
static PTR(Expression) parseAddend(std::istream &in) {
    PTR(Expression) e = parseMulticand(in);

    char c = peekAfterSpaces(in);
    if (c == '*') {
        in.get();
        PTR(Expression) rhs = parseAddend(in);
        e = NEW(MultiplyExpression)(e, rhs);
    }

    return e;
}

PTR(Expression) parseMulticand(std::istream &in) {
    PTR(Expression) expr = parseInner(in);
    while (peekAfterSpaces(in) == '(') {
        in.get();
        peekAfterSpaces(in);
        PTR(Expression) actualArg = parseExpr(in);
        peekAfterSpaces(in);
        expr = NEW(CallExpression)(expr, actualArg);
        in.get();
    }
    return expr;
}

/**
 * Parses something with no immediate `+` or `*` from `in`.
 * @param in input stream to be parsed.
 * @return Expression parsed from input stream.
 */
static PTR(Expression) parseInner(std::istream &in) {
    PTR(Expression) e = nullptr;

    char c = peekAfterSpaces(in);
    if (c == '(') {
        e = parseOpenParen(in);
    } else if (c == '_') {
        e = parseUnderScore(in);
    } else if (isdigit(c) || c == '-') {
        e = parseNumber(in);
    } else if (isalpha(c)) {
        e = parseVariable(in);
    } else {
        throw std::runtime_error((std::string) "expected a digit or open parenthesis at " + c);
    }
    return e;
}

PTR(Expression) parseOpenParen(std::istream &in) {
    in.get();
    PTR(Expression) e = parseExpr(in);
    char c = peekAfterSpaces(in);
    if (c == ')') {
        in.get();
    } else {
        throw std::runtime_error("expected a close parenthesis");
    }
    return e;
}

PTR(Expression) parseUnderScore(std::istream &in) {
    in.get();

    std::string string = parseNextWord(in);

    if (string == "let") {
        return parseLet(in);
    } else if (string == "true") {
        return parseTrue(in);
    } else if (string == "false") {
        return parseFalse(in);
    } else if (string == "fun") {
        return parseFun(in);
    } else if (string == "if") {
        return parseIf(in);
    } else {
        throw std::runtime_error("unknown Expression beginning with '_'");
    }
}

PTR(Expression) parseIf(std::istream &in) {
    PTR(Expression) ifCondition = parseExpr(in);

    PTR(Expression) thenExpression = parseThen(in);

    PTR(Expression) elseExpression = parseElse(in);

    return NEW(IfExpression)(ifCondition, thenExpression, elseExpression);
}

PTR(Expression) parseThen(std::istream &in) {
    peekAfterSpaces(in);
    if (in.get() == '_' && parseNextWord(in) == "then") {
        peekAfterSpaces(in);
        return parseExpr(in);
    } else {
        throw std::runtime_error("unknown Expression where _then is expected' ");
    }
}

PTR(Expression) parseElse(std::istream &in) {
    peekAfterSpaces(in);
    if (in.get() == '_' && parseNextWord(in) == "else") {
        peekAfterSpaces(in);
        return parseExpr(in);
    } else {
        throw std::runtime_error("unknown Expression where _else is expected' ");
    }
}

PTR(Expression) parseFun(std::istream &in) {
    peekAfterSpaces(in);
    PTR(Expression) functionParameter = parseOpenParen(in);
    peekAfterSpaces(in);
    PTR(Expression) functionBody = parseExpr(in);
    return NEW(FunctionExpression)(functionParameter->toString(), functionBody);
}

PTR(Expression) parseFalse(std::istream &in) {
    return NEW(BooleanExpression)(false);
}

PTR(Expression) parseTrue(std::istream &in) {
    return NEW(BooleanExpression)(true);
}

/**
 * _let va
 * _let x = 3 _in x
 * @param in
 * @return
 */
PTR(Expression) parseLet(std::istream &in) {
    peekAfterSpaces(in);
    PTR(Expression) var = parseVariable(in);

    peekAfterSpaces(in);
    PTR(Expression) varEquals;
    if (in.get() == '=') {
        varEquals = parseExpr(in);
    } else {
        throw std::runtime_error("unknown Expression after variable in _let ");
    }

    peekAfterSpaces(in);
    if (in.get() == '_' && parseNextWord(in) == "in") {
        PTR(Expression) expr = parseExpr(in);
        return NEW(LetExpression)(var->toString(), varEquals, expr);
    } else {
        throw std::runtime_error("unknown Expression where _in is expected' ");
    }
}

/**
 * Parses a number, assuming that `in` starts with a digit.
 * @param in istream
 * @return parsed NumberExpression
 */
static PTR(Expression) parseNumber(std::istream &in) {
    int sign = 1;
    if (in.peek() == '-') {
        in.get();
        sign *= -1;
        if (!isnumber(in.peek())) {
            throw std::runtime_error("unknown Expression after '-' where number was expected");
        }
    }
    int num = 0;
    in >> num;
    return NEW(NumberExpression)(sign * num);
}

/**
 * Parses an Expression, assuming that `in` starts with a letter.
 * @param in isStream
 * @return Parsed VariableExpression
 */
static PTR(Expression) parseVariable(std::istream &in) {
    std::string name;
    while (true) {
        char c = static_cast<char>(in.peek());
        if (!isalpha(c))
            break;
        name += (char) in.get();
    }
    return NEW(VariableExpression)(name);
}

/**
 * Like in.peek(), but consume an whitespace at the start of `in`
 * @param in
 * @return
 */
static char peekAfterSpaces(std::istream &in) {
    char c;
    while (true) {
        c = static_cast<char>(in.peek());
        if (!isspace(c))
            break;
        in.get();
    }
    return c;
}

static std::string parseNextWord(std::istream &in) {
    peekAfterSpaces(in);
    std::string word;
    char c;
    do {
        c = in.get();
        word += c;
    } while (std::isalpha(in.peek()));
    return word;
}

/* for tests */
static PTR(Expression) parseStr(const std::string &s) {
    std::istringstream in(s);
    return parse(in);
}

/* for tests */
static std::string parseStrError(const std::string &s) {
    std::istringstream in(s);
    try {
        (void) parse(in);
        return "";
    } catch (std::runtime_error &exn) {
        return exn.what();
    }
}

TEST_CASE("See if this works") {

    PTR(Expression) tenPlusOne = NEW(AddExpression)(NEW(NumberExpression)(10), NEW(NumberExpression)(1));

    CHECK (parseStrError(" ( 1 ") == "expected a close parenthesis");

    CHECK(parseStr("10")->equals(NEW(NumberExpression)(10)));
    CHECK(parseStr("(10)")->equals(NEW(NumberExpression)(10)));
    CHECK(parseStr("10+1")->equals(tenPlusOne));
    CHECK(parseStr("(10+1)")->equals(tenPlusOne));
    CHECK(parseStr("(10)+1")->equals(tenPlusOne));
    CHECK(parseStr("10+(1)")->equals(tenPlusOne));
    CHECK(parseStr("1+2*3")->equals(NEW(AddExpression)(NEW(NumberExpression)(1),
                                                       NEW(MultiplyExpression)(NEW(NumberExpression)(2),
                                                                               NEW(NumberExpression)(3)))));
    CHECK(parseStr("1*2+3")->equals(
            NEW(AddExpression)(NEW(MultiplyExpression)(NEW(NumberExpression)(1), NEW(NumberExpression)(2)),
                               NEW(NumberExpression)(3))));
    CHECK(parseStr("4*2*3")->equals(NEW(MultiplyExpression)(NEW(NumberExpression)(4),
                                                            NEW(MultiplyExpression)(NEW(NumberExpression)(2),
                                                                                    NEW(NumberExpression)(3)))));
    CHECK(parseStr("4+2+3")->equals(NEW(AddExpression)(NEW(NumberExpression)(4),
                                                       NEW(AddExpression)(NEW(NumberExpression)(2),
                                                                          NEW(NumberExpression)(3)))));
    CHECK(parseStr("4*(2+3)")->equals(NEW(MultiplyExpression)(NEW(NumberExpression)(4),
                                                              NEW(AddExpression)(NEW(NumberExpression)(2),
                                                                                 NEW(NumberExpression)(3)))));
    CHECK(parseStr("(2+3)*4")->equals(
            NEW(MultiplyExpression)(NEW(AddExpression)(NEW(NumberExpression)(2), NEW(NumberExpression)(3)),
                                    NEW(NumberExpression)(4))));
    CHECK(parseStr("xyz")->equals(NEW(VariableExpression)("xyz")));
    CHECK(parseStr("xyz+1")->equals(NEW(AddExpression)(NEW(VariableExpression)("xyz"), NEW(NumberExpression)(1))));

    CHECK (parseStrError("!") == "expected a digit or open parenthesis at !");
    CHECK (parseStrError("(1") == "expected a close parenthesis");

    CHECK(parseStr(" 10 ")->equals(NEW(NumberExpression)(10)));
    CHECK(parseStr(" (  10 ) ")->equals(NEW(NumberExpression)(10)));
    CHECK(parseStr(" 10  + 1")->equals(tenPlusOne));
    CHECK(parseStr(" ( 10 + 1 ) ")->equals(tenPlusOne));
    CHECK(parseStr(" 11 * ( 10 + 1 ) ")->equals(NEW(MultiplyExpression)(NEW(NumberExpression)(11),
                                                                        tenPlusOne)));
    CHECK(parseStr(" ( 11 * 10 ) + 1 ")
                  ->equals(NEW(AddExpression)(
                          NEW(MultiplyExpression)(NEW(NumberExpression)(11), NEW(NumberExpression)(10)),
                          NEW(NumberExpression)(1))));
    CHECK(parseStr(" 1 + 2 * 3 ")
                  ->equals(NEW(AddExpression)(NEW(NumberExpression)(1),
                                              NEW(MultiplyExpression)(NEW(NumberExpression)(2),
                                                                      NEW(NumberExpression)(3)))));

    CHECK (parseStrError(" ! ") == "expected a digit or open parenthesis at !");
    CHECK (parseStrError(" ( 1 ") == "expected a close parenthesis");
    CHECK (parseStrError(" 1 )") == "expected end of file at )");

}

TEST_CASE("add and multiply by steps") {

    CHECK(Step::interpBySteps(parseStr("2* 3 + 5"))->toString() == "11");
    CHECK(Step::interpBySteps(parseStr("5 + 2*3"))->toString() == "11");
}

TEST_CASE("LetValue") {
    CHECK_THROWS(parseStr("_let x = 3 _in y")->interpret(NEW(EmptyEnv)()));
    CHECK(parseStr("_let x = 3 _in 1")->interpret(NEW(EmptyEnv)())->toString() == "1");
    CHECK(parseStr("_let x = 3 _in x")->interpret(NEW(EmptyEnv)())->toString() == "3");
    CHECK(parseStr("_let x = 3 _in 3 + 2")->interpret(NEW(EmptyEnv)())->toString() == "5");
    CHECK(parseStr("_let x = 3 _in x + x")->interpret(NEW(EmptyEnv)())->toString() == "6");
    CHECK(parseStr("_let x = 3 _in 2 * x")->interpret(NEW(EmptyEnv)())->toString() == "6");
    CHECK(parseStr("_let x = 3 _in x * 2")->interpret(NEW(EmptyEnv)())->toString() == "6");
    CHECK(parseStr("_let x = 3 _in x + 4")->interpret(NEW(EmptyEnv)())->toString() == "7");
    CHECK(parseStr("_let x = 3 _in x + 1")->interpret(NEW(EmptyEnv)())->toString() == "4");
    CHECK(Step::interpBySteps(parseStr("_let x = 3 _in x + 1"))->toString() == "4");
}

TEST_CASE("LetExpr") {
    CHECK(parseStr("_let x = 3 _in x")->equals(
            NEW(LetExpression)("x", NEW(NumberExpression)(3), NEW(VariableExpression)("x"))));
    CHECK(parseStr("_let x = 3 _in x + 2")->equals(
            NEW(LetExpression)("x", NEW(NumberExpression)(3),
                               NEW(AddExpression)(NEW(VariableExpression)("x"), NEW(NumberExpression)(2)))));
    CHECK(parseStr("_let x = 3 _in x + 2")->equals(
            NEW(LetExpression)("x", NEW(NumberExpression)(3),
                               NEW(AddExpression)(NEW(VariableExpression)("x"), NEW(NumberExpression)(2)))));
}

TEST_CASE("ExprValue") {
    CHECK((NEW(LetExpression)("x", NEW(NumberExpression)(3),
                              NEW(VariableExpression)("x")))->interpret(NEW(EmptyEnv)())->equals(NEW(NumberValue)(3)));
    CHECK((NEW(LetExpression)("x", NEW(NumberExpression)(4),
                              NEW(VariableExpression)("x")))->interpret(NEW(EmptyEnv)())->equals(NEW(NumberValue)(4)));
    CHECK((NEW(LetExpression)("x", NEW(NumberExpression)(4),
                              NEW(AddExpression)(NEW(VariableExpression)("x"),
                                                 NEW(NumberExpression)(3))))->interpret(NEW(EmptyEnv)())->equals(
            NEW(NumberValue)(7)));
}

TEST_CASE("substitute") {
    CHECK(parseStr("x")->substitute("x", NEW(NumberValue)(3))->interpret(
            NEW(EmptyEnv)())->equals(
            NEW(NumberValue)(3)));
    CHECK(parseStr("x + 7")->substitute("x",
                                        NEW(NumberValue)(3))->interpret(NEW(EmptyEnv)())->toString() == "10");
    CHECK(parseStr("31 + x")->substitute("x",
                                         NEW(NumberValue)(8))->interpret(NEW(EmptyEnv)())->toString() == "39");
    CHECK(parseStr("x * 3")->substitute("x",
                                        NEW(NumberValue)(2))->interpret(NEW(EmptyEnv)())->toString() == "6");
    CHECK(parseStr("3 * x")->substitute("x",
                                        NEW(NumberValue)(2))->interpret(NEW(EmptyEnv)())->toString() == "6");
}

TEST_CASE("sub let") {
    CHECK(parseStr("_let x = 3 _in _let y = 2 _in 3")
                  ->equals(NEW(LetExpression)("x", NEW(NumberExpression)(3),
                                              NEW(LetExpression)("y", NEW(NumberExpression)(2),
                                                                 NEW(NumberExpression)(3)))));
    CHECK(parseStr("_let x = 3 _in _let y = 2 _in 3")->toString() == "_let x = 3 _in (_let y = 2 _in (3))");
    CHECK(parseStr("_let x = 3 _in _let y = 2 _in _let z = 3 _in 4")
                  ->equals(NEW(LetExpression)("x", NEW(NumberExpression)(3),
                                              NEW(LetExpression)("y", NEW(NumberExpression)(2),
                                                                 NEW(LetExpression)("z",
                                                                                    NEW(NumberExpression)(3),
                                                                                    NEW(NumberExpression)(4))))));
}

TEST_CASE("sub value") {
    CHECK(parseStr("_let x = 3 _in _let y = 2 _in 3")->interpret(NEW(EmptyEnv)())->toString() == "3");
    CHECK(parseStr("_let x = 3 _in _let y = 2 _in 3 + 4")->interpret(NEW(EmptyEnv)())->toString() == "7");
    CHECK(parseStr("_let x = 3 _in _let y = 2 _in 3 + y")->interpret(NEW(EmptyEnv)())->toString() == "5");
    CHECK(parseStr("_let x = 3 _in _let y = 2 _in x + y")->interpret(NEW(EmptyEnv)())->toString() == "5");
    CHECK(parseStr("_let z = 0 _in _let x = 3 _in _let y = 2 _in x + y + z")->interpret(NEW(EmptyEnv)())->toString() ==
          "5");
}

TEST_CASE("toString") {
    CHECK(parseStr("1+2")->optimize()->toString() == "3");
    CHECK(parseStr("1+x")->optimize()->toString() == "(1 + x)");
    CHECK(parseStr("x+1")->optimize()->toString() == "(x + 1)");
    CHECK(parseStr("x+1+1")->optimize()->toString() == "(x + 2)");
    CHECK(parseStr("x+2+1")->optimize()->toString() == "(x + 3)");
    CHECK(parseStr("x+3*3")->optimize()->toString() == "(x + 9)");
    CHECK(parseStr("x+(3*3)")->optimize()->toString() == "(x + 9)");
    CHECK(parseStr("(x+3)*3")->optimize()->toString() == "((x + 3) * 3)");
    CHECK(parseStr("_let x = 3 _in x")->optimize()->toString() == "3");
    CHECK(parseStr("_let x = 3 _in y")->optimize()->toString() == "y");
    CHECK(parseStr("_let x = 3 + y _in y")->optimize()->toString() == "_let x = (3 + y) _in (y)");
    CHECK(parseStr("_let x = 5 _in _let y = z + 2 _in x + y + (2*3)")->optimize()->toString() ==
          "_let y = (z + 2) _in ((5 + (y + 6)))");
    CHECK(parseStr("_let x = (_let y = 1 + 7 _in y + 2) _in x + 4")->interpret(NEW(EmptyEnv)())->toString() == "14");
    CHECK(parseStr("_let x = z + 2 _in x + x + x + x")->optimize()->toString() ==
          "_let x = (z + 2) _in ((x + (x + (x + x))))");
}

TEST_CASE("_parsing") {
    CHECK(parseStr("_true")->equals(NEW(BooleanExpression)(true)));
    CHECK(parseStr("_false")->equals(NEW(BooleanExpression)(false)));
    CHECK(parseStr("_false")->toString() == "_false");
    CHECK(parseStr("_true")->toString() == "_true");
    CHECK(parseStr("2 == 2")->toString() == "2 == 2");
    CHECK(parseStr("2 == 2")->interpret(NEW(EmptyEnv)())->toString() == "_true");
    CHECK(parseStr("2 == 2")->optimize()->toString() == "_true");
    CHECK(parseStr("_if 2 == 2 _then 3 _else 5")->interpret(NEW(EmptyEnv)())->toString() == "3");
    CHECK(parseStr("_if 2 == 2 _then 3 _else 5")->toString() == "_if 2 == 2 _then 3 _else 5");
}

TEST_CASE("negative") {
    CHECK(parseStr("-1")->equals(NEW(NumberExpression)(-1)));
    CHECK(parseStr("-1")->toString() == "-1");
    CHECK(parseStr("1 + -1")->toString() == "(1 + -1)");
    CHECK(parseStr("1 + -1")->interpret(NEW(EmptyEnv)())->toString() == "0");
    CHECK(Step::interpBySteps(parseStr("1 + -1"))->toString() == "0");
}

TEST_CASE("number plus boolean") {
    CHECK_THROWS(parseStr("1 + _false")->interpret(NEW(EmptyEnv)()));
    CHECK_THROWS(parseStr("1 + _false + _true")->interpret(NEW(EmptyEnv)()));
    CHECK_THROWS(parseStr("2 * 2 + _false * _true")->interpret(NEW(EmptyEnv)()));
    CHECK_THROWS(parseStr("_false + -2 * 2 + _false * _true")->interpret(NEW(EmptyEnv)()));
    CHECK_THROWS(parseStr("_false + _false")->interpret(NEW(EmptyEnv)()));
    CHECK_THROWS(parseStr("_true + _true")->interpret(NEW(EmptyEnv)()));
    CHECK_THROWS(parseStr("_true * _true")->interpret(NEW(EmptyEnv)()));
}

TEST_CASE("parseIf") {
    CHECK(parseStr("_if _true _then 3 _else 4")->optimize()->toString() == "3");
    CHECK(parseStr("_if _false _then 3 _else 4")->optimize()->toString() == "4");
    CHECK(Step::interpBySteps(parseStr("_if _false _then 3 _else 4"))->toString() == "4");
}

TEST_CASE("equalsExpressionEquals") {
    CHECK(parseStr("3 == 3")->equals(NEW(EqualsExpression)(NEW(NumberExpression)(3), NEW(NumberExpression)(3))));
    CHECK(parseStr("(1 + 2) == 3")->optimize()->toString() == "_true");
    CHECK(parseStr("(1 + 2) == x")->optimize()->toString() == "3 == x");
    CHECK(parseStr("(1 + 2) == x + 1")->optimize()->toString() == "3 == (x + 1)");
    CHECK(Step::interpBySteps(parseStr("(1 + 2) == 3"))->toString() == "_true");
}

TEST_CASE("badParse") {
    CHECK_THROWS(parseStr("_"));
    CHECK_THROWS(parseStr("_1"));
    CHECK_THROWS(parseStr("_tru"));
    CHECK_THROWS(parseStr("_tre"));
    CHECK_THROWS(parseStr("x_2"));
    CHECK_THROWS(parseStr("_in"));
    CHECK_THROWS(parseStr("_if _tru"));
    CHECK_THROWS(parseStr("_if _true _t"));
    CHECK_THROWS(parseStr("_if _true _then 3 _ell"));
    CHECK_THROWS(parseStr("_if _true _then 3 _elsee 4"));

    CHECK_THROWS(parseStr("_le d"));
    CHECK_THROWS(parseStr("_le d = 4"));
    CHECK_THROWS(parseStr("_le d = 4 _inn x"));
    CHECK_THROWS(parseStr("_le d = 4 _n x"));
    CHECK_THROWS(parseStr("_le d = 4 _in _"));
    CHECK_THROWS(parseStr("_let d = -d _inn d"));
}

TEST_CASE("fibFunction") {
    std::string fibonacciString10 = " _let fib = _fun (fib)\n"
                                    "              _fun (x)\n"
                                    "                 _if x == 0\n"
                                    "                 _then 1\n"
                                    "                 _else _if x == 2 + -1\n"
                                    "                 _then 1\n"
                                    "                 _else fib(fib)(x + -1)\n"
                                    "                       + fib(fib)(x + -2)\n"
                                    " _in fib(fib)(10)";

    CHECK(parseStr(fibonacciString10)->toString() == "_let fib = (_fun (fib) "
                                                     "(_fun (x)"
                                                     " _if x == 0"
                                                     " _then 1"
                                                     " _else _if x == (2 + -1)"
                                                     " _then 1"
                                                     " _else (fib(fib)((x + -1))"
                                                     " + fib(fib)((x + -2)))))"
                                                     " _in (fib(fib)(10))");
    CHECK(parseStr(fibonacciString10)->optimize()->toString() == "_let fib = (_fun (fib) "
                                                                 "(_fun (x)"
                                                                 " _if x == 0"
                                                                 " _then 1"
                                                                 " _else _if x == 1"
                                                                 " _then 1"
                                                                 " _else (fib(fib)((x + -1))"
                                                                 " + fib(fib)((x + -2)))))"
                                                                 " _in (fib(fib)(10))");

    CHECK(parseStr(" _let fib = _fun (fib)\n"
                   "              _fun (x)\n"
                   "                 _if x == 0\n"
                   "                 _then 1\n"
                   "                 _else _if x == 2 + -1\n"
                   "                 _then 1\n"
                   "                 _else fib(fib)(x + -1)\n"
                   "                       + fib(fib)(x + -2)\n"
                   " _in fib(fib)(0)")->interpret(NEW(EmptyEnv)())->toString() == "1");

    CHECK(parseStr(" _let fib = _fun (fib)\n"
                   "              _fun (x)\n"
                   "                 _if x == 0\n"
                   "                 _then 1\n"
                   "                 _else _if x == 2 + -1\n"
                   "                 _then 1\n"
                   "                 _else fib(fib)(x + -1)\n"
                   "                       + fib(fib)(x + -2)\n"
                   " _in fib(fib)(1)")->interpret(NEW(EmptyEnv)())->toString() == "1");

}

TEST_CASE("fib by steps") {

    CHECK(Step::interpBySteps(parseStr(" _let fib = _fun (fib)\n"
                                       "              _fun (x)\n"
                                       "                 _if x == 0\n"
                                       "                 _then 1\n"
                                       "                 _else _if x == 2 + -1\n"
                                       "                 _then 1\n"
                                       "                 _else fib(fib)(x + -1)\n"
                                       "                       + fib(fib)(x + -2)\n"
                                       " _in fib(fib)(1)"))->toString() == "1");
}

TEST_CASE("stressTest Fib") {
    std::string fibonacciString10 = " _let fib = _fun (fib)\n"
                                    "              _fun (x)\n"
                                    "                 _if x == 0\n"
                                    "                 _then 1\n"
                                    "                 _else _if x == 2 + -1\n"
                                    "                 _then 1\n"
                                    "                 _else fib(fib)(x + -1)\n"
                                    "                       + fib(fib)(x + -2)\n"
                                    " _in fib(fib)(10)";

    CHECK(parseStr(" _let fib = _fun (fib)\n"
                   "              _fun (x)\n"
                   "                 _if x == 0\n"
                   "                 _then 1\n"
                   "                 _else _if x == 2 + -1\n"
                   "                 _then 1\n"
                   "                 _else fib(fib)(x + -1)\n"
                   "                       + fib(fib)(x + -2)\n"
                   " _in fib(fib)(2)")->interpret(NEW(EmptyEnv)())->toString() == "2");

    CHECK(parseStr(fibonacciString10)->interpret(NEW(EmptyEnv)())->toString() == "89");
}

TEST_CASE("fib small test by steps") {
    CHECK(Step::interpBySteps(parseStr(" _let fib = _fun (fib)\n"
                                       "              _fun (x)\n"
                                       "                 _if x == 0\n"
                                       "                 _then 1\n"
                                       "                 _else _if x == 2 + -1\n"
                                       "                 _then 1\n"
                                       "                 _else fib(fib)(x + -1)\n"
                                       "                       + fib(fib)(x + -2)\n"
                                       " _in fib(fib)(2)"))->toString() == "2");
}

TEST_CASE("fib stress test by steps") {
    std::string fibonacciString10 = " _let fib = _fun (fib)\n"
                                    "              _fun (x)\n"
                                    "                 _if x == 0\n"
                                    "                 _then 1\n"
                                    "                 _else _if x == 2 + -1\n"
                                    "                 _then 1\n"
                                    "                 _else fib(fib)(x + -1)\n"
                                    "                       + fib(fib)(x + -2)\n"
                                    " _in fib(fib)(10)";
    CHECK(Step::interpBySteps(parseStr(fibonacciString10))->toString() == "89");
}

TEST_CASE("Interp Stress Tests Fun/Call Expressions") {
    const char *string = "_let factorial = _fun (factorial) _fun (x)"
                         "_if x == 1"
                         "_then 1"
                         "_else x * factorial(factorial)(x + -1)"
                         "_in _let factorial = factorial(factorial) _in factorial(5)";
    CHECK(parseStr(string)->interpret(NEW(EmptyEnv)())->toString() == "120");
    CHECK(Step::interpBySteps(parseStr(string))->toString() == "120");
}

TEST_CASE("babyStepsFunctions") {
    CHECK(parseStr("_fun (x) x + 1")->toString() == "(_fun (x) (x + 1))");
    CHECK(parseStr("(_fun (x) x + 1)(10)")->interpret(NEW(EmptyEnv)())->toString() == "11");
    CHECK(parseStr("(_fun (x) x + 1)(10)")->optimize()->toString() == "(_fun (x) (x + 1))(10)");
    CHECK(parseStr("_let foo = _fun (x) x + 1 _in foo(2)")->interpret(NEW(EmptyEnv)())->toString() == "3");
    CHECK(parseStr("_let foo = _fun (x) 1 + 1 _in foo(2)")->interpret(NEW(EmptyEnv)())->toString() == "2");
    CHECK(parseStr("_let foo = _fun (x) 1 + 1 _in foo(2)")->optimize()->toString() ==
          "_let foo = (_fun (x) 2) _in (foo(2))");
    CHECK(parseStr("_let foo = _fun (x) 1 + 1 _in foo(2)")->optimize()->toString() ==
          "_let foo = (_fun (x) 2) _in (foo(2))");
    CHECK(parseStr("_let foo = _fun (x) x + x + 1 _in foo(2)")->interpret(NEW(EmptyEnv)())->toString() == "5");
    CHECK(parseStr("_let foo = _fun (x) x + x + 1 _in foo(2)")->interpret(NEW(EmptyEnv)())->toString() == "5");
    CHECK(Step::interpBySteps(parseStr("_let foo = _fun (x) x + x + 1 _in foo(2)"))->toString() == "5");
}

TEST_CASE("composing Errors") {
    CHECK(parseStr("4(20)")->toString() == "4(20)");
    CHECK(parseStr("4(20)")->equals(NEW(CallExpression)(NEW(NumberExpression)(4), NEW(NumberExpression)(20))));
    CHECK_THROWS(parseStr("4(20)")->interpret(NEW(EmptyEnv)()));
    CHECK_THROWS(parseStr("_false(20)")->interpret(NEW(EmptyEnv)()));
    CHECK_THROWS(parseStr("_true(20)")->interpret(NEW(EmptyEnv)()));
    CHECK_THROWS(parseStr("4(_false)")->interpret(NEW(EmptyEnv)()));
    CHECK_THROWS(parseStr("4(_true)")->interpret(NEW(EmptyEnv)()));
}

TEST_CASE("funThrows") {
    CHECK_THROWS(parseStr("(_fun(x) x) + (_fun(x) x)")->interpret(NEW(EmptyEnv)()));
    CHECK_THROWS(parseStr("(_fun(x) x) + 3")->interpret(NEW(EmptyEnv)()));
    CHECK_THROWS(parseStr("3 + (_fun(x) x)")->interpret(NEW(EmptyEnv)()));

    CHECK_THROWS(parseStr("(_fun(x) x) * (_fun(x) x)")->interpret(NEW(EmptyEnv)()));
    CHECK_THROWS(parseStr("(_fun(x) x) * 3")->interpret(NEW(EmptyEnv)()));
    CHECK_THROWS(parseStr("3 * (_fun(x) x)")->interpret(NEW(EmptyEnv)()));

    CHECK_THROWS(parseStr("(_fun(x) x)")->interpret(NEW(EmptyEnv)())->isTrue());
    CHECK_THROWS(parseStr("x")->interpret(NEW(EmptyEnv)())->isTrue());
    CHECK_THROWS(parseStr("3")->interpret(NEW(EmptyEnv)())->isTrue());

    CHECK(!parseStr("_fun(x) x")->interpret(NEW(EmptyEnv)())->equals(parseStr("3")->interpret(NEW(EmptyEnv)())));
    CHECK(!parseStr("_fun(x) x")->interpret(NEW(EmptyEnv)())->equals(
            parseStr("_fun(x) x + 1")->interpret(NEW(EmptyEnv)())));
    CHECK(parseStr("_fun(x) x")->interpret(NEW(EmptyEnv)())->equals(parseStr("_fun(x) x")->interpret(NEW(EmptyEnv)())));
    CHECK(!parseStr("_fun(y) x")->interpret(NEW(EmptyEnv)())->equals(
            parseStr("_fun(x) x")->interpret(NEW(EmptyEnv)())));
    CHECK(!parseStr("_fun(y) x")->interpret(NEW(EmptyEnv)())->equals(
            parseStr("_fun(x) y")->interpret(NEW(EmptyEnv)())));
}

TEST_CASE("hasVariable") {
    CHECK(parseStr("_let x = 3 _in x")->hasVariable());
    CHECK(!parseStr("_let x = 3 _in x")->optimize()->hasVariable());
}

TEST_CASE("functionExpressionEquals") {
    CHECK(parseStr("_fun(x) x")->equals(parseStr("_fun(x) x")));
    CHECK(!parseStr("_fun(x) y")->equals(parseStr("_fun(x) x")));
    CHECK(!parseStr("_fun(x) y")->equals(parseStr("x")));
    CHECK(!parseStr("x")->equals(parseStr("_fun(x) x")));
}

TEST_CASE("IfExpressionEquals") {
    CHECK(parseStr("_if _true _then 3 _else 4")->equals(parseStr("_if _true _then 3 _else 4")));
    CHECK(parseStr("_if _true _then 3 _else 4")->interpret(NEW(EmptyEnv)())->equals(
            parseStr("3")->interpret(NEW(EmptyEnv)())));
    CHECK(parseStr("_if _false _then 3 _else 4")->interpret(NEW(EmptyEnv)())->equals(
            parseStr("4")->interpret(NEW(EmptyEnv)())));
    CHECK(!parseStr("_if _false _then 3 _else 4")->equals(parseStr("_if _true _then 3 _else 4")));
    CHECK(!parseStr("_if _true _then 3 _else 2")->equals(parseStr("3")));
    CHECK(!parseStr("3")->equals(parseStr("_if _true _then 3 _else 4")));
}

TEST_CASE("LetExpressionEquals") {
    CHECK(parseStr("_let x = 3 _in 3")->equals(parseStr("_let x = 3 _in 3")));
    CHECK(!parseStr("_let x = 3 _in 3")->equals(parseStr("3")));
    CHECK(parseStr("_let x = 3 _in 3")->interpret(NEW(EmptyEnv)())->equals(parseStr("3")->interpret(NEW(EmptyEnv)())));
}

TEST_CASE("CallExpressionEquals") {
    CHECK(parseStr("(_fun(x) x)(3)")->equals(
            NEW(CallExpression)(NEW(FunctionExpression)("x", NEW(VariableExpression)("x")),
                                NEW(NumberExpression)(3))));
    CHECK(parseStr("(_fun(x) x)(3)")->equals(parseStr("(_fun(x) x)(3)")));
    CHECK(parseStr("(_fun(x) x)(3)")->interpret(NEW(EmptyEnv)())->toString() == "3");
    CHECK(!parseStr("(_fun(x) x)(3)")->equals(parseStr("(_fun(x) x)(4)")));
}

TEST_CASE("LetExpressionSubstitute") {
    CHECK(parseStr("_let x = 3 _in x")->substitute("x",
                                                   NEW(NumberValue)(4))->interpret(NEW(EmptyEnv)())->toString() == "3");
}

TEST_CASE("functions in functions") {
    CHECK(parseStr("_let y = 8\n"
                   "_in _let f = _fun (x) x*y\n"
                   "_in f(2)")->interpret(NEW(EmptyEnv)())->toString() == "16");
}

TEST_CASE("Double Call Expr") {
    PTR(CallExpression) expression = NEW(CallExpression)(
            NEW(CallExpression)(NEW(VariableExpression)("f"), NEW(NumberExpression)(10)),
            NEW(NumberExpression)(1));

    CHECK(parseStr("f(10)(1)")->equals(expression));
    CHECK(parseStr("f(10)(1)")->optimize()->equals(expression));
}

TEST_CASE("Multiple Arguments") {
    CHECK(parseStr("_let f = _fun (x) _fun (y)\n"
                   "x*x + y*y _in f(2)(3)")->interpret(NEW(EmptyEnv)())->toString() == "13");
}

TEST_CASE("Multiple Arguments by steps") {
    CHECK(Step::interpBySteps(parseStr("_let f = _fun (x) _fun (y)\n"
                                       "x*x + y*y _in f(2)(3)"))->toString() == "13");
}

TEST_CASE("countDown") {
    CHECK(Step::interpBySteps(parseStr("_let countdown = _fun(countdown) _fun(n)\n"
                                       "_if n == 0\n"
                                       "_then 0\n"
                                       "_else countdown(countdown)(n + -1)\n"
                                       "_in countdown(countdown)(1000000)"))->toString() == "0");
}

TEST_CASE("countUp") {
    CHECK(Step::interpBySteps(parseStr("_let count = _fun(count) _fun(n)\n"
                                       "_if n == 0\n"
                                       "_then 0\n"
                                       "_else 1 + count(count)(n + -1)\n"
                                       "_in count(count)(100000)"))->toString() == "100000");
}
