#pragma once

#include <string>

class ExecResult {
public:
    int exitCode;
    std::string out;
    std::string err;

    ExecResult() {
        exitCode = 0;
        out = "";
        err = "";
    }
};

extern ExecResult execProgram(const char *const *command, std::string input);

