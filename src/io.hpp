#ifndef CHASM_IO_HPP
#define CHASM_IO_HPP
#include <filesystem>
#include <iostream>

class AssemblerArgs
{
    std::filesystem::path program_name_;
    std::filesystem::path input_;
    std::filesystem::path output_;
    bool debug_assembler_;
public:
    static void usage(std::filesystem::path program_, std::string_view message = "", std::ostream& err = std::cerr);

    AssemblerArgs(int argc, const char* argv[]);

    const std::filesystem::path& program_name() const;
    const std::filesystem::path& input() const;
    const std::filesystem::path& output() const;

    bool debug_assembler() const;
};

#endif //CHASM_IO_HPP