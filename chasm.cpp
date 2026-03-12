#include <array>
#include <cassert>

#include <iostream>
#include <memory>
#include <ostream>
#include <string_view>
#include <vector>
#include "register.hpp"
#include "util.hpp"

#include "lexer.hpp"
#include "op.hpp"

using namespace std;



class Assembler
{
    vector<unique_ptr<Item>> ir;
public:
    void add(Reg destination, Reg a, Reg b)
    {
        ir.push_back(make_unique<op::Add>(destination, a, b));
    }

    void addi(Reg destination, Reg a, u16 value)
    {
        ir.push_back(make_unique<op::Addi>(destination, a, value));
    }
};

bool parse_function(Lexer& lexer, Assembler& as)
{
    bool stop = false;
    do
    {
        Token t = lexer.next_token();
        cout << t << endl;

        switch (t.type)
        {
        case TokenType::Eof:
            stop = true;
            break;
        case TokenType::Identifier:
            break;
        case TokenType::Comma:
        case TokenType::Colon:
            break;
        case TokenType::Dot:
            break;
        case TokenType::Dollar:
            break;
        case TokenType::Add:
        case TokenType::Addu:
        case TokenType::Addiu:
        case TokenType::Addi:

        case TokenType::Or:
        case TokenType::And:
            break;
        case TokenType::J:
        case TokenType::Jr:
            assert(false && "NOT IMPLEMENTED: J\n");
        case TokenType::FIRST_INSTRUCTION:
        case TokenType::COUNT:
            fprintf(stderr, "ERROR: TokenType::FIRST_INSTRUCTION & TokenType::COUNT should never be used\n");
            exit(-1);
        default: assert(false && "UNREACHABLE");
        }
        if (stop) break;
    }
    while (!lexer.is_eof());
    return true;
}

int main()
{
    using namespace op;
    Assembler func{};
    assert(Add(Reg::t0, Reg::t1, Reg::t2).encode() == 0x12a4020);
    assert(Addi(Reg::t0, Reg::s1, -50).encode() == 0x2228ffce);
    const char* path = "main.asm";
    auto contents = read_file_to_string(path);

    auto view = string_view(contents);
    auto lexer = Lexer(view, path);
    auto assembler = Assembler{};
    if (!parse_function(lexer, assembler)) return -1;


    return 0;
}
