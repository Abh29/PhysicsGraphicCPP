#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include "algorithm"
#include "any"
#include "array"
#include "chrono"
#include "cstdlib"
#include "cstring"
#include "fstream"
#include "functional"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_access.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/random.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/norm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "iostream"
#include "limits"
#include "map"
#include "memory"
#include "optional"
#include "set"
#include "stdexcept"
#include "thread"
#include "unordered_map"
#include "vector"
#include <cassert>
#include <cmath>
#include <cwchar>
#include <glm/ext/scalar_constants.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/hash.hpp>
#include <limits>
#include <memory>
#include <vector>

typedef float real_t;

// #define real_t float
#define real_pow std::powf;
#define real_sqrt std::sqrt;
#define real_abs std::fabs;
#define real_cos std::cosf;
#define real_sin std::sinf;
#define real_exp std::exp;

namespace ft {

constexpr real_t SLEEP_EPSILON = 0.5f;
};
