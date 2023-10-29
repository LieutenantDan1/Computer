#pragma once

#include "common.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <typeinfo>
#include <typeindex>

class Argument {
public:
    virtual ~Argument();

    std::type_index type() const;
};

using ArgPtr = std::unique_ptr<Argument>;

class RegisterArg : public Argument {
public:
    const Register value;

    RegisterArg(Register value);
};

class ImmediateArg : public Argument {
public:
    const bool negative;
    const uint16_t value;

    ImmediateArg(bool negative, uint16_t value);

    static ArgPtr parse(const std::string& str);
};

class LabelArg : public Argument {
public:
    const std::string value;

    LabelArg(std::string&& value);
};

