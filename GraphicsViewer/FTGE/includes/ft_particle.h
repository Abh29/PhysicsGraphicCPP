#ifndef FT_PARTICLE_H
#define FT_PARTICLE_H

#include "ft_defines.h"
#include "ft_headers.h"
#include <cassert>
#include <cwchar>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <memory>
#include <vector>

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

  inline void integrate(const real_t duration) {

    if (_inverseMass <= 0)
      return;
    assert(duration > 0.0f);
    _position += duration * _velocity;
    glm::vec3 acc = _acceleration + _inverseMass * _accumator;
    _velocity += duration * acc;
    _velocity *= std::pow(_damping, duration);
    clearAccumulator();
  }

  inline void addForce(glm::vec3 force) { _accumator += force; }
  inline void clearAccumulator() { _accumator = {0.0f, 0.0f, 0.0f}; }

  raw_ptr ptr() { return this; }
  inline real_t getInverseMass() const { return _inverseMass; }

private:
  glm::vec3 _position;
  glm::vec3 _velocity;
  glm::vec3 _acceleration;
  glm::vec3 _accumator;
  real_t _inverseMass;
  real_t _damping;
};

// force generators
class ParticleForceGenerator {
public:
  using raw_ptr = ParticleForceGenerator *;
  virtual void updateForce(Particle::raw_ptr p, const real_t duration) = 0;
};

class ParticleForceRegistry {

public:
  void add(Particle::raw_ptr particle,
           ParticleForceGenerator::raw_ptr generator);
  void remove(Particle::raw_ptr particle,
              ParticleForceGenerator::raw_ptr generator);
  void clear();
  void updateForces(real_t duration) {
    for (auto i = _registry.begin(); i != _registry.end(); ++i)
      i->second->updateForce(i->first, duration);
  };

private:
  std::vector<std::pair<Particle::raw_ptr, ParticleForceGenerator::raw_ptr>>
      _registry;
};

class ParticleGravity : public ParticleForceGenerator {
public:
  using raw_ptr = ParticleGravity *;
  using pointer = std::shared_ptr<ParticleGravity>;

  ParticleGravity(const glm::vec3 &gravity) : _gravity(gravity){};

  void updateForce(Particle::raw_ptr p, const real_t duration) override {
    (void)duration;
    if (p->getInverseMass() == 0)
      return;
    p->addForce(_gravity * (1 / p->getInverseMass()));
  }

private:
  glm::vec3 _gravity;
};

class ParticleDrag : public ParticleForceGenerator {

public:
  using raw_ptr = ParticleDrag *;
  using pointer = std::shared_ptr<ParticleDrag>;

  ParticleDrag(const real_t k1, const real_t k2) : _k1(k1), _k2(k2) {}

  void updateForce(Particle::raw_ptr p, const real_t duration) override {
    auto force = p->getVelocity();
    real_t coeff = glm::l2Norm(force);
    coeff = _k1 * coeff + _k2 * coeff * coeff;
    force = glm::normalize(force);
    force *= -coeff;
    p->addForce(force);
    (void)duration;
  }

private:
  real_t _k1, _k2;
};

} // namespace ft

#endif // !FT_PARTICLE_H
