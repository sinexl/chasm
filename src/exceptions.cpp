#include "exceptions.hpp"

UnexpectedToken::UnexpectedToken(TokenType expected, TokenType actual): expected(expected), actual(actual)
{
    ostringstream ss;
    ss << "error: unexpected token. Expected " << get_string(expected) << ", but got: " << get_string(actual);
    msg_ = ss.str();
}

const char* UnexpectedToken::what() const noexcept
{
    return msg_.c_str();
}
