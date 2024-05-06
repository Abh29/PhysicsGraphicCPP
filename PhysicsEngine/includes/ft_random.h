#ifndef FT_RANDOM_H
#define FT_RANDOM_H

#include "ft_def.h"
#include <glm/fwd.hpp>
#include <random>

namespace ft {

/**
 * Keeps track of one random stream: i.e. a seed and its output.
 * This is used to get random numbers. Rather than a funcion, this
 * allows there to be several streams of repeatable random numbers
 * at the same time. Uses the RandRotB algorithm.
 */
class Random {
public:
  /**
   * left bitwise rotation
   */

  unsigned rotl(unsigned n, unsigned r);
  /**
   * right bitwise rotation
   */
  unsigned rotr(unsigned n, unsigned r);

  /**
   * Creates a new random number stream with a seed based on
   * timing data.
   */
  Random();

  /**
   * Creates a new random stream with the given seed.
   */
  Random(unsigned seed);

  /**
   * Sets the seed value for the random stream.
   */
  void seed(unsigned seed);

  /**
   * Returns the next random bitstring from the stream. This is
   * the fastest method.
   */
  unsigned randomBits();

  /**
   * Returns a random floating point number between 0 and 1.
   */
  real_t randomReal();

  /**
   * Returns a random floating point number between 0 and scale.
   */
  real_t randomReal(real_t scale);

  /**
   * Returns a random floating point number between min and max.
   */
  real_t randomReal(real_t min, real_t max);

  /**
   * Returns a random integer less than the given value.
   */
  unsigned randomInt(unsigned max);

  /**
   * Returns a random binomially distributed number between -scale
   * and +scale.
   */
  real_t randomBinomial(real_t scale);

  /**
   * Returns a random vector where each component is binomially
   * distributed in the range (-scale to scale) [mean = 0.0f].
   */
  glm::vec3 randomVector(real_t scale);

  /**
   * Returns a random vector where each component is binomially
   * distributed in the range (-scale to scale) [mean = 0.0f],
   * where scale is the corresponding component of the given
   * vector.
   */
  glm::vec3 randomVector(const glm::vec3 &scale);

  /**
   * Returns a random vector in the cube defined by the given
   * minimum and maximum vectors. The probability is uniformly
   * distributed in this region.
   */
  glm::vec3 randomVector(const glm::vec3 &min, const glm::vec3 &max);

  /**
   * Returns a random vector where each component is binomially
   * distributed in the range (-scale to scale) [mean = 0.0f],
   * except the y coordinate which is zero.
   */
  glm::vec3 randomXZVector(real_t scale);

  /**
   * Returns a random orientation (i.e. normalized) quaternion.
   */
  glm::quat randomQuaternion();

private:
  // Internal mechanics
  int p1, p2;
  std::random_device _randomDevice;
  std::mt19937 _randomGenerator;
  std::uniform_real_distribution<real_t> _randomDist;
  unsigned buffer[17];
};

} // namespace ft

#endif // FT_RANDOM_H
