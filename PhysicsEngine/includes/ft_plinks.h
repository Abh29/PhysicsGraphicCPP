#ifndef CYCLONE_PLINKS_H
#define CYCLONE_PLINKS_H

#include "ft_def.h"
#include "ft_particle.h"
#include "ft_pcontacts.h"
#include <glm/fwd.hpp>

namespace ft {

/**
 * Links connect two particles together, generating a contact if
 * they violate the constraints of their link. It is used as a
 * base class for cables and rods, and could be used as a base
 * class for springs with a limit to their extension..
 */
class ParticleLink : public ParticleContactGenerator {
public:
  /**
   * Geneates the contacts to keep this link from being
   * violated. This class can only ever generate a single
   * contact, so the pointer can be a pointer to a single
   * element, the limit parameter is assumed to be at least one
   * (zero isn't valid) and the return value is either 0, if the
   * cable wasn't over-extended, or one if a contact was needed.
   *
   * NB: This method is declared in the same way (as pure
   * virtual) in the parent class, but is replicated here for
   * documentation purposes.
   */
  virtual uint32_t addContact(ParticleContact *contact,
                              uint32_t limit) const = 0;

  void setParticles(Particle::raw_ptr p1, Particle::raw_ptr p2);

protected:
  /**
   * Holds the pair of particles that are connected by this link.
   */
  std::array<Particle::raw_ptr, 2> _particles;

  /**
   * Returns the current length of the link.
   */
  real_t currentLength() const;
};

/**
 * Cables link a pair of particles, generating a contact if they
 * stray too far apart.
 */
class ParticleCable : public ParticleLink {
public:
  /**
   * Fills the given contact structure with the contact needed
   * to keep the cable from over-extending.
   */
  uint32_t addContact(ParticleContact *contact, uint32_t limit) const override;

  real_t getMaxLength() const;
  real_t getRestitution() const;
  void setMaxLength(real_t l);
  void setRestitution(real_t r);

protected:
  /**
   * Holds the maximum length of the cable.
   */
  real_t _maxLength;

  /**
   * Holds the restitution (bounciness) of the cable.
   */
  real_t _restitution;
};

/**
 * Rods link a pair of particles, generating a contact if they
 * stray too far apart or too close.
 */
class ParticleRod : public ParticleLink {
public:
  /**
   * Fills the given contact structure with the contact needed
   * to keep the rod from extending or compressing.
   */
  uint32_t addContact(ParticleContact *contact, uint32_t limit) const override;

  void setLength(real_t l);
  real_t getLength() const;

protected:
  /**
   * Holds the length of the rod.
   */
  real_t _length;
};

/**
 * Constraints are just like links, except they connect a particle to
 * an immovable anchor point.
 */
class ParticleConstraint : public ParticleContactGenerator {
public:
  /**
   * Geneates the contacts to keep this link from being
   * violated. This class can only ever generate a single
   * contact, so the pointer can be a pointer to a single
   * element, the limit parameter is assumed to be at least one
   * (zero isn't valid) and the return value is either 0, if the
   * cable wasn't over-extended, or one if a contact was needed.
   *
   * NB: This method is declared in the same way (as pure
   * virtual) in the parent class, but is replicated here for
   * documentation purposes.
   */
  virtual uint32_t addContact(ParticleContact *contact,
                              uint32_t limit) const = 0;

  Particle::raw_ptr getParticle() const;
  glm::vec3 getAnchor() const;
  void setParticle(Particle::raw_ptr p);
  void setAnchor(const glm::vec3 &a);

protected:
  /**
   * Returns the current length of the link.
   */
  real_t currentLength() const;

  /**
   * Holds the particles connected by this constraint.
   */
  Particle::raw_ptr _particle;

  /**
   * The point to which the particle is anchored.
   */
  glm::vec3 _anchor;
};

/**
 * Cables link a particle to an anchor point, generating a contact if they
 * stray too far apart.
 */
class ParticleCableConstraint : public ParticleConstraint {
public:
  /**
   * Fills the given contact structure with the contact needed
   * to keep the cable from over-extending.
   */
  uint32_t addContact(ParticleContact *contact, uint32_t limit) const override;

  real_t getMaxLength() const;
  real_t getRestitution() const;
  void setMaxLength(real_t l);
  void setRestitution(real_t r);

public:
  /**
   * Holds the maximum length of the cable.
   */
  real_t _maxLength;

  /**
   * Holds the restitution (bounciness) of the cable.
   */
  real_t _restitution;
};

/**
 * Rods link a particle to an anchor point, generating a contact if they
 * stray too far apart or too close.
 */
class ParticleRodConstraint : public ParticleConstraint {
public:
  /**
   * Fills the given contact structure with the contact needed
   * to keep the rod from extending or compressing.
   */
  uint32_t addContact(ParticleContact *contact, uint32_t limit) const override;

  real_t getLength() const;
  void setLength(real_t l);

protected:
  /**
   * Holds the length of the rod.
   */
  real_t _length;
};
} // namespace ft

#endif // CYCLONE_CONTACTS_H
