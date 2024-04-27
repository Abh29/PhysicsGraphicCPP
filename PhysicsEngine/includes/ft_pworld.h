#ifndef FT_PWORLD_H
#define FT_PWORLD_H

#include "ft_def.h"
#include "ft_forceGenerator.h"
#include "ft_particle.h"
#include "ft_pcontacts.h"
#include <cstdint>

namespace ft {

/**
 * Keeps track of a set of particles, and provides the means to
 * update them all.
 */
class ParticleWorld {
public:
  using pointer = std::shared_ptr<ParticleWorld>;
  using raw_ptr = ParticleWorld *;

  /**
   * Creates a new particle simulator that can handle up to the
   * given number of contacts per frame. You can also optionally
   * give a number of contact-resolution iterations to use. If you
   * don't give a number of iterations, then twice the number of
   * contacts will be used.
   */
  ParticleWorld(unsigned maxContacts, unsigned iterations = 0);

  /**
   * Deletes the simulator.
   */
  ~ParticleWorld() = default;

  /**
   * Calls each of the registered contact generators to report
   * their contacts. Returns the number of generated contacts.
   */
  unsigned generateContacts();

  /**
   * Integrates all the particles in this world forward in time
   * by the given duration.
   */
  void integrate(real_t duration);

  /**
   * Processes all the physics for the particle world.
   */
  void runPhysics(real_t duration);

  /**
   * Initializes the world for a simulation frame. This clears
   * the force accumulators for particles in the world. After
   * calling this, the particles can have their forces for this
   * frame added.
   */
  void startFrame();

  /**
   *  Returns the list of particles.
   */
  std::vector<Particle::raw_ptr> &getParticles();

  /**
   * Returns the list of contact generators.
   */
  std::vector<ParticleContactGenerator::raw_ptr> &getContactGenerators();

  /**
   * Returns the force registry.
   */
  ParticleForceRegistry &getForceRegistry();

protected:
  /**
   * Holds the particles
   */
  std::vector<Particle::raw_ptr> _particles;

  /**
   * True if the world should calculate the number of iterations
   * to give the contact resolver at each frame.
   */
  bool _calculateIterations;

  /**
   * Holds the force generators for the particles in this world.
   */
  ParticleForceRegistry _registry;

  /**
   * Holds the resolver for contacts.
   */
  ParticleContactResolver _resolver;

  /**
   * Contact generators.
   */
  std::vector<ParticleContactGenerator::raw_ptr> _contactGenerators;

  /**
   * Holds the list of contacts.
   */
  std::vector<ParticleContact> _contacts;

  /**
   * Holds the maximum number of contacts allowed (i.e. the
   * size of the contacts array).
   */
  unsigned _maxContacts;
};

/**
 * A contact generator that takes an STL vector of particle pointers and
 * collides them against the ground.
 */
class GroundContacts : public ParticleContactGenerator {

public:
  using pointer = std::shared_ptr<GroundContacts>;
  using raw_ptr = GroundContacts *;

  void init(std::vector<Particle::raw_ptr> *particles);

  unsigned addContact(ParticleContact::raw_ptr contact,
                      unsigned limit) const override;

private:
  std::vector<Particle::raw_ptr> *_particles;
};

}; // namespace ft

#endif // !FT_PWORLD_H
