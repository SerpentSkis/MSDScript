#include <iostream>
#include "parse.h"
#include "Environment.h"
#include "step.h"

#define CATCH_CONFIG_RUNNER

int main(int argc, char **argv) {
    if (argc == 1) {
//        Catch::Session().run(argc, argv);
        try {
            PTR(Expression) e = parse(std::cin);
            std::cout << e->interpret(NEW(EmptyEnv)())->toString() << std::endl;
        } catch (int e) {
            std::cerr << e << std::endl;
            return e;
        }
    } else if (strncmp(argv[1], "-opt", 3) == 0) {
        try {
            PTR(Expression) e = parse(std::cin);
            std::cout << e->optimize()->toString() << std::endl;
        } catch (int e) {
            std::cerr << e << std::endl;
            return e;
        }
    } else if (strncmp(argv[1], "-step", 4) == 0) {
        try {
            PTR(Expression) e = parse(std::cin);
            std::cout << Step::interpBySteps(e)->toString() << std::endl;
        } catch (int e) {
            std::cerr << e << std::endl;
            return e;
        }
    } else {
        std::cout << "bad flag " << std::endl;
        return 1;
    }
    return 0;
}


