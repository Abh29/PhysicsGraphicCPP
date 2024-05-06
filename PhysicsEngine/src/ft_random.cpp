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

void ft::Random::seed(unsigned s) {
  if (s == 0) {
    s = (unsigned)clock();
  }

  for (unsigned i = 0; i < 17; i++) {
    s = s * 2891336453 + 1;
    buffer[i] = s;
  }

  p1 = 0;
  p2 = 10;
}

unsigned ft::Random::rotl(unsigned n, unsigned r) {
  return (n << r) | (n >> (32 - r));
}

unsigned ft::Random::rotr(unsigned n, unsigned r) {
  return (n >> r) | (n << (32 - r));
}

unsigned ft::Random::randomBits() {
  unsigned result;

  result = buffer[p1] = rotl(buffer[p2], 13) + rotl(buffer[p1], 9);

  if (--p1 < 0)
    p1 = 16;
  if (--p2 < 0)
    p2 = 16;

  return result;
}

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
