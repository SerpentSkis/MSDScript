#include <iostream>
#include <string>
#include <cstdlib>
#include "execute.hpp"
#include <cmath>

const int N = 1000000;
const int RANDOM_ITERATORS = 100;

static void checkSuccess(ExecResult &r);

static std::string makeBigString(const std::string &word, int size);

static std::string randomWord();

std::string randomLetter();

bool randomAddTwoNumbersTest(const char *const *interpCommand, const char *const *optCommand);

bool randomMultiplyTwoNumbersTest(const char *const *interpCommand, const char *const *optCommand);

bool randomTwoNumberEqualsTest(const char *const *interpCommand, const char *const *optCommand);

std::string generateBinaryFunctionString(int a, int b, std::string operand);

bool checkOutput(const char *const *interpCommand, const char *const *optCommand, const std::string &userInput,
                 const std::string &expectedInterpretResult, const std::string &expectedOptimizeResult);

bool randomVariableTest(const char *const *optCommand);

bool randomFunctionCallTest(const char *const *interpCommand, const char *const *optCommand);

bool randomLetExpressionTest(const char *const *interpCommand, const char *const *optCommand);

bool randomIfElseExpressionTest(const char *const *interpCommand, const char *const *optCommand);

int main(int argc, const char *argv[]) {
    const char *pathToProgramToBeTested = argv[1];

    //FIXME remove
    pathToProgramToBeTested = "/Users/chadhurst/repo/ChadHurst/CS6015/ArithmeticParser/cmake-build-debug/main";

    srand((unsigned) time(nullptr));

    const char *const interpCommand[] = {
            pathToProgramToBeTested,
            "-interp",
            nullptr};
    const char *const optCommand[] = {
            pathToProgramToBeTested,
            "-opt",
            nullptr};
    const char *const stepCommand[] = {
            pathToProgramToBeTested,
            "-step",
            nullptr};

    std::cout << "started test on " << interpCommand[0] << "\n\n";
    bool error = false;

    error = randomIfElseExpressionTest(interpCommand, optCommand) || error;

    error = randomAddTwoNumbersTest(interpCommand, optCommand) || error;

    error = randomMultiplyTwoNumbersTest(interpCommand, optCommand) || error;

    error = randomTwoNumberEqualsTest(interpCommand, optCommand) || error;

    error = randomVariableTest(optCommand) || error;

    error = randomFunctionCallTest(interpCommand, optCommand) || error;

    error = randomLetExpressionTest(interpCommand, optCommand) || error;

    if (!error) {
        std::cout << "all tests passed" << std::endl;
    }
    return 0;
}

bool randomLetExpressionTest(const char *const *interpCommand, const char *const *optCommand) {
    bool error = false;
    for (int i = 0; i < RANDOM_ITERATORS; i++) {
        int b = rand();

        std::string variable = randomLetter();
        std::string number = std::to_string(b);
        std::string string = "_let " + variable + " = " + number + " _in " + variable + "\n";

        error = error || checkOutput(interpCommand, optCommand, string, number + "\n", number + "\n");
    }
    if (!error) {
        std::cout << "randomLetExpressionTest Passed" << std::endl;
    }
    return error;
}

bool randomIfElseExpressionTest(const char *const *interpCommand, const char *const *optCommand) {
    bool error = false;
    for (int i = 0; i < RANDOM_ITERATORS; i++) {
        int a = rand();
        int b = rand();

        bool randomBool = rand() % 2 == 1;
        std::string randomBoolString;
        if (randomBool) {
            randomBoolString = " _true ";
        } else {
            randomBoolString = " _false ";
        }
        std::string number1 = std::to_string(a);
        std::string number2 = std::to_string(b);
        std::string string = "_if" + randomBoolString + " _then " + number1 + " _else " + number2 + "\n";

        std::string answer;
        if (randomBool) {
            answer = number1;
        } else {
            answer = number2;
        }
        error = error || checkOutput(interpCommand, optCommand, string, answer + "\n", answer + "\n");
    }
    if (!error) {
        std::cout << "randomIfElseExpressionTest Passed" << std::endl;
    }
    return error;
}

bool randomFunctionCallTest(const char *const *interpCommand, const char *const *optCommand) {
    bool error = false;
    for (int i = 0; i < RANDOM_ITERATORS; i++) {
        std::string number = std::to_string(rand());
        std::string variable = randomLetter();

        std::string basicFunctionCallString = "(_fun (" + variable + ") " + variable + ")(" + number + ")\n";
        error = checkOutput(interpCommand, optCommand, basicFunctionCallString, number + "\n",
                            basicFunctionCallString) || error;
    }
    if (!error) {
        std::cout << "randomFunctionCallTest Passed" << std::endl;
    }
    return error;
}

