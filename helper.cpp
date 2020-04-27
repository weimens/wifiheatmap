#include "helper.h"

#include <iomanip>
#include <sstream>

std::string mac_addr(unsigned char *arg) {
  std::stringstream ss;
  for (int i = 0; i < 6; ++i) {
    if (i > 0)
      ss << ":";
    ss << std::setfill('0') << std::setw(2) << std::hex << (int)arg[i];
  }
  return ss.str();
}
