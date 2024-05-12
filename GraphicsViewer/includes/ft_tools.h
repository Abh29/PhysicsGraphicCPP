#ifndef FTGRAPHICS_FT_TOOLS_H
#define FTGRAPHICS_FT_TOOLS_H

#include "ft_headers.h"
#include <glm/fwd.hpp>
#include <tuple>

namespace ft {

struct tools {

  static bool fileExists(const std::string &filename) {
    std::ifstream f(filename.c_str());
    return !f.fail();
  }

  static std::pair<float, glm::vec3>
  getAngleAxisFromRotation(const glm::mat4 &m) {

    float d = glm::clamp(0.5f * (m[0][0] + m[1][1] + m[2][2] - 1), -1.0f, 1.0f);
    float angle = std::acos(d);

    if (std::abs(angle) < 0.001f)
      return {0.0f, {1.0f, 0.0f, 0.0f}};

    float s = std::sqrt((m[2][1] - m[1][2]) * (m[2][1] - m[1][2]) +
                        (m[0][2] - m[2][0]) * (m[0][2] - m[2][0]) +
                        (m[1][0] - m[0][1]) * (m[1][0] - m[0][1]));

    // float s = 2 * std::sin(angle);

    if (std::abs(s) < 0.01f)
      s = 1.0f;

    glm::vec3 v;
    v.x = (m[2][1] - m[1][2]) / s;
    v.y = (m[0][2] - m[2][0]) / s;
    v.z = (m[1][0] - m[0][1]) / s;

    v = -v;
    return std::make_pair(angle, v);
  }
};

} // namespace ft

#endif // FTGRAPHICS_FT_TOOLS_H
