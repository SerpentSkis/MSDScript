#include <iostream>
#include <sstream>
#include "parse.h"
#include "Environment.h"

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
