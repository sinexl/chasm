#ifndef CHASM_EXCEPTIONS_HPP
#define CHASM_EXCEPTIONS_HPP
#include <exception>
#include <sstream>
#include <string>

#include "lexer.hpp"
enum class TokenType;
using namespace std;

class ParserException : std::exception
{
public:
    const char* what() const noexcept override = 0;
};

class UnexpectedToken : ParserException
{
    string msg_;

public:
    TokenType expected;
    TokenType actual;

    UnexpectedToken(TokenType expected, TokenType actual);

    const char* what() const noexcept override;;
};


#endif //CHASM_EXCEPTIONS_HPP
