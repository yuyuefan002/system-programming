#ifndef __HELPER_H__
#define __HELPER_H__
#include <string>
class Helper {
public:
  std::string fetchNextSeg(std::string &msg, char c = ' ',
                           size_t substrlen = 1);
  bool arealphas(std::string &str);
  std::string generateValue(std::string &line);
};
#endif
