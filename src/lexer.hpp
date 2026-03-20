#ifndef CHASM_LEXER_HPP
#define CHASM_LEXER_HPP
#include <string_view>

#include "op.hpp"
#include "register.hpp"
#include "util.hpp"
using namespace std;
using namespace op;

bool is_id_start(char c);

bool is_id_continue(char c);

enum class TokenType: int
{
    Eof,
    Identifier,
    Comma,
    LabelDefinition,

    Register,
    RFormatInstruction,
    IFormatInstruction,
    JFormatInstruction,
};



const char* get_string(TokenType t);
const char* get_string(RFormatInstruction instruction);
const char* get_string(IFormatInstruction instruction);


class Token
{
    TokenType type;
    SourceLocation loc;

    std::string text_;
    Reg register_;
    RFormatInstruction r_format_;
    IFormatInstruction i_format_;
    JFormatInstruction j_format_;

    Token(TokenType type, SourceLocation loc, string text);
    Token(SourceLocation loc, Reg reg);
    Token(SourceLocation loc, RFormatInstruction r_format);
    Token(SourceLocation loc, IFormatInstruction i_format);
    Token(SourceLocation loc, JFormatInstruction j_format);

public:
    static Token eof(SourceLocation loc);
    static Token r_format(SourceLocation loc, RFormatInstruction instruction);
    static Token i_format(SourceLocation loc, IFormatInstruction instruction);
    static Token j_format(SourceLocation loc, JFormatInstruction instruction);
    static Token label_definition(SourceLocation loc, std::string name);
    static Token identifier(SourceLocation loc, std::string text);
    static Token comma(SourceLocation loc);
    static Token reg(SourceLocation loc, Reg reg);

    std::string get_text() const;
    Reg get_reg() const;
    TokenType get_type() const;
    RFormatInstruction get_r_format() const;
    IFormatInstruction get_i_format() const;
    JFormatInstruction get_j_format() const;
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
