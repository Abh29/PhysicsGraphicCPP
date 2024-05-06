#include "../includes/ft_plinks.h"
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>

real_t ft::ParticleLink::currentLength() const {
  if (!_particles[0] || !_particles[1])
    return 0;
  glm::vec3 relativePos =
      _particles[0]->getPosition() - _particles[1]->getPosition();

  return glm::length(relativePos);
}

void ft::ParticleLink::setParticles(ft::Particle::raw_ptr p1,
                                    ft::Particle::raw_ptr p2) {
  _particles[0] = p1;
  _particles[1] = p2;
}

/***********************************ParticleCable*********************************/

real_t ft::ParticleCable::getMaxLength() const { return _maxLength; }

real_t ft::ParticleCable::getRestitution() const { return _restitution; }

void ft::ParticleCable::setRestitution(real_t r) { _restitution = r; }

void ft::ParticleCable::setMaxLength(real_t l) { _maxLength = l; }

uint32_t ft::ParticleCable::addContact(ParticleContact::raw_ptr contact,
                                       uint32_t limit) const {
  (void)limit;
  real_t length = currentLength();

  if (length < _maxLength) {
    return 0;
  }

  contact->setParticles(_particles[0], _particles[1]);

  glm::vec3 normal =
      _particles[1]->getPosition() - _particles[0]->getPosition();
  normal = glm::normalize(normal);
  contact->setContactNormal(normal);
  contact->setPenetration(length - _maxLength);
  contact->setRestitution(_restitution);

  return 1;
}

/***********************************ParticleRod*********************************/

uint32_t ft::ParticleRod::addContact(ParticleContact *contact,
                                     uint32_t limit) const {
  (void)limit;

  real_t currentLen = currentLength();

  if (currentLen == _length) {
    return 0;
  }

  contact->setParticles(_particles[0], _particles[1]);

  glm::vec3 normal = glm::normalize(_particles[1]->getPosition() -
                                    _particles[0]->getPosition());

  if (currentLen > _length) {
    contact->setContactNormal(normal);
    contact->setPenetration(currentLen - _length);
  } else {
    contact->setContactNormal(-normal);
    contact->setPenetration(_length - currentLen);
  }

  contact->setRestitution(0.0f);

  return 1;
}

void ft::ParticleRod::setLength(real_t l) { _length = l; }

real_t ft::ParticleRod::getLength() const { return _length; }

/***********************************ParticleConstraint*********************************/

ft::Particle::raw_ptr ft::ParticleConstraint::getParticle() const {
  return _particle;
}

glm::vec3 ft::ParticleConstraint::getAnchor() const { return _anchor; }

void ft::ParticleConstraint::setAnchor(const glm::vec3 &v) { _anchor = v; }

void ft::ParticleConstraint::setParticle(Particle::raw_ptr p) { _particle = p; }

real_t ft::ParticleConstraint::currentLength() const {
  glm::vec3 relativePos = _particle->getPosition() - _anchor;
  return glm::length(relativePos);
}

/***********************************ParticleCableConstraint*********************************/

real_t ft::ParticleCableConstraint::getMaxLength() const { return _maxLength; }

real_t ft::ParticleCableConstraint::getRestitution() const {
  return _restitution;
}

uint32_t ft::ParticleCableConstraint::addContact(ParticleContact *contact,
                                                 uint32_t limit) const {
  (void)limit;

  real_t length = currentLength();

  if (length < _maxLength) {
    return 0;
  }

  contact->setParticles(_particle, nullptr);

  glm::vec3 normal = glm::normalize(_anchor - _particle->getPosition());
  contact->setContactNormal(normal);
  contact->setPenetration(length - _maxLength);
  contact->setRestitution(_restitution);

  return 1;
}

/***********************************ParticleRodConstraint*********************************/

uint32_t ft::ParticleRodConstraint::addContact(ParticleContact *contact,
                                               uint32_t limit) const {
  (void)limit;

  real_t currentLen = currentLength();

  if (currentLen == _length) {
    return 0;
  }

  contact->setParticles(_particle, nullptr);

  glm::vec3 normal = glm::normalize(_anchor - _particle->getPosition());

  if (currentLen > _length) {
    contact->setContactNormal(normal);
    contact->setPenetration(currentLen - _length);
  } else {
    contact->setContactNormal(-normal);
    contact->setPenetration(_length - currentLen);
  }

  contact->setRestitution(0.0f);

  return 1;
}
