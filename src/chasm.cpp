#include <array>
#include <cassert>
#include <fstream>
#include <bitset>
#include <filesystem>
#include <iomanip>
#include <variant>

#include <limits>
#include <ostream>
#include <stack>
#include <string_view>
#include <vector>

#include "exceptions.hpp"
#include "format.hpp"
#include "io.hpp"
#include "register.hpp"
#include "util.hpp"

#include "lexer.hpp"
#include "op.hpp"

using namespace reg;

using std::vector;
using std::string;
using std::string_view;
using std::stack;
using std::bitset;

class Assembler
{
    vector<Format*> ir;
    std::unordered_map<string, u32> labels;
    bool no_reorder;

    // holds the pair of instruction and it's address
    stack<std::pair<IFormat*, u32>> i_formats_to_backpatch;
    stack<std::pair<JFormat*, u32>> j_formats_to_backpatch;

    void push_instruction(Format* format)
    {
        ir.push_back(format);
        if (format->is_control_flow() && !no_reorder)
        {
            // FROM MIPS REFERENCE (see README.md -> References)
            /*
             Pre-Release 6: Processor operation is UNPREDICTABLE if a control transfer instruction (CTI) is placed in
             the delay slot of a branch or jump.
             Release 6: If a control transfer instruction (CTI) is executed in the delay slot of a branch or jump,
             Release 6 implementations are required to signal a Reserved Instruction exception.
             */
            // For now, I'm just going to place a NOP instruction after each branching instruction. In the future,
            // smarter reordering will be implemented
            nop();
        }
    }

    // both pc and target_pc are instruction addresses (words), not byte addresses.
    // This function returns whether operation succeeded, or jump is to far away
    [[nodiscard]] static bool calculate_near_jump_pc_relative(u32 pc, u32 target_pc, u16& result)
    {
        // MIPS IMMEDIATE ADDRESSING: Pc-relative.
        // target_pc = (PC_byte + 4) + offset * 4
        // The factor of 4 (or << 2) is chosen because each instruction has length of 4.
        // Since in assembler instructions are addressed by MIPS words (4 bytes), the factor of 4 will be omitted.
        // thus, target_pc = (PC_word + 1) + offset =>  offset = target_pc - (PC_word + 1)

        // Note that static_cast doesn't change internal representation (2's complement) of numbers,
        // we only do the cast for convenience of checking boundaries.
        i32 offset = static_cast<i32>(target_pc) - (static_cast<i32>(pc) + 1);
        if (offset < std::numeric_limits<i16>::min() || offset > std::numeric_limits<i16>::max()) return false;

        result = offset;

        return true;
    }

    // both pc and target_pc are instruction addresses (words), not byte addresses.
    // This function returns whether operation succeeded, or jump is to far away
    [[nodiscard]] static bool calculate_pseudo_direct_jump(u32 pc, u32 target_pc, u32& result)
    {
        u32 current_region = (pc + 1) & REGION_MASK; // take bits [29:26] (the region 256 MB Region)
        u32 target_region = target_pc & REGION_MASK;

        if (current_region != target_region) // If regions differ, jump is long
            return false;

        result = target_pc;
        return true;
    }


    void backpatch_all()
    {
        // TODO: Factor out duplicate code used for backpatching of J & I format instructions.
        while (!i_formats_to_backpatch.empty())
        {
            auto instruction_pair = i_formats_to_backpatch.top();
            i_formats_to_backpatch.pop();

            auto instruction = instruction_pair.first;
            auto pc = instruction_pair.second;

            auto target = labels.find(instruction->get_label());
            if (target == labels.end())
                assert(false && "TODO: Proper error handling"); // TODO: Proper error handling

            u16 bytes;
            calculate_near_jump_pc_relative(pc, target->second, bytes);
            instruction->resolve(bytes);
        }
        while (!j_formats_to_backpatch.empty())
        {
            auto instruction_pair = j_formats_to_backpatch.top();
            j_formats_to_backpatch.pop();

            auto instruction = instruction_pair.first;
            auto pc = instruction_pair.second;

            auto target = labels.find(instruction->get_label());
            if (target == labels.end())
                assert(false && "TODO: Proper error handling"); // TODO: Proper error handling

            u32 final_destination = 0;
            calculate_pseudo_direct_jump(pc, target->second, final_destination);
            instruction->resolve(final_destination);
        }
    }

