#pragma once

#include <cstdint>
#include <memory>
#include <variant>
#include "instruction.hpp"

class InstrInstance {
private:
    const InstructionDef& _def;
    size_t _curr_variant;
    union {
        uint8_t* ptr;
        uint8_t loc[sizeof(uint8_t*)];
    } _buffer;
    bool _success;

    uint8_t* _alloc_buffer();
    const uint8_t* _get_buffer() const;
    void _dealloc_buffer();

public:
    InstrInstance(const Signature& signature);
    ~InstrInstance();

    bool try_emit(const std::vector<ArgPtr>& args);
    void write(uint8_t*) const;
};