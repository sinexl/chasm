#include "lexer.hpp"

#include <array>
#include <cassert>
#include <charconv>
#include <variant>
#include <iostream>
#include <ostream>
#include <stdexcept>

#include "op.hpp"
#include "util.hpp"

using std::string;
using std::string_view;

std::ostream& operator<<(std::ostream& os, const Integer& obj)
{
    if (std::holds_alternative<u64>(obj))
        os << std::get<u64>(obj);
    else if (std::holds_alternative<i64>(obj))
        os << std::get<i64>(obj);

    return os;
}

Lexer::State::State() : current(0), line_start(0), line_number(1)
{
}

SourceLocation Lexer::State::to_source_loc(const char* path) const
{
    return SourceLocation{path, line_number, (current - line_start + 1)};
}

Lexer::Lexer(string_view src, const char* filepath) : src(src), filepath(filepath)
{
}

Token Lexer::next_token()
{
    if (is_eof())
        return Token::eof(source_loc());
    while (isspace(peek()))
    {
        consume_char();
        if (is_eof()) return Token::eof(source_loc());
    }

    if (is_eof()) return Token::eof(source_loc());
    char current = peek();
    token_start = state;

    if (is_id_start(current))
    {
        string_view id_view = consume_identifier();
        auto id = string(id_view);

        if (is_eof()) return Token::identifier(token_start_loc(), id);

        auto save = state;
        current = consume_char();
        switch (current)
        {
        // Lookahead comma: result is label
        case ':':
            return Token::label_definition(token_start_loc(), id);
        default:
            state = save;
            auto r_instruction = r_format_strings.find(id_view);
            auto i_instruction = i_format_strings.find(id_view);
            auto j_instruction = j_format_strings.find(id_view);
            if (r_instruction != r_format_strings.end())
            {
                // Assert mutual exclusivity between hash sets.
                assert(i_instruction == op::i_format_strings.end());
                assert(j_instruction == op::j_format_strings.end());
                return Token::r_format(token_start_loc(), r_instruction->second);
            }
            if (i_instruction != i_format_strings.end())
            {
                // Assert mutual exclusivity between hash sets.
                assert(r_instruction == op::r_format_strings.end());
                assert(j_instruction == op::j_format_strings.end());
                return Token::i_format(token_start_loc(), i_instruction->second);
            }
            if (j_instruction != j_format_strings.end())
            {
                assert(r_instruction == op::r_format_strings.end());
                assert(i_instruction == op::i_format_strings.end());
                return Token::j_format(token_start_loc(), j_instruction->second);
            }

            return Token::identifier(token_start_loc(), id);
        }
    }

    if (isdigit(current) || current == '-')
    {
        bool is_signed = false;
        if (current == '-')
        {
            is_signed = true;
            consume_char();
        }
        while (!is_eof() && isdigit(peek())) consume_char();
        std::from_chars_result result;
        Integer value;
        if (is_signed)
        {
            i64 res;
            result = std::from_chars(src.data() + token_start.current, src.data() + state.current, res);
            value = res;
        }
        else
        {
            u64 res;
            result = std::from_chars(src.data() + token_start.current, src.data() + state.current, res);
            value = res;
        }

        if (result.ec != std::errc())
        {
            std::cerr << "Could not parse integer" << std::endl;
            exit(-1); // TODO: Proper errors from lexer.
        }
        return Token::integer(token_start_loc(), value);
    }

    switch (current)
    {
    case ',':
        consume_char();
        return Token::comma(token_start_loc());
    default: break;
    // Register / Dollar label.
    case '.':
    case '$':
        char punctuation = peek();
        consume_char();
        string_view word_view = consume_word();
        bool label_definition = !is_eof() && peek() == ':';

        if (!label_definition)
        {
            auto reg_type = strings.find(word_view);
            if (reg_type == strings.end() || punctuation == '.')
            {
                return Token::identifier(token_start_loc(), string(word_view));
            }
            return Token::reg(token_start_loc(), reg_type->second);
        }
        consume_char();
        return Token::label_definition(token_start_loc(), string(word_view));
    }

    consume_char();
    std::cerr << "UNEXPECTED CHARACTER: " << current << std::endl;
    exit(-1); // TODO: Proper error handling from lexer.
}