    Assembler(const Assembler&) = delete;
    Assembler(const Assembler&&) = delete;
    Assembler& operator=(const Assembler& other) = delete;
    Assembler& operator=(Assembler&& other) noexcept = delete;

public:
    Assembler(bool no_reorder = false) : no_reorder(no_reorder)
    {
    }

    void r_format(RFormatInstruction type, Reg dst, Reg a, Reg b)
    {
        push_instruction(new RFormat(type, a, b, dst, 0));
    }

    // Please notice that on ISA level functions
    // "i_format_u16" and "i_format_i16" don't differ (they generate the same code).
    // Both just take the big endian bytes from 'value' parameter.
    // Both functions are provided explicitly in order to make library interface easier to work from C & C++.
    void i_format_u16(IFormatInstruction type, Reg dst, Reg src, u16 value)
    {

        push_instruction(new IFormat(type, src, dst, value));
    }

    void i_format_i16(IFormatInstruction type, Reg dst, Reg src, i16 value)
    {
        push_instruction(new IFormat(type, src, dst, static_cast<u16>(value)));
    }

    void i_format_label(IFormatInstruction type, Reg dst, Reg src, string label)
    {
        u32 pc = ir.size();

        auto addr_pointer = labels.find(label);
        // Label was not defined yet, add the instruction to backpatch list.
        if (addr_pointer == labels.end())
        {
            auto result = new IFormat(type, src, dst, label);
            i_formats_to_backpatch.push(std::make_pair(result, pc));
            push_instruction(result);
            return;
        }

        u16 final_address;
        u32 target = addr_pointer->second;
        calculate_near_jump_pc_relative(pc, target, final_address);
        push_instruction(new IFormat(type, src, dst, final_address));
    }


    void j_format_label(JFormatInstruction type, string label)
    {
        u32 pc = ir.size();
        auto addr_pointer = labels.find(label);
        if (addr_pointer == labels.end())
        {
            auto result = new JFormat(type, label);
            j_formats_to_backpatch.push(std::make_pair(result, pc));
            push_instruction(result);
            return;
        }

        u32 final_address = 0;
        u32 target = addr_pointer->second;
        calculate_pseudo_direct_jump(pc, target, final_address);
        push_instruction(new JFormat(type, final_address));
    }


    // Jump to address found in register.
    void jr(Reg dst)
    {
        push_instruction(new RFormat(RFormatInstruction::Jr, dst, Reg::zero, Reg::zero, 0));
    }

    void label(string name)
    {
        labels.insert({name, ir.size()});
    }

    void nop()
    {
        ir.push_back(new RFormat(RFormatInstruction::Nop, Reg::zero, Reg::zero, Reg::zero, 0));
    }


    void assemble(vector<u8>& data)
    {
        backpatch_all();
        for (auto& item : ir)
        {
            u32 inst = item->encode();
            // Big endian encoding
            u8 bytes[4];
            u32_to_be(bytes, inst);
            data.push_back(bytes[0]);
            data.push_back(bytes[1]);
            data.push_back(bytes[2]);
            data.push_back(bytes[3]);
        }
    }

    ~Assembler()
    {
        for (auto item : ir)
            delete item;
    }
};


Token expect(Lexer& lexer, TokenType expected)
{
    auto token = lexer.next_token();
    auto got_type = token.get_type();
    if (got_type != expected)
    {
        throw UnexpectedToken(token.get_source_location(), expected, got_type);
    }

    return token;
}


void parse_r_format(Lexer& lexer, Assembler& assembler, RFormatInstruction r_format)
{
    Token dest = expect(lexer, TokenType::Register);

    if (r_format == RFormatInstruction::Jr)
    {
        assembler.jr(dest.get_reg());
        return;
    }

    expect(lexer, TokenType::Comma);

    Token a = expect(lexer, TokenType::Register);
    expect(lexer, TokenType::Comma);
    Token b = expect(lexer, TokenType::Register);
    assembler.r_format(r_format, dest.get_reg(), a.get_reg(), b.get_reg());
}

