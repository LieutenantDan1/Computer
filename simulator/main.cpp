#include <array>
#include <bit>
#include <concepts>
#include <fstream>
#include <limits>
#include <memory>
#include <iostream>
#include <ranges>
#include <vector>

static_assert(std::numeric_limits<unsigned char>::digits == 8, "silly platform");

class CPU
{
public:
    enum Opcode
    {
        ADD, // 0
        SUB, // 1
        RO0, // 2, reserved
        RO1, // 3, reserved
        RO2, // 4, reserved
        RO3, // 5, reserved
        RO4, // 6, reserved
        LSL, // 7
        LSR, // 8
        ASR, // 9
        XOR, // A
        OR,  // B
        AND, // C
        BRA, // D
        JMP, // E
        MEM, // F
    };

    enum Register
    {
        ZERO, // Hard-wired zero
        RA, // Return address
        SP, // Stack pointer
        RR0, // Reserved

        A0, // Arguments (callee saved)
        A1,
        A2,
        A3,

        T0, // Temporary (caller saved)
        T1,
        T2,
        T3,

        S0, // Saved (callee saved)
        S1,
        S2,
        S3
    };

    static const size_t MEM_SIZE = 65536;
    static const uint16_t RESET_VECTOR = 0xFFFD;

private:
    // Data
    std::unique_ptr<uint8_t[]> _memory;
    std::array<uint16_t, 16> _register;
    uint16_t _bus = 0; // Common bus, reset to 0 every time it's read.
    uint16_t _address = RESET_VECTOR; // The current memory address reads/writes will go to.
    uint16_t _temp_pc = 0; // Used to save the next instruction's address when performing a load/store.
    uint16_t _alu_left = 0; // ALU current left operand.
    uint16_t _alu_right = 0; // ALU current right operand.
    uint16_t _alu_result = 0; // ALU result.

    // Decoder stuff
    uint8_t _cycle = 2; // The current cycle.
    uint8_t _opcode = JMP; // Current opcode.
    uint8_t _dest = 0; // Destination register, or branch flags.
    uint8_t _left = 0; // Left operand register, or load/store flags.
    uint8_t _right = 0;  // Right operand register, or 0 to signify immediate.
    uint8_t _index = 0; // If !_inc_addr, this is added (sign extended) to the effective address and reset to 0 before the next cycle.
    bool _inc_addr = false; // Increment address before next cycle (overrides _index, making it wait one more cycle).
    bool _load_imm = true; // Are we loading an immediate?
    bool _load_word = true; // If we're loading an immediate, it is a word or a byte?
    bool _load_high = true; // If we're loading a word, do we still have to load the high byte?
    bool _imm_to_idx = false; // Will this immediate go to _index or to _alu_right?
    bool _take_branch = false; // Are we going to take the upcoming branch?

    enum _Flags : uint8_t
    {
        BRA_NOT = 0x1,
        BRA_LT = 0x2,
        BRA_U = 0x4,
        BRA_EQ = 0x8,
        
        MEM_LOAD = 0x1,
        MEM_WORD = 0x2,
        MEM_SEX = 0x4,
    };

    // Performs arithmetic right shift by n bits on x.
    // The 4 highest bits of n are discarded, so the shift count is within 0-15.
    static uint16_t _asr(uint16_t x, uint8_t n)
    {
        n &= 0x000F;
        if (n == 0)
            return x;

        bool neg = x & 0x8000;
        x >>= n;
        if (!neg)
            return x;
            
        uint16_t pad = 0xFFFF;
        pad <<= 16 - n;
        x |= pad;
        return x;
    }

    // Read the word currently on the bus. This will reset the bus to 0.
    // If sex is true, the value will be sign extended according to the low byte. The high byte is ignored.
    uint16_t _read_bus(bool sex)
    {
        uint16_t value = _bus;
        _bus = 0;
        if (sex && value & 0x0080)
            value |= 0xFF00;
        return value;
    }

    // Overwrite the value on the bus.
    void _write_bus(uint16_t value)
    {
        _bus = value;
    }

    // Overwrite the low byte of the value on the bus.
    void _write_bus_low(uint8_t value)
    {
        _bus &= 0xFF00;
        _bus |= value;
    }

    // Overwrite the high byte of the value on the bus.
    void _write_bus_high(uint8_t value)
    {
        _bus &= 0x00FF;
        _bus |= (uint16_t)value << 8;
    }

