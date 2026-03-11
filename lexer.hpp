#ifndef CHASM_LEXER_HPP
#define CHASM_LEXER_HPP
#include <string_view>
#include "util.hpp"
using namespace std;

bool is_id_start(char c);

bool is_id_continue(char c);

enum class TokenType
{
    Eof,
    Identifier,
    Colon,
    Dot,
    Dollar,
    COUNT,
};

const char* get_string(TokenType t);

struct Token
{

    TokenType type;
    std::string text;
    SourceLocation loc;

    Token(TokenType type, std::string&& text, SourceLocation loc);

    static Token eof(SourceLocation loc);


    [[nodiscard]] bool is_eof() const;
};

std::ostream& operator<<(std::ostream& os, const Token& obj);

class Lexer
{
    std::string_view src;
    const char* filepath;

    struct State
    {
        size_t current;
        size_t line_start;
        size_t line_number;
        State();

        SourceLocation to_source_loc(const char* path) const;
    };

    State state;
    State token_start;

public:
    Lexer(string_view src, const char* filepath);

    Token next_token();

    [[nodiscard]] SourceLocation source_loc() const;

    [[nodiscard]] bool is_eof() const;

private:
    char consume_char();

    char peek() const;
};

#endif //CHASM_LEXER_HPP
