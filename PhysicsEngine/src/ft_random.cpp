#include "../includes/ft_random.h"
#include <cstdlib>
#include <ctime>
#include <glm/common.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>

ft::Random::Random()
    : _randomDevice(), _randomGenerator(_randomDevice()),
      _randomDist(0.0f, 1.0f) {
  seed(0);
}

ft::Random::Random(unsigned seed) { Random::seed(seed); }

real_t ft::Random::randomReal() { return _randomDist(_randomGenerator); }

real_t ft::Random::randomReal(real_t min, real_t max) {
  return randomReal() * (max - min) + min;
}

real_t ft::Random::randomReal(real_t scale) { return randomReal() * scale; }

unsigned ft::Random::randomInt(unsigned max) { return randomBits() % max; }

real_t ft::Random::randomBinomial(real_t scale) {
  return (randomReal() - randomReal()) * scale;
}

glm::quat ft::Random::randomQuaternion() {

  glm::vec3 u =
      glm::normalize(glm::vec3(randomReal(-1.0f, 1.0f), randomReal(-1.0f, 1.0f),
                               randomReal(-1.0f, 1.0f)));
  float a = randomReal(0.0f, 2 * glm::pi<real_t>());

  return glm::normalize(glm::angleAxis(a, u));
}

glm::vec3 ft::Random::randomVector(real_t scale) {
  return glm::vec3(randomBinomial(scale), randomBinomial(scale),
                   randomBinomial(scale));
}

glm::vec3 ft::Random::randomXZVector(real_t scale) {
  return glm::vec3(randomBinomial(scale), 0, randomBinomial(scale));
}

glm::vec3 ft::Random::randomVector(const glm::vec3 &scale) {
  return glm::vec3(randomBinomial(scale.x), randomBinomial(scale.y),
                   randomBinomial(scale.z));
}

glm::vec3 ft::Random::randomVector(const glm::vec3 &min, const glm::vec3 &max) {
  auto v = glm::vec3(randomReal(min.x, max.x), randomReal(min.y, max.y),
                     randomReal(min.z, max.z));
  return v;
}