std::string_view Lexer::consume_identifier()
{
    int start = state.current;
    consume_char();
    while (!is_eof() && is_id_continue(peek()))
        consume_char();
    return {src.data() + start, state.current - start};
}


std::string_view Lexer::consume_word()
{
    consume_char();
    while (!is_eof() && isalnum(peek()))
        consume_char();
    return {src.data() + token_start.current, state.current - token_start.current};
}

SourceLocation Lexer::source_loc() const
{
    return state.to_source_loc(filepath);
}

SourceLocation Lexer::token_start_loc() const
{
    return token_start.to_source_loc(filepath);
}

bool Lexer::is_eof() const
{
    return state.current >= src.length();
}

char Lexer::consume_char()
{
    if (is_eof()) throw std::out_of_range("lexer: out of range");
    char c = src[state.current];
    if ('\n' == c)
    {
        state.line_number += 1;
        state.line_start = state.current + 1;
    };

    state.current += 1;
    return c;
}


char Lexer::peek() const
{
    if (is_eof()) throw std::out_of_range("lexer: Lexer::peek() invoked at end of file. ");
    return src[state.current];
}

///////////////////////////////////////////////////////////////////////////////
//                                   TOKEN                                   //
///////////////////////////////////////////////////////////////////////////////


bool is_id_start(char c)
{
    return isalpha(c) || c == '_';
}

bool is_id_continue(char c)
{
    return isalnum(c) || c == '_';
}

const char* get_string(TokenType t)
{
    switch (t)
    {
    // Syntax
    case TokenType::Eof: return "end of file";
    case TokenType::Identifier: return "identifier";
    case TokenType::Comma: return "comma";
    case TokenType::LabelDefinition: return "label definition";
    case TokenType::Register: return "register";
    case TokenType::RFormatInstruction:
    case TokenType::JFormatInstruction:
    case TokenType::IFormatInstruction: return "instruction";
    case TokenType::Integer: return "integer";
    }
    assert(false && "UNREACHABLE");
}

using Value = Token::Value;

Token::Token(TokenType type, SourceLocation loc, Value value)
    : type(type), loc(loc), value(std::move(value))
{
}

Token Token::eof(SourceLocation loc)
{
    return Token(TokenType::Eof, loc, std::monostate{});
}

Token Token::r_format(SourceLocation loc, RFormatInstruction instruction)
{
    return Token(TokenType::RFormatInstruction, loc, instruction);
}

Token Token::j_format(SourceLocation loc, JFormatInstruction instruction)
{
    return Token(TokenType::JFormatInstruction, loc, instruction);
}

Token Token::i_format(SourceLocation loc, IFormatInstruction instruction)
{
    return Token(TokenType::IFormatInstruction, loc, instruction);
}

Token Token::label_definition(SourceLocation loc, std::string name)
{
    return Token(TokenType::LabelDefinition, loc, std::move(name));
}

Token Token::identifier(SourceLocation loc, std::string text)
{
    return Token(TokenType::Identifier, loc, std::move(text));
}

Token Token::comma(SourceLocation loc)
{
    return Token(TokenType::Comma, loc, std::monostate{});
}

Token Token::reg(SourceLocation loc, Reg reg)
{
    return Token(TokenType::Register, loc, reg);
}

Token Token::integer(SourceLocation loc, Integer value)
{
    return Token(TokenType::Integer, loc, value);
}

// -------------------- Getters --------------------

std::string Token::get_text() const
{
    if (type != TokenType::Identifier && type != TokenType::LabelDefinition)
        throw std::out_of_range("lexer: token has no text");

    return std::get<std::string>(value);
}

