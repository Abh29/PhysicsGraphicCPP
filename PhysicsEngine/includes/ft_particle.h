#ifndef FT_PARTICLE_H
#define FT_PARTICLE_H

#include "ft_def.h"

namespace ft {

class Particle {

public:
  using raw_ptr = Particle *;
  using ref = Particle &;
  using pointer = std::shared_ptr<Particle>;

  Particle() : _inverseMass(1), _damping(0.99) {}

  inline void setPosition(const glm::vec3 &pos) { _position = pos; }
  inline void setVelocity(const glm::vec3 &vel) { _velocity = vel; }
  inline void setAcceleration(const glm::vec3 &acc) { _acceleration = acc; }
  inline void setMass(const real_t mass) {
    assert(mass > 0.0f);
    _inverseMass = 1 / mass;
  }
  inline void setInverseMass(const real_t imass) { _inverseMass = imass; }
  inline void setDamping(const real_t damp) { _damping = damp; }

  inline glm::vec3 getPosition() const { return _position; }
  inline glm::vec3 getVelocity() const { return _velocity; }
  inline glm::vec3 getAcceleration() const { return _acceleration; }

  inline void integrate(const real_t duration) {

    if (_inverseMass <= 0)
      return;
    assert(duration > 0.0f);
    _position += duration * _velocity;
    glm::vec3 acc = _acceleration + _inverseMass * _accumulator;
    _velocity += duration * acc;
    _velocity *= std::pow(_damping, duration);
    clearAccumulator();
  }

  inline void addForce(glm::vec3 force) { _accumulator += force; }
  inline void clearAccumulator() { _accumulator = {0.0f, 0.0f, 0.0f}; }

  raw_ptr ptr() { return this; }
  inline real_t getInverseMass() const { return _inverseMass; }

private:
  glm::vec3 _position;
  glm::vec3 _velocity;
  glm::vec3 _acceleration;
  glm::vec3 _accumulator;
  real_t _inverseMass;
  real_t _damping;
};

} // namespace ft

#endif // !FT_PARTICLE_H
