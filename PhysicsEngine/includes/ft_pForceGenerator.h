#ifndef FT_FORCE_GENERATOR
#define FT_FORCE_GENERATOR

#include "ft_def.h"
#include "ft_particle.h"

namespace ft {

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
  void updateForces(real_t duration);

private:
  std::vector<std::pair<Particle::raw_ptr, ParticleForceGenerator::raw_ptr>>
      _registry;
};

class ParticleGravity : public ParticleForceGenerator {
public:
  using raw_ptr = ParticleGravity *;
  using pointer = std::shared_ptr<ParticleGravity>;

  ParticleGravity(const glm::vec3 &gravity);
  void updateForce(Particle::raw_ptr p, const real_t duration) override;

private:
  glm::vec3 _gravity;
};

class ParticleDrag : public ParticleForceGenerator {

public:
  using raw_ptr = ParticleDrag *;
  using pointer = std::shared_ptr<ParticleDrag>;

  ParticleDrag(const real_t k1, const real_t k2);

  void updateForce(Particle::raw_ptr p, const real_t duration) override;

private:
  real_t _k1, _k2;
};

class ParticleSpring : public ParticleForceGenerator {
public:
  using pointer = std::shared_ptr<ParticleSpring>;
  using raw_ptr = ParticleSpring *;

  ParticleSpring(const Particle::pointer &other, real_t springConst,
                 real_t restLength);

  void updateForce(Particle::raw_ptr p, const real_t duration);

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
                         real_t restLength);

  void updateForce(Particle::raw_ptr p, const real_t duration) override;

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
                 real_t restLenght);

  void updateForce(Particle::raw_ptr p, const real_t duration) override;

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
                   real_t liquidDensity = 1000.0f);

  void updateForce(Particle::raw_ptr p, const real_t duration) override;

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
                     real_t damping);

  void updateForce(Particle::raw_ptr p, real_t duration) override;

private:
  glm::vec3 _anchor;
  real_t _springConst;
  real_t _damping;
};

} // namespace ft

#endif // !FT_FORCE_GENERATOR
