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
            auto instruction = op::strings.find(id_view);
            if (instruction == op::strings.end())
            {
                return Token::identifier(token_start_loc(), id);
            }
            return Token::instruction(token_start_loc(), instruction->second);
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
    case TokenType::Register:
        return "register";

    // Instructions
    case TokenType::Add: return "add";
    case TokenType::Sub: return "sub";
    case TokenType::Addi: return "addi";
    case TokenType::Addu: return "addu";
    case TokenType::Addiu: return "addiu";
    case TokenType::Or: return "or";
    case TokenType::And: return "and";
    case TokenType::J: return "j";
    case TokenType::Jr: return "jr";

    case TokenType::FIRST_INSTRUCTION:
    case TokenType::COUNT:
        throw std::out_of_range("ERROR: TokenType::FIRST_INSTRUCTION & TokenType::COUNT should never be used");
    }
    assert(false && "UNREACHABLE");
}

Token::Token(TokenType type, SourceLocation loc, string text) : type(type), loc(loc), str_(std::move(text)),
                                                                reg_(reg::Reg::zero)
{
}

Token::Token(SourceLocation loc, reg::Reg reg) : type(TokenType::Register), loc(loc), str_(""), reg_(reg)
{
}

Token Token::eof(SourceLocation loc)
{
    return Token(TokenType::Eof, loc, "");
}

Token Token::instruction(SourceLocation loc, TokenType type)
{
    int instruction = static_cast<int>(type);
    if (instruction <= static_cast<int>(TokenType::FIRST_INSTRUCTION))
    {
        throw std::out_of_range("lexer: provided token type is not an instruction");
    }
    return {type, loc, ""};
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

Token Token::reg(SourceLocation loc, reg::Reg reg)
{
    return Token(loc, reg);
}

std::string Token::get_text() const
{
    if (type != TokenType::Identifier && type != TokenType::LabelDefinition)
    {
        throw std::out_of_range("lexer: token has no text");
    }
    return this->str_;
}

reg::Reg Token::get_reg() const
{
    if (type != TokenType::Register)
    {
        throw std::out_of_range("lexer: token is not a register");
    }
    return this->reg_;
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

std::ostream& operator<<(std::ostream& os, const Token& obj)
{
    auto type = obj.get_type();
    os
        << obj.get_source_location() << ": "
        << get_string(type);
    if (type == TokenType::LabelDefinition || type == TokenType::Identifier)
    {
        os << " text: " << obj.get_text();
    }
    else if (type == TokenType::Register)
    {
        os << " reg: $" << static_cast<int>(obj.get_reg());
    }
    return os;
}
