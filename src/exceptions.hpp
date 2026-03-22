#ifndef CHASM_EXCEPTIONS_HPP
#define CHASM_EXCEPTIONS_HPP
#include <exception>
#include <vector>
#include "util.hpp"

#include "lexer.hpp"
enum class TokenType;

class ParserException : public std::exception
{
    SourceLocation location;
public:
    ParserException(SourceLocation location);

    // Should not be used. Use write() (or operator <<) instead
    const char* what() const noexcept override;;

    void write(std::ostream& os) const;

    SourceLocation get_location() const;

protected:
    virtual void display(std::ostream& os) const noexcept = 0;
};

std::ostream& operator<<(std::ostream& os, const ParserException& ex);

class UnexpectedToken : public ParserException
{
public:
    std::vector<TokenType> expected;
    TokenType actual;

    UnexpectedToken(SourceLocation location, TokenType expected, TokenType actual);
    UnexpectedToken(SourceLocation location, vector<TokenType> expected, TokenType actual);

protected:
    void display(std::ostream& os) const noexcept override;
};

class StaticIntegerOverflow : public ParserException
{
    Integer value_;

public:
    StaticIntegerOverflow(SourceLocation location, Integer value);

    Integer value() const;

protected:
    void display(std::ostream& os) const noexcept override;
};

#endif //CHASM_EXCEPTIONS_HPP
