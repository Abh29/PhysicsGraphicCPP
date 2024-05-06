#include "../includes/ft_pworld.h"

ft::ParticleWorld::ParticleWorld(unsigned maxContacts, unsigned iterations)
    : _resolver(iterations), _maxContacts(maxContacts) {

  _contacts.resize(_maxContacts);
  _calculateIterations = (iterations == 0);
}

void ft::ParticleWorld::startFrame() {

  for (auto &p : _particles)
    p->clearAccumulator();
}

unsigned ft::ParticleWorld::generateContacts() {
  int limit = _maxContacts;
  int nextContact = 0;

  for (auto &g : _contactGenerators) {
    unsigned used = g->addContact(&_contacts[nextContact], limit);
    limit -= used;
    nextContact += used;

    if (limit <= 0)
      break;
  }

  return _maxContacts - limit;
}

void ft::ParticleWorld::integrate(real_t duration) {

  for (auto &p : _particles) {
    p->integrate(duration);
  }
}

void ft::ParticleWorld::runPhysics(real_t duration) {

  _registry.updateForces(duration);

  integrate(duration);

  unsigned usedContacts = generateContacts();

  if (usedContacts) {
    if (_calculateIterations)
      _resolver.setIterations(usedContacts * 2);
    _resolver.resolveContacts(_contacts.data(), usedContacts, duration);
  }
}

std::vector<ft::Particle::raw_ptr> &ft::ParticleWorld::getParticles() {
  return _particles;
}

std::vector<ft::ParticleContactGenerator::raw_ptr> &
ft::ParticleWorld::getContactGenerators() {
  return _contactGenerators;
}

ft::ParticleForceRegistry &ft::ParticleWorld::getForceRegistry() {
  return _registry;
}

void ft::GroundContacts::init(std::vector<ft::Particle::raw_ptr> *particles) {
  GroundContacts::_particles = particles;
}

unsigned ft::GroundContacts::addContact(ft::ParticleContact *contact,
                                        unsigned limit) const {
  unsigned count = 0;

  for (auto &p : *_particles) {
    real_t y = p->getPosition().y;

    if (y < 0.0f) {
      contact->setContactNormal({0.0f, 1.0f, 0.0f});
      contact->setParticles(p, nullptr);
      contact->setPenetration(-y);
      contact->setRestitution(0.2f);
      contact++;
      count++;
    }
    if (count >= limit)
      break;
  }

  return count;
}
