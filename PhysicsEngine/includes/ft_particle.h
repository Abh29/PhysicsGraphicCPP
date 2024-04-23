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

// force generators
class ParticleForceGenerator {
public:
  using raw_ptr = ParticleForceGenerator *;
  virtual void updateForce(Particle::raw_ptr p, const real_t duration) = 0;
};

class ParticleForceRegistry {

public:
  void add(Particle::raw_ptr particle,
           ParticleForceGenerator::raw_ptr generator) {
    _registry.push_back(std::make_pair(particle, generator));
  }
  void remove(Particle::raw_ptr particle,
              ParticleForceGenerator::raw_ptr generator) {
    _registry.erase(std::remove(_registry.begin(), _registry.end(),
                                std::make_pair(particle, generator)),
                    _registry.end());
  }
  void clear() { _registry.clear(); }
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

class ParticleSpring : public ParticleForceGenerator {
public:
  using pointer = std::shared_ptr<ParticleSpring>;
  using raw_ptr = ParticleSpring *;

  ParticleSpring(const Particle::pointer &other, real_t springConst,
                 real_t restLength)
      : _other(other), _springConst(springConst), _restLength(restLength) {}

  void updateForce(Particle::raw_ptr p, const real_t duration) override {
    (void)duration;
    glm::vec3 v = p->getPosition() - _other->getPosition();
    real_t l = std::fabs(glm::length(v) - _restLength) * _springConst;
    v = glm::normalize(v) * -l;
    p->addForce(v);
  }

private:
  Particle::pointer _other;
  real_t _springConst;
  real_t _restLength;
};

class ParticleAnchoredSpring : public ParticleForceGenerator {
public:
  using pointer = std::shared_ptr<ParticleAnchoredSpring>;
  using raw_ptr = ParticleAnchoredSpring *;

  ParticleAnchoredSpring(const glm::vec3 &anchor, real_t springConst,
                         real_t restLength)
      : _anchor(anchor), _springConst(springConst), _restLength(restLength) {}

  void updateForce(Particle::raw_ptr p, const real_t duration) override {
    (void)duration;
    glm::vec3 v = p->getPosition() - _anchor;
    real_t l = (glm::length(v) - _restLength) * _springConst;
    v = glm::normalize(v) * l;
    p->addForce(v);
  }

private:
  glm::vec3 _anchor;
  real_t _springConst;
  real_t _restLength;
};

class ParticleBungee : public ParticleForceGenerator {
public:
  using pointer = std::shared_ptr<ParticleBungee>;
  using raw_ptr = ParticleBungee *;

  ParticleBungee(const Particle::pointer &other, real_t springConst,
                 real_t restLenght)
      : _other(other), _springConst(springConst), _restLength(restLenght) {}

  void updateForce(Particle::raw_ptr p, const real_t duration) override {
    (void)duration;
    glm::vec3 v = p->getPosition() - _other->getPosition();
    real_t l = glm::length(v);
    if (l <= _restLength)
      return;
    l = (l - _restLength) * _springConst;

    v = glm::normalize(v) * l;
    p->addForce(v);
  }

private:
  Particle::pointer _other;
  real_t _springConst;
  real_t _restLength;
};

class ParticleBuoyancy : public ParticleForceGenerator {
public:
  using pointer = std::shared_ptr<ParticleBuoyancy>;
  using raw_ptr = ParticleBuoyancy *;

  ParticleBuoyancy(real_t maxDepth, real_t volume, real_t waterHeight,
                   real_t liquidDensity = 1000.0f)
      : _maxDepth(maxDepth), _volume(volume), _waterHeight(waterHeight),
        _liquidDensity(liquidDensity) {}

  void updateForce(Particle::raw_ptr p, const real_t duration) override {
    (void)duration;
    real_t depth = p->getPosition().y; // this could be z !
    if (depth >= _waterHeight + _maxDepth)
      return;

    glm::vec3 f(0.0f);
    if (depth <= _waterHeight - _maxDepth) {
      f.y = _liquidDensity * _volume;
    } else {
      f.y = _liquidDensity * _volume * (depth - _maxDepth - _waterHeight) / 2 *
            _maxDepth;
    }
    p->addForce(f);
  }

private:
  real_t _maxDepth;
  real_t _volume;
  real_t _waterHeight;
  real_t _liquidDensity;
};

// stiff springs
class ParticleFakeSpring : public ParticleForceGenerator {
public:
  using pointer = std::shared_ptr<ParticleFakeSpring>;
  using raw_ptr = ParticleFakeSpring *;

  ParticleFakeSpring(const glm::vec3 &anchor, real_t springConst,
                     real_t damping)
      : _anchor(anchor), _springConst(springConst), _damping(damping) {}

  void updateForce(Particle::raw_ptr p, real_t duration) override {
    if (p->getInverseMass() < std::numeric_limits<real_t>::epsilon())
      return;
    glm::vec3 v = p->getPosition() - _anchor;
    real_t g = 4 * _springConst - _damping * _damping;
    if (g < std::numeric_limits<real_t>::epsilon())
      return;
    g = 0.5f * std::sqrt(g);

    glm::vec3 c = v * (_damping / (2.0f * g)) + p->getVelocity() * (1.0f / g);

    // Calculate the target position.
    glm::vec3 target = v * std::cos(g * duration) + c * std::sin(g * duration);
    target *= std::exp(-0.5f * duration * _damping);

    // Calculate the resulting acceleration, and therefore the force.
    glm::vec3 accel = (target - v) * (1.0f / duration * duration) -
                      p->getVelocity() * duration;
    p->addForce(accel * (1 / p->getInverseMass()));
  }

private:
  glm::vec3 _anchor;
  real_t _springConst;
  real_t _damping;
};

} // namespace ft

#endif // !FT_PARTICLE_H
