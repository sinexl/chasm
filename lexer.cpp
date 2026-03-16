#include "lexer.hpp"

#include <cassert>
#include <iostream>
#include <ostream>
#include <stdexcept>

#include "op.hpp"
#include "util.hpp"

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
            auto r_instruction = op::r_format_strings.find(id_view);
            auto i_instruction = op::i_format_strings.find(id_view);
            if (r_instruction != op::r_format_strings.end())
            {
                // Assert mutual exclusivity between hash sets.
                assert(i_instruction == op::i_format_strings.end());
                return Token::r_format(token_start_loc(), r_instruction->second);
            }
            if (i_instruction != op::i_format_strings.end())
            {
                // Assert mutual exclusivity between hash sets.
                assert(r_instruction == op::r_format_strings.end());
                return Token::i_format(token_start_loc(), i_instruction->second);
            }

            return Token::identifier(token_start_loc(), id);
        }
    }

    switch (current)
    {
    case ',':
        consume_char();
        return Token::comma(token_start_loc());
    default: break;
    // Register / Dollar label.
    case '$':
        consume_char();
        string_view word_view = consume_word();
        bool label_definition = !is_eof() && peek() == ':';

        if (!label_definition)
        {
            auto reg_type = reg::strings.find(word_view);
            if (reg_type == reg::strings.end())
            {
                return Token::identifier(token_start_loc(), string(word_view));
            }
            return Token::reg(token_start_loc(), reg_type->second);
        }
        consume_char();
        return Token::label_definition(token_start_loc(), string(word_view));
    }

    consume_char();
    cerr << "UNEXPECTED CHARACTER: " << current << endl;
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
    case TokenType::RFormatInstruction: return "r format instruction";
    case TokenType::IFormatInstruction: return "i format instruction";
    }
    assert(false && "UNREACHABLE");
}

Token::Token(TokenType type, SourceLocation loc, string text) : type(type), loc(loc), text_(std::move(text)),
                                                                register_(reg::Reg::zero),
                                                                r_format_(RFormatInstruction::COUNT),
                                                                i_format_(IFormatInstruction::COUNT)
{
}

Token::Token(SourceLocation loc, reg::Reg reg) : type(TokenType::Register), loc(loc), text_(""), register_(reg),
                                                 r_format_(RFormatInstruction::COUNT),
                                                 i_format_(IFormatInstruction::COUNT)
{
}

Token::Token(SourceLocation loc, RFormatInstruction instruction) : type(TokenType::RFormatInstruction), loc(loc),
                                                                   text_(""),
                                                                   register_(reg::Reg::zero),
                                                                   r_format_(instruction),
                                                                   i_format_(IFormatInstruction::COUNT)
{
}

Token::Token(SourceLocation loc, IFormatInstruction instruction) : type(TokenType::RFormatInstruction), loc(loc),
                                                                   text_(""),
                                                                   register_(reg::Reg::zero),
                                                                   r_format_(RFormatInstruction::COUNT),
                                                                   i_format_(instruction)
{
}

Token Token::eof(SourceLocation loc)
{
    return Token(TokenType::Eof, loc, "");
}

Token Token::r_format(SourceLocation loc, RFormatInstruction instruction)
{
    return {loc, instruction};
}

Token Token::i_format(SourceLocation loc, IFormatInstruction instruction)
{
    return {loc, instruction};
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
    return Token(TokenType::Comma, loc, "");
}

Token Token::reg(SourceLocation loc, Reg reg)
{
    return Token(loc, reg);
}

std::string Token::get_text() const
{
    if (type != TokenType::Identifier && type != TokenType::LabelDefinition)
    {
        throw std::out_of_range("lexer: token has no text");
    }
    return this->text_;
}

Reg Token::get_reg() const
{
    if (type != TokenType::Register)
    {
        throw std::out_of_range("lexer: token is not a register");
    }
    return this->register_;
}

RFormatInstruction Token::get_r_format() const
{
    if (type != TokenType::RFormatInstruction)
    {
        throw std::out_of_range("lexer: token is not a r format instruction");
    }
    return this->r_format_;
}

IFormatInstruction Token::get_i_format() const
{
    if (type != TokenType::IFormatInstruction)
    {
        throw std::out_of_range("lexer: token is not a i format instruction");
    }
    return this->i_format_;
}


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
    switch (instruction)
    {
    case RFormatInstruction::Add: return "add";
    case RFormatInstruction::Addu: return "addu";
    case RFormatInstruction::Sub: return "sub";
    case RFormatInstruction::Subu: return "subu";
    case RFormatInstruction::Or: return "or";
    case RFormatInstruction::And: return "and";
    case RFormatInstruction::Jr: return "jr";

    case RFormatInstruction::COUNT:
        throw std::out_of_range("lexer: invalid r_format instruction token");
    }
    assert(false && "UNREACHABLE");
};

const char* get_string(IFormatInstruction instruction)
{
    switch (instruction)
    {
    case IFormatInstruction::Addi: return "addi";
    case IFormatInstruction::Addiu: return "addiu";
    case IFormatInstruction::COUNT:
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
    return os;
}
