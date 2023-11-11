#include "../assembler.hpp"
#include <cstring>
#include <format>
#include <stdexcept>

Buffer::Buffer() : _data() {
    std::get<Local>(_data).fill(0);
}

uint8_t* Buffer::get_size(size_t size) {
    if (size > sizeof(Pointer)) {
        _data = Pointer(new uint8_t[size]);
        return std::get<Pointer>(_data).get();
    }
    return std::get<Local>(_data).data();
}

const uint8_t* Buffer::get() const {
    if (std::holds_alternative<Pointer>(_data))
        return std::get<Pointer>(_data).get();
    return std::get<Local>(_data).data();
}

const InstructionDef& find_def(const Signature& signature) {
    auto x = INSTRUCTIONS.find(signature);
    if (x == INSTRUCTIONS.end()) {
        throw std::runtime_error(std::format("Unknown instruction {}.", signature.to_string()));
    }
    return x->second;
}

InstrInstance::InstrInstance(const Signature& signature, std::vector<ArgPtr>&& args) :
    _args(std::forward<std::vector<ArgPtr>>(args)),
    _def(find_def(signature)),
    _curr_variant(0),
    _success(false)
{}

bool InstrInstance::try_emit() {
    if (_def.independent && _success)
        return true;
    const Variant& variant = _def.variants[_curr_variant];
    _success = variant.emitter(_def.signature.opcode, _args, _buffer.get_size(variant.size));
    if (!_success) {
        ++_curr_variant;
        if (_curr_variant == _def.variants.size()) {
            throw std::runtime_error("Failed to emit instruction."); // TODO: list all errors by having emitter return them and storing them
        }
    }
    return _success;
}

size_t InstrInstance::size() const {
    return _def.variants[_curr_variant].size;
}

void InstrInstance::write(uint8_t* to) const {
    if (!_success)
        throw std::runtime_error("Cannot write instruction that hasn't been successfully emitted.");
    std::memcpy(to, _buffer.get(), size());
}

void assemble(std::vector<InstrInstance>& program, uint8_t* dest) {
    bool retry = true;
    while (retry) {
        retry = false;
        for (size_t i = 0; i < program.size(); ++i) {
            if (!program[i].try_emit()) {
                retry = true;
            }
        }
    }

    for (size_t i = 0, j = 0; i < program.size(); ++i) {
        if (program[i].size() > 65536 - j) {
            throw std::runtime_error("Program too large :----(");
        }
        program[i].write(dest + j);
        j += program[i].size();
    }
}