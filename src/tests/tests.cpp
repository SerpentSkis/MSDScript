#define CATCH_CONFIG_RUNNER

#include "../catch/catch.hpp"

int main(int argc, const char *argv[]) {
    Catch::Session().run(argc, argv);
    return 0;
}