#ifndef FT_PCONTACTS_H
#define FT_PCONTACTS_H

#include "ft_def.h"
#include "ft_particle.h"

namespace ft {

class ParticleContactResolver;

class ParticleContact {

  friend class ParticleContactResolver;

public:
  using pointer = std::shared_ptr<ParticleContact>;
  using raw_ptr = ParticleContact *;

  ParticleContact() = default;

  void setParticles(Particle::raw_ptr p1, Particle::raw_ptr p2) {
    _particles[0] = p1;
    _particles[1] = p2;
  }

  void setRestitution(real_t res) { _restitution = res; }

protected:
  void resolve(real_t duration) {
    resolveVelocity(duration);
    resolveInterpenetration(duration);
  }

  real_t calculateSeparatingVelocity() const {
    if (!_particles[0])
      return 0;
    auto v = _particles[0]->getVelocity();
    if (_particles[1])
      v -= _particles[1]->getVelocity();
    return glm::dot(v, _contactNormal);
  }

  void resolveVelocity(real_t duration) {
    if (!_particles[0])
      return;

    // Find the velocity in the direction of the contact.
    real_t separatingVelocity = calculateSeparatingVelocity();
    // Check if it needs to be resolved.
    if (separatingVelocity > 0) {
      // The contact is either separating, or stationary; there’s
      // no impulse required.
      return;
    }
    // Calculate the new separating velocity.
    real_t newSepVelocity = -separatingVelocity * _restitution;
    // Check the velocity buildup due to acceleration only.
    glm::vec3 accCausedVelocity = _particles[0]->getAcceleration();
    if (_particles[1])
      accCausedVelocity -= _particles[1]->getAcceleration();

    real_t accCausedSepVelocity =
        glm::dot(accCausedVelocity, _contactNormal) * duration;

    // If we’ve got a closing velocity due to aceleration buildup,
    // remove it from the new separating velocity.
    if (accCausedSepVelocity < 0) {
      newSepVelocity += _restitution * accCausedSepVelocity;
      // Make sure we haven’t removed more than was
      // there to remove.
      if (newSepVelocity < 0)
        newSepVelocity = 0;
    }
    real_t deltaVelocity = newSepVelocity - separatingVelocity;
    // We apply the change in velocity to each object in proportion to
    // their inverse mass (i.e., those with lower inverse mass [higher
    // actual mass] get less change in velocity).
    real_t totalInverseMass = _particles[0]->getInverseMass();
    if (_particles[1])
      totalInverseMass += _particles[1]->getInverseMass();
    // If all particles have infinite mass, then impulses have no effect.
    if (totalInverseMass <= 0)
      return;
    // Calculate the impulse to apply.
    real_t impulse = deltaVelocity / totalInverseMass;
    // Find the amount of impulse per unit of inverse mass.
    glm::vec3 impulsePerIMass = impulse * _contactNormal;
    // Apply impulses: they are applied in the direction of the contact,
    // and are proportional to the inverse mass.
    _particles[0]->setVelocity(_particles[0]->getVelocity() +
                               impulsePerIMass *
                                   _particles[0]->getInverseMass());
    if (_particles[1]) {
      // Particle 1 goes in the opposite direction.
      _particles[1]->setVelocity(_particles[1]->getVelocity() -
                                 impulsePerIMass *
                                     _particles[1]->getInverseMass());
    }
  }

  void resolveInterpenetration(real_t duration) {
    (void)duration;
    if (_penetration < std::numeric_limits<real_t>::epsilon() || !_particles[0])
      return;
    real_t totalInverse = _particles[0]->getInverseMass();
    if (_particles[1])
      totalInverse += _particles[1]->getInverseMass();
    if (totalInverse < std::numeric_limits<real_t>::epsilon())
      return;
    glm::vec3 movePerIMass = _contactNormal * (_penetration / totalInverse);

    _particleMovement[0] = movePerIMass * _particles[0]->getInverseMass();
    if (_particles[1]) {
      _particleMovement[1] = -movePerIMass * _particles[1]->getInverseMass();
    } else {
      _particleMovement[1] = {0.0f, 0.0f, 0.0f};
    }

    _particles[0]->setPosition(_particles[0]->getPosition() +
                               _particleMovement[0]);
    if (_particles[1]) {
      _particles[1]->setPosition(_particles[1]->getPosition() +
                                 _particleMovement[1]);
    }
  }

  Particle::raw_ptr _particles[2] = {nullptr};
  real_t _restitution;
  glm::vec3 _contactNormal;
  real_t _penetration;
  glm::vec3 _particleMovement[2];
};

class ParticleContactResolver {

public:
  using pointer = std::shared_ptr<ParticleContactResolver>;
  using raw_ptr = ParticleContactResolver *;

  ParticleContactResolver(uint32_t iterations)
      : _iterations(iterations), _iterationsUsed(0) {}

  void setIterations(uint32_t iterations) { _iterations = iterations; }

  void resolveContacts(ParticleContact::raw_ptr contactArray,
                       uint32_t numContacts, real_t duration) {

    uint32_t i;

    _iterationsUsed = 0;
    while (_iterationsUsed < _iterations) {
      // Find the contact with the largest closing velocity;
      real_t max = std::numeric_limits<real_t>::max();
      unsigned maxIndex = numContacts;
      for (i = 0; i < numContacts; i++) {
        real_t sepVel = contactArray[i].calculateSeparatingVelocity();
        if (sepVel < max && (sepVel < 0 || contactArray[i]._penetration > 0)) {
          max = sepVel;
          maxIndex = i;
        }
      }

      // Do we have anything worth resolving?
      if (maxIndex == numContacts)
        break;

      // Resolve this contact
      contactArray[maxIndex].resolve(duration);

      // Update the interpenetrations for all particles
      glm::vec3 *move = contactArray[maxIndex]._particleMovement;

      for (i = 0; i < numContacts; i++) {
        if (contactArray[i]._particles[0] ==
            contactArray[maxIndex]._particles[0]) {
          contactArray[i]._penetration -=
              glm::dot(move[0], contactArray[i]._contactNormal);
        } else if (contactArray[i]._particles[0] ==
                   contactArray[maxIndex]._particles[1]) {
          contactArray[i]._penetration -=
              glm::dot(move[1], contactArray[i]._contactNormal);
        }
        if (contactArray[i]._particles[1]) {
          if (contactArray[i]._particles[1] ==
              contactArray[maxIndex]._particles[0]) {
            contactArray[i]._penetration +=
                glm::dot(move[0], contactArray[i]._contactNormal);
          } else if (contactArray[i]._particles[1] ==
                     contactArray[maxIndex]._particles[1]) {
            contactArray[i]._penetration +=
                glm::dot(move[1], contactArray[i]._contactNormal);
          }
        }
      }
      _iterationsUsed++;
    }
  }

protected:
  uint32_t _iterations;
  uint32_t _iterationsUsed;
};

}; // namespace ft

#endif // !FT_PCONTACTS_H
