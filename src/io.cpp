#include "io.hpp"

#include <deque>
#include <filesystem>

void AssemblerArgs::usage(std::filesystem::path program_, std::string_view message, std::ostream& err)
{
    if (!message.empty())
        err << "Error: " << message << '\n';

    err << "Usage:\n";
    err << "  " << program_.filename().string() << " <input> [-o <output>] [--debug-assembler]\n\n";
    err << "Arguments:\n";
    err << "  <input>           Input file path\n";
    err << "  -o <output>       Output file path (default: <input>.bin)\n\n";
    err << "  -debug-assembler  output intermediate information during assembling process\n\n";
}

AssemblerArgs::AssemblerArgs(int argc, const char* argv[])
{
    auto args = std::deque<std::string_view>(argv, argv + argc);
    program_name_ = args.front();
    args.pop_front();
    debug_assembler_ = false;

    output_ = "";
    input_ = "";

    if (args.empty())
    {
        usage(program_name_);
        exit(-1);
    }

    while (!args.empty())
    {
        std::string_view arg = args.front();
        args.pop_front();

        if (arg == "-o")
        {
            if (args.empty())
            {
                usage(program_name_, "no output path provided");
                exit(-1);
            }
            output_ = args.front();
            args.pop_front();
        }
        else if (arg == "--debug-assembler")
        {
            debug_assembler_ = true;
        }
        else
        {
            if (!input_.empty())
            {
                usage(program_name_, "multiple input files provided");
                exit(-1);
            }
            input_ = arg;
        }
    }

    if (input_.empty())
    {
        usage(program_name_, "no input path provided");
        exit(-1);
    }
    if (output_.empty())
        output_ = input_.stem().append(".bin");
}

const std::filesystem::path& AssemblerArgs::program_name() const
{ return program_name_; }

const std::filesystem::path& AssemblerArgs::input() const
{ return input_; }

const std::filesystem::path& AssemblerArgs::output() const
{ return output_; }

bool AssemblerArgs::debug_assembler() const { return debug_assembler_; }
