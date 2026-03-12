#include "util.hpp"
#include <fstream>

std::string read_file_to_string(const char* filename) {
  std::ifstream stream{filename};
  std::string file_contents{std::istreambuf_iterator(stream),
                            std::istreambuf_iterator<char>()};
  return file_contents;
}

std::ostream& operator<<(std::ostream& os, const SourceLocation& loc)
{
    return os << loc.file << ":" << loc.line << ":" << loc.offset;
}
