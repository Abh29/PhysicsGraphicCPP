#include "../includes/ft_pForceGenerator.h"

void ft::ParticleForceRegistry::add(Particle::raw_ptr particle,
                                    ParticleForceGenerator::raw_ptr generator) {
  _registry.push_back(std::make_pair(particle, generator));
}
void ft::ParticleForceRegistry::remove(
    Particle::raw_ptr particle, ParticleForceGenerator::raw_ptr generator) {
  _registry.erase(std::remove(_registry.begin(), _registry.end(),
                              std::make_pair(particle, generator)),
                  _registry.end());
}
void ft::ParticleForceRegistry::clear() { _registry.clear(); }
void ft::ParticleForceRegistry::updateForces(real_t duration) {
  for (auto i = _registry.begin(); i != _registry.end(); ++i)
    i->second->updateForce(i->first, duration);
};

/*******************************ParticleGravity******************************/

ft::ParticleGravity::ParticleGravity(const glm::vec3 &gravity)
    : _gravity(gravity){};

void ft::ParticleGravity::updateForce(Particle::raw_ptr p,
                                      const real_t duration) {
  (void)duration;
  if (p->getInverseMass() == 0)
    return;
  p->addForce(_gravity * (1 / p->getInverseMass()));
}

/*******************************ParticleDrag******************************/

ft::ParticleDrag::ParticleDrag(const real_t k1, const real_t k2)
    : _k1(k1), _k2(k2) {}

void ft::ParticleDrag::updateForce(Particle::raw_ptr p, const real_t duration) {
  auto force = p->getVelocity();
  real_t coeff = glm::l2Norm(force);
  coeff = _k1 * coeff + _k2 * coeff * coeff;
  force = glm::normalize(force);
  force *= -coeff;
  p->addForce(force);
  (void)duration;
}

/*******************************ParticleSpring******************************/

ft::ParticleSpring::ParticleSpring(const Particle::pointer &other,
                                   real_t springConst, real_t restLength)
    : _other(other), _springConst(springConst), _restLength(restLength) {}

void ft::ParticleSpring::updateForce(Particle::raw_ptr p,
                                     const real_t duration) {
  (void)duration;
  glm::vec3 v = p->getPosition() - _other->getPosition();
  real_t l = std::fabs(glm::length(v) - _restLength) * _springConst;
  v = glm::normalize(v) * -l;
  p->addForce(v);
}

/*******************************ParticleAnchoredSpring******************************/

ft::ParticleAnchoredSpring::ParticleAnchoredSpring(const glm::vec3 &anchor,
                                                   real_t springConst,
                                                   real_t restLength)
    : _anchor(anchor), _springConst(springConst), _restLength(restLength) {}

void ft::ParticleAnchoredSpring::updateForce(Particle::raw_ptr p,
                                             const real_t duration) {
  (void)duration;
  glm::vec3 v = p->getPosition() - _anchor;
  real_t l = (glm::length(v) - _restLength) * _springConst;
  v = glm::normalize(v) * l;
  p->addForce(v);
}

/*******************************ParticleBungee******************************/

ft::ParticleBungee::ParticleBungee(const Particle::pointer &other,
                                   real_t springConst, real_t restLenght)
    : _other(other), _springConst(springConst), _restLength(restLenght) {}

void ft::ParticleBungee::updateForce(Particle::raw_ptr p,
                                     const real_t duration) {
  (void)duration;
  glm::vec3 v = p->getPosition() - _other->getPosition();
  real_t l = glm::length(v);
  if (l <= _restLength)
    return;
  l = (l - _restLength) * _springConst;

  v = glm::normalize(v) * l;
  p->addForce(v);
}

/*******************************ParticleBuoyancy******************************/

ft::ParticleBuoyancy::ParticleBuoyancy(real_t maxDepth, real_t volume,
                                       real_t waterHeight, real_t liquidDensity)
    : _maxDepth(maxDepth), _volume(volume), _waterHeight(waterHeight),
      _liquidDensity(liquidDensity) {}

void ft::ParticleBuoyancy::updateForce(Particle::raw_ptr p,
                                       const real_t duration) {
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

/*******************************ParticleFakeSpring******************************/

ft::ParticleFakeSpring::ParticleFakeSpring(const glm::vec3 &anchor,
                                           real_t springConst, real_t damping)
    : _anchor(anchor), _springConst(springConst), _damping(damping) {}

void ft::ParticleFakeSpring::updateForce(Particle::raw_ptr p, real_t duration) {
  if (p->getInverseMass() < std::numeric_limits<real_t>::epsilon())
    return;
  glm::vec3 v = p->getPosition() - _anchor;
  real_t g = 4 * _springConst - _damping * _damping;
  if (g < std::numeric_limits<real_t>::epsilon())
    return;
  g = 0.5f * std::sqrt(g);

  glm::vec3 c = v * (_damping / (2.0f * g)) + p->getVelocity() * (1.0f / g);

  glm::vec3 target = v * std::cos(g * duration) + c * std::sin(g * duration);
  target *= std::exp(-0.5f * duration * _damping);

  glm::vec3 accel =
      (target - v) * (1.0f / duration * duration) - p->getVelocity() * duration;
  p->addForce(accel * (1 / p->getInverseMass()));
}
