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


StaticIntegerOverflow::StaticIntegerOverflow(Integer value) : value_(value)
{
    ostringstream ss;
    ss << "Integer value (" << value << ") is to big to fit into a single instruction";
    msg_ = ss.str();
}

Integer StaticIntegerOverflow::value() const
{
    return value_;
}

const char* StaticIntegerOverflow::what() const noexcept
{
    return msg_.c_str();
}
