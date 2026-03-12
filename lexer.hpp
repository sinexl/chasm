#ifndef CHASM_LEXER_HPP
#define CHASM_LEXER_HPP
#include <string_view>

#include "register.hpp"
#include "util.hpp"
using namespace std;

bool is_id_start(char c);

bool is_id_continue(char c);

enum class TokenType
{
    // Syntax
    Eof,
    Identifier,
    Comma,
    LabelDefinition,

    Register,

    // Instructions
    FIRST_INSTRUCTION,
    Add,
    Addi,
    Addu,
    Addiu,
    Or,
    And,
    J,
    Jr,
    Sub,

    COUNT,
};

const char* get_string(TokenType t);


class Token
{
private:
    TokenType type;
    SourceLocation loc;

    std::string str_;
    reg::Reg reg_;

    Token(TokenType type, SourceLocation loc, string text);
    Token(SourceLocation loc, reg::Reg reg);

public:

    static Token eof(SourceLocation loc);
    static Token instruction(SourceLocation loc, TokenType type);
    static Token label_definition(SourceLocation loc, std::string name);
    static Token identifier(SourceLocation loc, std::string text);
    static Token comma(SourceLocation loc);
    static Token reg(SourceLocation loc, reg::Reg reg);

    std::string get_text() const;
    reg::Reg get_reg() const;
    TokenType get_type() const;
    SourceLocation get_source_location() const;

    bool is_eof() const;
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

    SourceLocation source_loc() const;
    SourceLocation token_start_loc() const;

    bool is_eof() const;

private:
    char consume_char();

    std::string_view consume_identifier();
    std::string_view consume_word();

    char peek() const;
};

#endif //CHASM_LEXER_HPP
