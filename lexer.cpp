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
        consume_char();
        while (!is_eof() && is_id_continue(peek()))
            consume_char();

        auto result = string(src.data() + token_start.current, src.data() + state.current);
        return Token::from_string(std::move(result), token_start.to_source_loc(filepath));
    }
    consume_char();
    switch (current)
    {
    case ':':
        return {TokenType::Colon, "", token_start.to_source_loc(filepath)};
    case '.':
        return {TokenType::Dot, "", token_start.to_source_loc(filepath)};
    case '$':
        return {TokenType::Dollar, "", token_start.to_source_loc(filepath)};
    case ',':
        return {TokenType::Comma, "", token_start.to_source_loc(filepath)};
    default: break;
    }
    cerr << "UNEXPECTED CHARACTER: " << current << endl;
    exit(-1); // TODO: Proper error handling from lexer.
}

SourceLocation Lexer::source_loc() const
{
    return state.to_source_loc(filepath);
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
    if (is_eof()) throw std::out_of_range("lexer: out of range");
    return src[state.current];
}

///////////////////////////////////////////////////////////////////////////////
//                                   TOKEN                                   //
///////////////////////////////////////////////////////////////////////////////


Token::Token(TokenType type, std::string&& text, SourceLocation loc) : type(type), text(text), loc(loc)
{
}

Token Token::eof(SourceLocation loc)
{
    return {TokenType::Eof, "", loc};
}

Token Token::from_string(std::string&& str, SourceLocation loc)
{
    auto kind = op::kind_from_word(str);
    if (kind == TokenType::Identifier)
    {
        return {TokenType::Identifier, std::move(str), loc};
    }

    return {kind, "", loc};
}


bool Token::is_eof() const
{
    return type == TokenType::Eof;
}

std::ostream& operator<<(std::ostream& os, const Token& obj)
{
    os
        << obj.loc << ": "
        << get_string(obj.type);
    if (!obj.text.empty())
    {
        os << " text: " << obj.text;
    }
    return os;
}

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
    static_assert(static_cast<int>(TokenType::COUNT) == 15);
    switch (t)
    {
    case TokenType::Eof: return "EOF";
    case TokenType::Identifier: return "Identifier";
    case TokenType::Colon: return "Colon";
    case TokenType::Comma: return "Comma";
    case TokenType::Dot: return "Dot";
    case TokenType::Dollar: return "Dollar";

    case TokenType::Add: return "add";
    case TokenType::Addi: return "addi";
    case TokenType::Addu: return "addu";
    case TokenType::Addiu: return "addiu";
    case TokenType::Or: return "or";
    case TokenType::And: return "and";
    case TokenType::Jr: return "jr";
    case TokenType::J: return "j";

    case TokenType::FIRST_INSTRUCTION:
    case TokenType::COUNT:
        fprintf(stderr, "ERROR: TokenType::FIRST_INSTRUCTION & TokenType::COUNT should never be used\n");
        exit(-1);
    default: assert(false && "UNREACHABLE");
    }
}