bool randomVariableTest(const char *const *optCommand) {
    bool error = false;
    for (int i = 0; i < RANDOM_ITERATORS; i++) {
        std::string variableString = randomLetter() + '\n';
        ExecResult varResult = execProgram(optCommand, variableString);
        checkSuccess(varResult);

        //TODO check errors on interp
        if (variableString != varResult.out) {
            error = true;
            std::cerr << "expected got\n" << variableString << "\n" << varResult.out << "\n";
        }
    }
    if (!error) {
        std::cout << "randomVariableTest Passed" << std::endl;
    }
    return error;
}

bool randomAddTwoNumbersTest(const char *const *interpCommand, const char *const *optCommand) {
    bool error = false;
    for (int i = 0; i < RANDOM_ITERATORS; i++) {
        int a = rand() / 2;
        int b = rand() / 2;
        std::string randomAddString = generateBinaryFunctionString(a, b, "+");
        std::string trueInterpAddResult = std::to_string(a + b) + '\n';
        std::string trueOptimizeAddResult = std::to_string(a + b) + '\n';
        error = checkOutput(interpCommand, optCommand, randomAddString, trueInterpAddResult, trueOptimizeAddResult) ||
                error;
    }
    if (!error) {
        std::cout << "randomAddTwoNumbersTest Passed" << std::endl;
    }
    return error;
}

bool randomMultiplyTwoNumbersTest(const char *const *interpCommand, const char *const *optCommand) {
    bool error = false;
    for (int i = 0; i < RANDOM_ITERATORS; i++) {
        int a = (int) sqrt(rand());
        int b = (int) sqrt(rand());
        std::string randomMultiplyString = generateBinaryFunctionString(a, b, "*");
        std::string trueInterpretMultiplyResult = std::to_string(a * b) + '\n';
        std::string trueOptimizeMultiplyResult = std::to_string(a * b) + '\n';

        error = checkOutput(interpCommand, optCommand, randomMultiplyString, trueInterpretMultiplyResult,
                            trueOptimizeMultiplyResult) || error;
    }
    if (!error) {
        std::cout << "randomMultiplyTwoNumbersTest Passed" << std::endl;
    }
    return error;
}

bool randomTwoNumberEqualsTest(const char *const *interpCommand, const char *const *optCommand) {
    bool error = false;
    for (int i = 0; i < RANDOM_ITERATORS; i++) {
        int a = rand();
        std::string randomMultiplyString = generateBinaryFunctionString(a, a, "==");

        error = checkOutput(interpCommand, optCommand, randomMultiplyString, "_true\n",
                            "_true\n") || error;
    }
    if (!error) {
        std::cout << "randomTwoNumberEqualsTest Passed" << std::endl;
    }
    return error;
}

bool checkOutput(const char *const *interpCommand, const char *const *optCommand, const std::string &userInput,
                 const std::string &expectedInterpretResult, const std::string &expectedOptimizeResult) {
    bool error = false;
    ExecResult interpCommandResult = execProgram(interpCommand, userInput);
    ExecResult optCommandResult = execProgram(optCommand, userInput);
    checkSuccess(interpCommandResult);
    checkSuccess(optCommandResult);

    if (expectedInterpretResult != interpCommandResult.out) {
        error = true;
        std::cerr << "expected\n" << expectedInterpretResult << "got\n" << interpCommandResult.out << "in\n"
                  << userInput << "with interp\n\n";
    }
    if (expectedOptimizeResult != optCommandResult.out) {
        error = true;
        std::cerr << "expected\n" << expectedOptimizeResult << "got\n" << optCommandResult.out << "in\n"
                  << userInput << "with opt\n\n";
    }
    return error;
}

std::string generateBinaryFunctionString(int a, int b, std::string operand) {
    return "(" + std::to_string(a) + operand + std::to_string(b) + ")\n";
}

std::string randomLetter() {
    char letter = 'a' + rand() % 26;
    std::string string;
    string += letter;
    return string;
}

static void checkSuccess(ExecResult &r) {
    std::cerr << r.err;
    if (r.exitCode != 0) {
        std::cerr << "non-zero exit: " << r.exitCode << "\n";
        std::cerr << r.out << "\n";
    }
}

static std::string makeBigString(const std::string &word, int size) {
    std::string big;
    for (int i = 0; i < size; i++)
        big += word + "\n";
    return big;
}

static std::string randomWord() {
    std::string word = (std::string) "";
    for (int i = rand() % 32; i-- > 0;) {
        word += rand() % 256;
    }
    return word;
}
