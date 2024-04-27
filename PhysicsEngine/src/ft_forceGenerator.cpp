#include "../includes/ft_forceGenerator.h"
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>

glm::mat3 _linearInterpolate(const glm::mat3 &m1, const glm::mat3 &m2,
                             real_t a) {
  return (1.0f - a) * m1 + a * m2;
}

void ft::ForceRegistry::updateForces(real_t duration) {
  Registry::iterator i = registrations.begin();
  for (; i != registrations.end(); i++) {
    i->fg->updateForce(i->body, duration);
  }
}

void ft::ForceRegistry::add(RigidBody *body, ForceGenerator *fg) {
  ft::ForceRegistry::ForceRegistration registration;
  registration.body = body;
  registration.fg = fg;
  registrations.push_back(registration);
}

ft::Buoyancy::Buoyancy(const glm::vec3 &cOfB, real_t maxDepth, real_t volume,
                       real_t waterHeight,
                       real_t liquidDensity /* = 1000.0f */) {
  centreOfBuoyancy = cOfB;
  ft::Buoyancy::liquidDensity = liquidDensity;
  ft::Buoyancy::maxDepth = maxDepth;
  ft::Buoyancy::volume = volume;
  ft::Buoyancy::waterHeight = waterHeight;
}

void ft::Buoyancy::updateForce(RigidBody *body, real_t duration) {
  (void)duration;
  // Calculate the submersion depth
  glm::vec3 pointInWorld = body->getPointInWorldSpace(centreOfBuoyancy);
  real_t depth = pointInWorld.y;

  // Check if we're out of the water
  if (depth >= waterHeight + maxDepth)
    return;
  glm::vec3 force(0, 0, 0);

  // Check if we're at maximum depth
  if (depth <= waterHeight - maxDepth) {
    force.y = liquidDensity * volume;
    body->addForceAtBodyPoint(force, centreOfBuoyancy);
    return;
  }

  // Otherwise we are partly submerged
  force.y =
      liquidDensity * volume * (depth - maxDepth - waterHeight) / 2 * maxDepth;
  body->addForceAtBodyPoint(force, centreOfBuoyancy);
}

ft::Gravity::Gravity(const glm::vec3 &gravity) : gravity(gravity) {}

void ft::Gravity::updateForce(RigidBody *body, real_t duration) {
  (void)duration;
  // Check that we do not have infinite mass
  if (!body->hasFiniteMass())
    return;

  // Apply the mass-scaled force to the body
  body->addForce(gravity * body->getMass());
}

ft::Spring::Spring(const glm::vec3 &localConnectionPt, RigidBody *other,
                   const glm::vec3 &otherConnectionPt, real_t springConstant,
                   real_t restLength)
    : connectionPoint(localConnectionPt),
      otherConnectionPoint(otherConnectionPt), other(other),
      springConstant(springConstant), restLength(restLength) {}

void ft::Spring::updateForce(RigidBody *body, real_t duration) {
  (void)duration;
  // Calculate the two ends in world space
  glm::vec3 lws = body->getPointInWorldSpace(connectionPoint);
  glm::vec3 ows = other->getPointInWorldSpace(otherConnectionPoint);

  // Calculate the vector of the spring
  glm::vec3 force = lws - ows;

  // Calculate the magnitude of the force
  real_t magnitude = glm::length(force);
  magnitude = std::abs(magnitude - restLength);
  magnitude *= springConstant;

  // Calculate the final force and apply it
  force = glm::normalize(force);
  force *= -magnitude;
  body->addForceAtPoint(force, lws);
}

ft::Aero::Aero(const glm::mat3 &tensor, const glm::vec3 &position,
               const glm::vec3 *windspeed) {
  ft::Aero::tensor = tensor;
  ft::Aero::position = position;
  ft::Aero::windspeed = windspeed;
}

void ft::Aero::updateForce(RigidBody *body, real_t duration) {
  ft::Aero::updateForceFromTensor(body, duration, tensor);
}

void ft::Aero::updateForceFromTensor(RigidBody *body, real_t duration,
                                     const glm::mat3 &tensor) {
  (void)duration;
  // Calculate total velocity (windspeed and body's velocity).
  glm::vec3 velocity = body->getVelocity();
  velocity += *windspeed;

  // Calculate the velocity in body coordinates
  glm::vec3 bodyVel = body->getTransform() * glm::vec4(velocity, 0.0f);

  // Calculate the force in body coordinates
  glm::vec3 bodyForce = tensor * bodyVel;

  glm::vec3 force = body->getTransform() * glm::vec4(bodyForce, 0.0f);

  // Apply the force
  body->addForceAtBodyPoint(force, position);
}

ft::AeroControl::AeroControl(const glm::mat3 &base, const glm::mat3 &min,
                             const glm::mat3 &max, const glm::vec3 &position,
                             const glm::vec3 *windspeed)
    : Aero(base, position, windspeed) {
  ft::AeroControl::minTensor = min;
  ft::AeroControl::maxTensor = max;
  controlSetting = 0.0f;
}

glm::mat3 ft::AeroControl::getTensor() {
  if (controlSetting <= -1.0f)
    return minTensor;
  else if (controlSetting >= 1.0f)
    return maxTensor;
  else if (controlSetting < 0) {
    return _linearInterpolate(minTensor, tensor, controlSetting + 1.0f);
  } else if (controlSetting > 0) {
    return _linearInterpolate(tensor, maxTensor, controlSetting);
  } else
    return tensor;
}

void ft::AeroControl::setControl(real_t value) { controlSetting = value; }

void ft::AeroControl::updateForce(RigidBody *body, real_t duration) {
  glm::mat3 tensor = getTensor();
  ft::Aero::updateForceFromTensor(body, duration, tensor);
}

void ft::Explosion::updateForce(RigidBody *body, real_t duration) {
  (void)body;
  (void)duration;
}
