#include "Environment.h"

#include <utility>

PTR(Environment) Environment::empty = NEW(EmptyEnv)();

PTR(Value)EmptyEnv::lookup(std::string findName) {
    throw std::runtime_error("free variable: " + findName);
}

EmptyEnv::EmptyEnv() = default;

PTR(Value)ExtendedEnv::lookup(std::string findName) {
    if (findName == name) {
        return val;
    } else {
        return rest->lookup(findName);
    }
}

ExtendedEnv::ExtendedEnv(std::string name, PTR(Value) val, PTR(Environment) rest) {
    this->name = std::move(name);
    this->val = std::move(val);
    this->rest = std::move(rest);

}
