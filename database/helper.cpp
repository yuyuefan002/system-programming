#include "helper.h"

std::string trimLeadingSpace(std::string &msg) {
  if (msg.empty())
    return "";
  size_t target = msg.find_first_not_of(' ');
  if (target == std::string::npos)
    return msg;
  return msg.substr(target);
}

std::string Helper::fetchNextSeg(std::string &msg, char c, size_t substrlen) {
  msg = trimLeadingSpace(msg);
  size_t target = msg.find(c);
  std::string res = msg.substr(0, target);
  if (target != std::string::npos)
    msg = msg.substr(target + substrlen);
  else
    msg = "";

  return res;
}

bool Helper::arealphas(std::string &str) {
  for (auto it = str.begin(); it != str.end(); ++it) {
    if (*it == '.' || *it == '-')
      continue;
    else if (*it == '\'') {
      str.insert(it, '\'');
      ++it;
    } else if (!isalpha(*it))
      return false;
  }
  return true;
}

std::string Helper::generateValue(std::string &line) {
  std::string res;
  std::string seg = fetchNextSeg(line);
  if (arealphas(seg))
    res += "'" + seg + "'";
  else
    res += seg;
  while ((seg = fetchNextSeg(line)) != "") {
    if (arealphas(seg))
      res += ", '" + seg + "'";
    else
      res += ", " + seg;
  }
  return res;
}
