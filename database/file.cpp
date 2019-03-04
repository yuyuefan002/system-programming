#include "file.h"

File::File(std::string filename) { ifs.open(filename, std::ifstream::in); }
File::~File() { ifs.close(); }
std::string File::getNextLine() {
  std::string str;
  getline(ifs, str);
  if (str.empty())
    return {};
  return str;
}
