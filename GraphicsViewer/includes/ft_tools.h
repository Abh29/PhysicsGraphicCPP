#ifndef FTGRAPHICS_FT_TOOLS_H
#define FTGRAPHICS_FT_TOOLS_H

#include "ft_headers.h"

namespace ft {

struct tools {

  static bool fileExists(const std::string &filename) {
    std::ifstream f(filename.c_str());
    return !f.fail();
  }
};

} // namespace ft

#endif // FTGRAPHICS_FT_TOOLS_H
