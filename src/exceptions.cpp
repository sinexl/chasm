#include "exceptions.hpp"

#include <cassert>
#include <iostream>
#include <vector>


ParserException::ParserException(SourceLocation location) : location(location)
{
}

const char* ParserException::what() const noexcept
{
    assert(false && "ERROR: ParserException::what() should never be used. Use Exception::write() instead.");
}

void ParserException::write(std::ostream& os) const
{
    os << location << ": ";
    this->display(os);
}

std::ostream& operator<<(std::ostream& os, const ParserException& ex)
{
    ex.write(os);
    return os;
}

SourceLocation ParserException::get_location() const
{
    return location;
}

UnexpectedToken::UnexpectedToken(SourceLocation location, TokenType expected, TokenType actual)
    : ParserException(location), expected(std::vector{expected}), actual(actual)
{
}


UnexpectedToken::UnexpectedToken(SourceLocation location, std::vector<TokenType> expected, TokenType actual) :
    ParserException(location), expected(std::move(expected)), actual(actual)
{
}

void UnexpectedToken::display(std::ostream& os) const noexcept
{
    os << "error: unexpected token. Expected ";
    for (size_t i = 0; i < this->expected.size(); ++i)
    {
        os << get_string(this->expected[i]);
        if (i != this->expected.size() - 1)
            os << ", ";
    }

    os << ", but got: " << get_string(actual);
}

StaticIntegerOverflow::StaticIntegerOverflow(SourceLocation location, Integer value)
    : ParserException(location), value_(value)
{
}

Integer StaticIntegerOverflow::value() const
{
    return value_;
}

void StaticIntegerOverflow::display(std::ostream& os) const noexcept
{
    os << "Could not fit integer (" << value_ << ") into a single instruction";
}
