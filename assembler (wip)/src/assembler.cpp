#include "../assembler.hpp"
#include <cstring>
#include <format>
#include <stdexcept>

const InstructionDef& find_def(const Signature& signature) {
    auto x = INSTRUCTIONS.find(signature);
    if (x == INSTRUCTIONS.end()) {
        throw std::runtime_error(std::format("Unknown instruction {}.", signature.to_string()));
    }
    return x->second;
}

InstrInstance::InstrInstance(const Signature& signature) : _def(find_def(signature)), _curr_variant(0), _success(false) {}

uint8_t* InstrInstance::_alloc_buffer() {
    if (_def.variants[_curr_variant].size > sizeof(_buffer.ptr)) {
        if (_curr_variant == 0 || _def.variants[_curr_variant - 1].size <= sizeof(_buffer.ptr)) {
            _buffer.ptr = new uint8_t[_def.variants[_curr_variant].size];
        }
        return _buffer.ptr;
    }
    return _buffer.loc;
}

const uint8_t* InstrInstance::_get_buffer() const {
    if (_def.variants[_curr_variant].size > sizeof(_buffer.ptr)) {
        return _buffer.ptr;
    }
    return _buffer.loc;
}

void InstrInstance::_dealloc_buffer() {
    if (_def.variants[_curr_variant].size > sizeof(_buffer.ptr)) {
        delete[] _buffer.ptr;
    }
}

InstrInstance::~InstrInstance() {
    _dealloc_buffer();
}

bool InstrInstance::try_emit(const std::vector<ArgPtr>& args) {
    if (_def.independent && _success)
        return true;
    _success = _def.variants[_curr_variant].emitter(_def.signature.opcode, args, _alloc_buffer());
    if (!_success) {
        ++_curr_variant;
        if (_curr_variant == _def.variants.size()) {
            throw std::runtime_error("Failed to emit instruction."); // TODO: list all errors by having emitter return them and storing them
        }
    }
    return _success;
}

void InstrInstance::write(uint8_t* to) const {
    if (!_success)
        throw std::runtime_error("Cannot write instruction that hasn't been successfully emitted.");
    std::memcpy(to, _get_buffer(), _def.variants[_curr_variant].size);
}