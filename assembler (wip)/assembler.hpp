#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <variant>
#include "instruction.hpp"

class Buffer {
private:
    using Pointer = std::unique_ptr<uint8_t[]>;
    using Local = std::array<uint8_t, sizeof(Pointer)>;
    [[no_unique_address]] std::variant<Pointer, Local> _data;

public:
    Buffer();

    uint8_t* get_size(size_t size);
    const uint8_t* get() const;
};

class InstrInstance {
private:
    const std::vector<ArgPtr> _args;
    const InstructionDef& _def;
    size_t _curr_variant;
    [[no_unique_address]] Buffer _buffer;
    bool _success;
    size_t _address;

public:
    InstrInstance(const Signature& signature, std::vector<ArgPtr>&& args);

    bool try_emit();
    size_t size() const;
    void write(uint8_t* to) const;
};

void assemble(std::vector<InstrInstance>& program, uint8_t* dest);