    void _decode(uint16_t instruction)
    {
        _opcode = instruction >> 12;
        _dest = (instruction >> 8) & 0xF;
        _left = (instruction >> 4) & 0xF;
        _right = instruction & 0xF;

        _load_imm = false;
        _load_word = false;
        _imm_to_idx = false;
        switch (_opcode)
        {
        case SUB:
            _load_imm = _right == 0;
            _load_word = true;
            break;
        case ADD:
        case LSL:
        case LSR:
        case ASR:
            _load_imm = _right == 0;
            _load_word = false;
            break;
        case XOR:
        case OR:
        case AND:
            _load_imm = _right == 0;
            _load_word = true;
            break;
        case JMP:
            _load_imm = _right == 0 || _left != 0;
            _load_word = _left == 0;
            _imm_to_idx = _left != 0;
            break;
        case BRA:
            _load_imm = false;
            break;
        case MEM:
            _load_imm = true;
            _load_word = false;
            _imm_to_idx = true;
            break;
        default:
            break;
        }
        _load_high = true;
    }

    void _test_branch()
    {
        _take_branch = false;
        if (_dest & BRA_EQ)
            _take_branch = _alu_left == _alu_right;
        if (_dest & BRA_LT)
        {
            if (_dest & BRA_U)
                _take_branch = _take_branch || _alu_left < _alu_right;
            else
                _take_branch = _take_branch || (int16_t)_alu_left < (int16_t)_alu_right;
        }
        if (_dest & BRA_NOT)
            _take_branch = !_take_branch;
    }

    void _execute_alu()
    {
        switch (_opcode)
        {
        case ADD:
            _alu_result = _alu_left + _alu_right;
            break;
        case SUB:
            _alu_result = _alu_left - _alu_right;
            break;
        case LSL:
            _alu_result = _alu_left << (_alu_right & 0xF);
            break;
        case LSR:
            _alu_result = _alu_left >> (_alu_right & 0xF);
            break;
        case ASR:
            _alu_result = _asr(_alu_left, _alu_right);
            break;
        case XOR:
            _alu_result = _alu_left ^ _alu_right;
            break;
        case OR:
            _alu_result = _alu_left | _alu_right;
            break;
        case AND:
            _alu_result = _alu_left & _alu_right;
            break;
        case BRA:
            _test_branch();
            break;
        default:
            break;
        }
    }

    void _load_immediate()
    {
        _inc_addr = true;
        if (_load_word)
        {
            if (_load_high)
            {
                _write_bus_low(_memory[_address]);
                _load_high = false;
            }
            else
            {
                _write_bus_high(_memory[_address]);
                if (_imm_to_idx)
                    _index = _read_bus(false);
                else
                    _alu_right = _read_bus(false);
                ++_cycle;
            }
        }
        else
        {
            _write_bus_low(_memory[_address]);
            if (_imm_to_idx)
                _index = _read_bus(false);
            else
                _alu_right = _read_bus(true);
            ++_cycle;
        }
    }

    void _load()
    {
        if (_left & MEM_WORD)
        {
            if (_load_high)
            {
                _inc_addr = true;
                _write_bus_low(_memory[_address]);
                _load_high = false;
            }
            else
            {
                _write_bus_high(_memory[_address]);
                _register[_dest] = _read_bus(false);
                _address = _temp_pc;
                _cycle = 0;
            }
        }
        else
        {
            _write_bus_low(_memory[_address]);
            _register[_dest] = _read_bus(_left & MEM_SEX);
            _address = _temp_pc;
            _cycle = 0;
        }
    }

    void _store()
    {
        if (_left & MEM_WORD)
        {
            if (_load_high)
            {
                _inc_addr = true;
                _write_bus_low(_register[_dest]);
                _memory[_address] = _read_bus(false);
                _load_high = false;
            }
            else
            {
                _write_bus_low(_register[_dest] >> 8);
                _memory[_address] = _read_bus(false);
                _address = _temp_pc;
                _cycle = 0;
            }
        }
        else
        {
            _write_bus_low(_register[_dest]);
            _memory[_address] = _read_bus(false);
            _address = _temp_pc;
            _cycle = 0;
        }
    }

    void _cycle_0()
    {
        _inc_addr = true;
        _write_bus_low(_memory[_address]);
        ++_cycle;
    }

    void _cycle_1()
    {
        _inc_addr = true;
        _write_bus_high(_memory[_address]);
        _decode(_read_bus(false));
        ++_cycle;
    }

