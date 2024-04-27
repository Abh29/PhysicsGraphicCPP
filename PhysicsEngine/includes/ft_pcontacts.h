#ifndef FT_PCONTACTS_H
#define FT_PCONTACTS_H

#include "ft_def.h"
#include "ft_particle.h"
#include <glm/fwd.hpp>

namespace ft {

class ParticleContactResolver;

class ParticleContact {

  friend class ParticleContactResolver;

public:
  using pointer = std::shared_ptr<ParticleContact>;
  using raw_ptr = ParticleContact *;

  ParticleContact() = default;

  void setParticles(Particle::raw_ptr p1, Particle::raw_ptr p2);
  void setRestitution(real_t res);
  void setContactNormal(const glm::vec3 &n);
  void setPenetration(real_t p);

  real_t getRestitution() const;
  real_t getPenetration() const;
  glm::vec3 getContactNormal() const;
  std::array<Particle::raw_ptr, 2> &getParticles();

protected:
  void resolve(real_t duration);
  real_t calculateSeparatingVelocity() const;
  void resolveVelocity(real_t duration);
  void resolveInterpenetration(real_t duration);

  std::array<Particle::raw_ptr, 2> _particles;
  real_t _restitution;
  glm::vec3 _contactNormal;
  real_t _penetration;
  glm::vec3 _particleMovement[2];
};

class ParticleContactResolver {

public:
  using pointer = std::shared_ptr<ParticleContactResolver>;
  using raw_ptr = ParticleContactResolver *;

  ParticleContactResolver(uint32_t iterations);

  void setIterations(uint32_t iterations);
  void resolveContacts(ParticleContact::raw_ptr contactArray,
                       uint32_t numContacts, real_t duration);

protected:
  uint32_t _iterations;
  uint32_t _iterationsUsed;
};

/**
 * This is the basic polymorphic interface for contact generators
 * applying to particles.
 */
class ParticleContactGenerator {
public:
  using pointer = std::shared_ptr<ParticleContactGenerator>;
  using raw_ptr = ParticleContactGenerator *;

  /**
   * Fills the given contact structure with the generated
   * contact. The contact pointer should point to the first
   * available contact in a contact array, where limit is the
   * maximum number of contacts in the array that can be written
   * to. The method returns the number of contacts that have
   * been written.
   */
  virtual unsigned addContact(ParticleContact::raw_ptr contact,
                              unsigned limit) const = 0;
};

}; // namespace ft

#endif // !FT_PCONTACTS_H