void parse_i_format(Lexer& lexer, Assembler& assembler, IFormatInstruction i_format)
{
    Reg dest = expect(lexer, TokenType::Register).get_reg();
    expect(lexer, TokenType::Comma);
    Reg source = expect(lexer, TokenType::Register).get_reg();
    expect(lexer, TokenType::Comma);

    Token value = lexer.next_token();

    // TODO: Not all i_format instructions should be able to accept a label,
    if (value.get_type() == TokenType::Integer)
    {
        Integer integer = value.get_integer();
        if (std::holds_alternative<i64>(integer))
        {
            i64 val = std::get<i64>(integer);
            if (val < std::numeric_limits<i16>::min() || val > std::numeric_limits<i16>::max())
                throw StaticIntegerOverflow(value.get_source_location(), integer);
            assembler.i_format_i16(i_format, dest, source, static_cast<i16>(val));
        }
        else if (std::holds_alternative<u64>(integer))
        {
            u64 val = std::get<u64>(integer);
            if (val > std::numeric_limits<u16>::max())
                throw StaticIntegerOverflow(value.get_source_location(), integer);

            assembler.i_format_u16(i_format, dest, source, static_cast<u16>(val));
        }
        return;
    }
    if (value.get_type() == TokenType::Identifier)
    {
        assembler.i_format_label(i_format, dest, source, value.get_text());
        return;
    }

    throw UnexpectedToken(value.get_source_location(), {TokenType::Identifier, TokenType::Integer}, TokenType::Comma);
}

void parse_j_format(Lexer& lexer, Assembler& assembler, JFormatInstruction j_format)
{
    string label = expect(lexer, TokenType::Identifier).get_text();
    assembler.j_format_label(j_format, label);
}

void parse_program(Lexer& lexer, Assembler& assembler, const AssemblerArgs& args)
{
    bool stop = false;
    do
    {
        Token t = lexer.next_token();
        if (args.debug_assembler()) std::cout << t.get_source_location() << ": " << t << std::endl;

        switch (t.get_type())
        {
        case TokenType::Eof:
            stop = true;
            break;
        case TokenType::LabelDefinition:
            assembler.label(t.get_text());
            break;

        case TokenType::RFormatInstruction:
            {
                auto instruction = t.get_r_format();
                if (instruction == RFormatInstruction::Nop) assembler.nop();
                else parse_r_format(lexer, assembler, instruction);
                break;
            }
        case TokenType::IFormatInstruction:
            {
                auto instruction = t.get_i_format();
                parse_i_format(lexer, assembler, instruction);
                break;
            }

        case TokenType::JFormatInstruction:
            parse_j_format(lexer, assembler, t.get_j_format());
            break;

        case TokenType::Identifier:
            // I.e, assembler directives
            break;

        case TokenType::Register:
        case TokenType::Comma:
        case TokenType::Integer:
            throw UnexpectedToken(t.get_source_location(), TokenType::RFormatInstruction, t.get_type());
        }
        if (stop) break;
    }
    while (!lexer.is_eof());
}


void debug_result(const vector<u8>& result)
{
    for (size_t i = 0; i < result.size(); i += 4)
    {
        u32 inst = u32_from_be(result.data() + i);
        std::cout << std::hex << "0x" << std::setfill('0') << std::setw(8) << inst << std::dec << " (0b" << std::bitset<
            32>(inst) << ") " << inst << std::endl;
    }
}

int main(int argc, const char* argv[])
{
    using namespace op;
    using namespace std;

    assert(RFormat(RFormatInstruction::Add, Reg::t1, Reg::t2, Reg::t0, 0).encode() == 0x12a4020);
    assert(IFormat(IFormatInstruction::Addi ,Reg::s1, Reg::t0, static_cast<u16>(-50)).encode() == 0x2228ffce);

    auto args = AssemblerArgs{argc, argv};

    auto contents = read_file_to_string(args.input());

    auto view = string_view(contents);
    auto lexer = Lexer(view, args.input().c_str());
    auto assembler = Assembler{};
    try
    {
        parse_program(lexer, assembler, args);
    }
    catch (ParserException& e)
    {
        cerr << e << endl;
        exit(-1);
    }

    auto result = vector<u8>{};
    assembler.assemble(result);
    assert(result.size() % 4 == 0);
    if (args.debug_assembler())
    {
        cout << "Decoding each word generated:" << endl;
        debug_result(result);
    }
    {
        std::ofstream file{args.output(), std::ios::binary};

        file.write(reinterpret_cast<char*>(result.data()), result.size());
        file.close();
    }


    return 0;
}
