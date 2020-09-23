#pragma once

#include <iostream>
#include "expression.hpp"
#include "pointer.h"

PTR(Expression) parse(std::istream &in);

static PTR(Expression) parseExpr(std::istream &in);

static PTR(Expression) parseAddend(std::istream &in);

static PTR(Expression) parseInner(std::istream &in);

static PTR(Expression) parseNumber(std::istream &in);

static PTR(Expression) parseVariable(std::istream &in);

static PTR(Expression) parseMulticand(std::istream &in);

static char peekAfterSpaces(std::istream &in);

static std::string parseNextWord(std::istream &in);

PTR(Expression) parseUnderScore(std::istream &in);

PTR(Expression) parseLet(std::istream &in);

PTR(Expression) parseOpenParen(std::istream &in);

PTR(Expression) parseTrue(std::istream &in);

PTR(Expression) parseFalse(std::istream &in);

PTR(Expression) parseFun(std::istream &in);

PTR(Expression) parseIf(std::istream &in);

PTR(Expression) parseThen(std::istream &in);

PTR(Expression) parseElse(std::istream &in);

static PTR(Expression) parseStr(const std::string &s);

static std::string parseStrError(const std::string &s);

