#include "../argument.hpp"

Argument::~Argument() {}

std::type_index Argument::type() const {
    return typeid(*this);
}

RegisterArg::RegisterArg(Register value) : value(value) {}

ImmediateArg::ImmediateArg(bool negative, uint16_t value) : negative(negative), value(value) {}

LabelArg::LabelArg(InstrInstance& target) : target(target) {}