Reg Token::get_reg() const
{
    if (type != TokenType::Register)
        throw std::out_of_range("lexer: token is not a register");

    return std::get<Reg>(value);
}

RFormatInstruction Token::get_r_format() const
{
    if (type != TokenType::RFormatInstruction)
        throw std::out_of_range("lexer: token is not a r format instruction");

    return std::get<RFormatInstruction>(value);
}

IFormatInstruction Token::get_i_format() const
{
    if (type != TokenType::IFormatInstruction)
        throw std::out_of_range("lexer: token is not a i format instruction");

    return std::get<IFormatInstruction>(value);
}

JFormatInstruction Token::get_j_format() const
{
    if (type != TokenType::JFormatInstruction)
        throw std::out_of_range("lexer: token is not a j format instruction");

    return std::get<JFormatInstruction>(value);
}

Integer Token::get_integer() const
{
    if (type != TokenType::Integer)
        throw std::out_of_range("lexer: token is not an integer");

    return std::get<Integer>(value);
}

// -------------------- Meta --------------------

TokenType Token::get_type() const
{
    return type;
}

SourceLocation Token::get_source_location() const
{
    return loc;
}

bool Token::is_eof() const
{
    return type == TokenType::Eof;
}

const char* get_string(RFormatInstruction instruction)
{
    assert(8 == static_cast<int>(RFormatInstruction::COUNT));
    switch (instruction)
    {
    case RFormatInstruction::Add: return "add";
    case RFormatInstruction::Addu: return "addu";
    case RFormatInstruction::Sub: return "sub";
    case RFormatInstruction::Subu: return "subu";
    case RFormatInstruction::Or: return "or";
    case RFormatInstruction::And: return "and";
    case RFormatInstruction::Jr: return "jr";
    case RFormatInstruction::Nop: return "nop";

    case RFormatInstruction::COUNT:
        throw std::out_of_range("lexer: invalid r_format instruction token");
    }
    assert(false && "UNREACHABLE");
};

const char* get_string(IFormatInstruction instruction)
{
    static_assert(5 == static_cast<int>(IFormatInstruction::COUNT));
    switch (instruction)
    {
    case IFormatInstruction::Addi: return "addi";
    case IFormatInstruction::Addiu: return "addiu";
    case IFormatInstruction::Slti: return "slti";
    case IFormatInstruction::Bne: return "bne";
    case IFormatInstruction::Beq: return "beq";
    case IFormatInstruction::COUNT:
        throw std::out_of_range("lexer: invalid i_format instruction token");
        break;
    }

    assert(false && "UNREACHABLE");
}

const char* get_string(JFormatInstruction instruction)
{
    static_assert(1 == static_cast<int>(JFormatInstruction::COUNT));
    switch (instruction)
    {
    case JFormatInstruction::J:
        return "j";
    case JFormatInstruction::COUNT:
        throw std::out_of_range("lexer: invalid i_format instruction token");
    }

    assert(false && "UNREACHABLE");
}


std::ostream& operator<<(std::ostream& os, const Token& obj)
{
    auto type = obj.get_type();
    os << get_string(type);
    if (type == TokenType::LabelDefinition || type == TokenType::Identifier)
    {
        os << " (" << obj.get_text() << ")";
    }
    else if (type == TokenType::Register)
    {
        os << " ($" << static_cast<int>(obj.get_reg()) << ")";
    }
    else if (type == TokenType::RFormatInstruction)
    {
        os << " (" << get_string(obj.get_r_format()) << ")";
    }
    else if (type == TokenType::IFormatInstruction)
    {
        os << " (" << get_string(obj.get_i_format()) << ")";
    }
    else if (type == TokenType::JFormatInstruction)
    {
        os << " (" << get_string(obj.get_j_format()) << ")";
    }
    return os;
}
