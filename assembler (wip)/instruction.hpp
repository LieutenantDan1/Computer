#pragma once

#include "argument.hpp"
#include <functional>
#include <typeindex>
#include <typeinfo>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// Hashable and equality comparable class that defines the opcode and arguments
// of an instruction.
class Signature {
public:
    const Opcode opcode;
    const std::vector<std::type_index> arguments;

    bool operator==(const Signature& other) const;
    std::string to_string() const;
};

template <>
class std::hash<Signature> {
private:
    [[no_unique_address]] std::hash<Opcode> _opcode_hasher;
    [[no_unique_address]] std::hash<std::type_index> _type_hasher;

public:
    size_t operator()(const Signature& sig) const;
};

class Variant {
public:
    using Emitter = bool(*)(Opcode opcode, const std::vector<ArgPtr>& args, uint8_t* to);

    const size_t size;
    const Emitter emitter;
};

// Definition of an instruction. A definition may have multiple variants of the
// same instruction. Variants are sorted by size (of this instruction in the
// generated binary) so that optimistic attempts can be made.
//
// Invariants:
//   - Each InstructionDef must have at least one variant
//   - All variants must be sorted by size (non-descending)
class InstructionDef {
public:
    const Signature signature;
    const std::vector<Variant> variants;
    const bool independent;
};

const extern std::unordered_map<Signature, InstructionDef> INSTRUCTIONS;