    void _cycle_2()
    {
        if (_load_imm)
        {
            _load_immediate();
        }
        else
        {
            _write_bus(_register[_right]);
            _alu_right = _read_bus(false);
            ++_cycle;
        }
    }

    void _cycle_3()
    {
        if (_opcode <= BRA)
        { // arithmetic or branch
            _write_bus(_register[_left]);
            _alu_left = _read_bus(false);
            ++_cycle;
        }
        else if (_opcode == JMP)
        {
            _inc_addr = _imm_to_idx;
            _write_bus(_address);
            _register[_dest] = _read_bus(false);
            ++_cycle;
        }
        else if (_opcode == MEM)
        {
            _temp_pc = _address;
            _write_bus(_register[_right]);
            _address = _read_bus(false);
            ++_cycle;
        }
    }

    void _cycle_4()
    {
        if (_opcode <= AND)
        { // arithmetic
            _execute_alu();
            _write_bus(_alu_result);
            _register[_dest] = _read_bus(false);
            _cycle = 0;
        }
        else if (_opcode == BRA)
        {
            _inc_addr = true;
            _write_bus_low(_memory[_address]);
            _execute_alu();
            ++_cycle;
        }
        else if (_opcode == JMP)
        {
            if (!_imm_to_idx)
            {
                _write_bus(_alu_right);
                _address = _read_bus(false);
            }
            else
            {
                _write_bus(_register[_right]);
                _address = _read_bus(false);
            }
            _cycle = 0;
        }
        else if (_opcode == MEM)
        {
            if (_left & MEM_LOAD)
                _load();
            else
                _store();
        }
    }

    void _cycle_5()
    {
        if (_opcode == BRA)
        {
            if (_take_branch)
                _index = _read_bus(false);
            _cycle = 0;
        }
    }

public:
    CPU() : _memory(new uint8_t[MEM_SIZE])
    {
        std::fill(&_memory[0], &_memory[MEM_SIZE], 0);
        _register.fill(0);
    }

    void update()
    {
        _register[0] = 0;
        if (_inc_addr)
        {
            _address += 1;
            _inc_addr = false;
        }
        else
        {
            _address += (int8_t)_index;
            _index = 0;
        }

        switch (_cycle)
        {
            case 0: _cycle_0(); break;
            case 1: _cycle_1(); break;
            case 2: _cycle_2(); break;
            case 3: _cycle_3(); break;
            case 4: _cycle_4(); break;
            case 5: _cycle_5(); break;
            default: break;
        }
    }

    template <std::ranges::forward_range Range>
    requires std::convertible_to<std::ranges::range_value_t<Range>, uint8_t>
    void load_memory(const Range& data, uint16_t address)
    {
        auto end = std::ranges::end(data);
        for (auto it = std::ranges::begin(data); it != end; ++it)
            _memory[address++] = *it;
    }

    void debug_print()
    {
        std::cout << std::dec << "Cycle: " << (int)_cycle << '\n';
        std::cout << std::hex << "Instruction: " << (_opcode << 12 | (_dest << 8) | (_left << 4) | _right) << '\n';
        std::cout << std::hex;
        std::cout << "Address: " << _address << '\n';
        std::cout << "Temporary PC: " << _temp_pc << '\n';
        std::cout << "Bus: " << _bus << '\n';
        std::cout << "ALU Left: " << _alu_left << '\n';
        std::cout << "ALU Right: " << _alu_right << '\n';
        std::cout << "ALU Result: " << _alu_result << '\n';
        std::cout << "Take branch: " << _take_branch << '\n';
        for (int i = 0; i < 16; ++i)
            std::cout << "r" << i << ": " << _register[i] << '\n';
    }
};

std::vector<uint8_t> read_binary_file(const std::string& filename)
{
    std::ifstream file(filename, file.binary | file.ate);
    std::vector<uint8_t> memory(file.tellg());
    file.seekg(file.beg);
    file.read(reinterpret_cast<char*>(memory.data()), memory.size());
    return memory;
}

void clear() {
    std::cout << "\033[2J\033[1;1H";
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cout << "Usage: ./test [PROGRAM]\n";
        return 1;
    }

    CPU cpu;
    std::vector<uint8_t> memory = read_binary_file(argv[1]);
    if (memory.size() != cpu.MEM_SIZE)
    {
        std::cout << "Invalid input file.\n";
        return 2;
    }
    cpu.load_memory(memory, 0);

    while (true)
    {
        clear();
        cpu.debug_print();
        int c = std::cin.get();
        if (c == EOF)
            break;
        cpu.update();
    }
    clear();
}