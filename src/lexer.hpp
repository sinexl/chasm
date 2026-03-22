#ifndef CHASM_LEXER_HPP
#define CHASM_LEXER_HPP
#include <string_view>
#include <variant>

#include "op.hpp"
#include "register.hpp"
#include "util.hpp"
using namespace op;
using std::size_t;

bool is_id_start(char c);

bool is_id_continue(char c);

enum class TokenType: int
{
    Eof,
    Identifier,
    Comma,
    LabelDefinition,

    Integer,

    Register,
    RFormatInstruction,
    IFormatInstruction,
    JFormatInstruction,
};

const char* get_string(TokenType t);
const char* get_string(RFormatInstruction instruction);
const char* get_string(IFormatInstruction instruction);

using Integer = std::variant<i64, u64>;

std::ostream& operator<<(std::ostream& os, const Integer& obj);

class Token
{
public:
    using Value = std::variant<
        std::monostate,          // for tokens without payload (e.g. comma, eof)
        std::string,
        Integer,
        Reg,
        RFormatInstruction,
        IFormatInstruction,
        JFormatInstruction
    >;

private:
    TokenType type;
    SourceLocation loc;
    Value value;

    Token(TokenType type, SourceLocation loc, Value value);

public:
    static Token eof(SourceLocation loc);
    static Token r_format(SourceLocation loc, RFormatInstruction instruction);
    static Token i_format(SourceLocation loc, IFormatInstruction instruction);
    static Token j_format(SourceLocation loc, JFormatInstruction instruction);
    static Token label_definition(SourceLocation loc, std::string name);
    static Token identifier(SourceLocation loc, std::string text);
    static Token comma(SourceLocation loc);
    static Token reg(SourceLocation loc, Reg reg);
    static Token integer(SourceLocation loc, Integer value);

    std::string get_text() const;
    Reg get_reg() const;
    TokenType get_type() const;
    RFormatInstruction get_r_format() const;
    IFormatInstruction get_i_format() const;
    JFormatInstruction get_j_format() const;
    SourceLocation get_source_location() const;
    Integer get_integer() const;

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
    Lexer(std::string_view src, const char* filepath);

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
