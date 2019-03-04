#ifndef __FILE_H__
#define __FILE_H__
#include <fstream>
#include <string>
class File {
private:
  std::ifstream ifs;

public:
  File(std::string filename);
  ~File();
  std::string getNextLine();
};

#endif
