#include "../includes/ft_contacts.h"
#include <assert.h>
#include <cstdlib>
#include <glm/common.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/matrix.hpp>
#include <limits>
#include <memory.h>

static glm::mat3 _skewMatrix(const glm::vec3 &v) {
  return glm::mat3(0.0f, -v.z, v.y, v.z, 0.0f, -v.x, -v.y, v.x, 0.0f);
}

static inline void _quanterionAddVector(glm::quat &q, const glm::vec3 &v,
                                        const float duration) {
  float angle = glm::length(v) * duration;

  if (angle < std::numeric_limits<real_t>::epsilon() ||
      glm::length2(v) < std::numeric_limits<real_t>::epsilon())
    return;

  glm::vec3 axis = glm::normalize(v);

  glm::quat rotation = glm::angleAxis(angle, axis);

  q = glm::normalize(rotation * q);
}

void ft::Contact::setBodyData(RigidBody *one, RigidBody *two, real_t friction,
                              real_t restitution) {
  ft::Contact::_body[0] = one;
  ft::Contact::_body[1] = two;
  ft::Contact::_friction = friction;
  ft::Contact::_restitution = restitution;
}

void ft::Contact::matchAwakeState() {
  if (!_body[1])
    return;

  bool body0awake = _body[0]->getAwake();
  bool body1awake = _body[1]->getAwake();

  if (body0awake ^ body1awake) {
    if (body0awake)
      _body[1]->setAwake();
    else
      _body[0]->setAwake();
  }
}

void ft::Contact::swapBodies() {
  _contactNormal *= -1;

  RigidBody *temp = _body[0];
  _body[0] = _body[1];
  _body[1] = temp;
}

inline void ft::Contact::calculateContactBasis() {
  glm::vec3 contactTangent[2];

  if (std::abs(_contactNormal.x) > std::abs(_contactNormal.y)) {
    const real_t s =
        (real_t)1.0f / std::sqrt(_contactNormal.z * _contactNormal.z +
                                 _contactNormal.x * _contactNormal.x);

    contactTangent[0].x = _contactNormal.z * s;
    contactTangent[0].y = 0;
    contactTangent[0].z = -_contactNormal.x * s;

    contactTangent[1].x = _contactNormal.y * contactTangent[0].x;
    contactTangent[1].y = _contactNormal.z * contactTangent[0].x -
                          _contactNormal.x * contactTangent[0].z;
    contactTangent[1].z = -_contactNormal.y * contactTangent[0].x;
  } else {
    const real_t s =
        (real_t)1.0 / std::sqrt(_contactNormal.z * _contactNormal.z +
                                _contactNormal.y * _contactNormal.y);

    contactTangent[0].x = 0;
    contactTangent[0].y = -_contactNormal.z * s;
    contactTangent[0].z = _contactNormal.y * s;

    contactTangent[1].x = _contactNormal.y * contactTangent[0].z -
                          _contactNormal.z * contactTangent[0].y;
    contactTangent[1].y = -_contactNormal.x * contactTangent[0].z;
    contactTangent[1].z = _contactNormal.x * contactTangent[0].y;
  }

  _contactToWorld = {_contactNormal, contactTangent[0], contactTangent[1]};
}

glm::vec3 ft::Contact::calculateLocalVelocity(unsigned bodyIndex,
                                              real_t duration) {
  RigidBody::raw_ptr thisBody = _body[bodyIndex];

  glm::vec3 velocity =
      glm::cross(thisBody->getRotation(), _relativeContactPosition[bodyIndex]);

  velocity += thisBody->getVelocity();
  auto m = glm::transpose(_contactToWorld);
  glm::vec3 contactVelocity = m * velocity;

  glm::vec3 accVelocity = thisBody->getLastFrameAcceleration() * duration;

  accVelocity = m * accVelocity;

  accVelocity.x = 0;

  contactVelocity += accVelocity;

  return contactVelocity;
}

void ft::Contact::calculateDesiredDeltaVelocity(real_t duration) {
  const static real_t velocityLimit = 0.25f;

  real_t velocityFromAcc = 0;

  if (_body[0]->getAwake()) {
    velocityFromAcc += glm::dot(_body[0]->getLastFrameAcceleration() * duration,
                                _contactNormal);
  }

  if (_body[1] && _body[1]->getAwake()) {
    velocityFromAcc -= glm::dot(_body[1]->getLastFrameAcceleration() * duration,
                                _contactNormal);
  }

  real_t thisRestitution = _restitution;
  if (std::abs(_contactVelocity.x) < velocityLimit) {
    thisRestitution = (real_t)0.0f;
  }

  _desiredDeltaVelocity =
      -_contactVelocity.x -
      thisRestitution * (_contactVelocity.x - velocityFromAcc);
}

void ft::Contact::calculateInternals(real_t duration) {
  if (!_body[0])
    swapBodies();
  assert(_body[0]);

  calculateContactBasis();

  _relativeContactPosition[0] = _contactPoint - _body[0]->getPosition();
  if (_body[1]) {
    _relativeContactPosition[1] = _contactPoint - _body[1]->getPosition();
  }

  _contactVelocity = calculateLocalVelocity(0, duration);
  if (_body[1]) {
    _contactVelocity -= calculateLocalVelocity(1, duration);
  }

  calculateDesiredDeltaVelocity(duration);
}

void ft::Contact::applyVelocityChange(glm::vec3 velocityChange[2],
                                      glm::vec3 rotationChange[2]) {
  glm::mat3 inverseInertiaTensor[2];
  _body[0]->getInverseInertiaTensorWorld(&inverseInertiaTensor[0]);
  if (_body[1])
    _body[1]->getInverseInertiaTensorWorld(&inverseInertiaTensor[1]);

  glm::vec3 impulseContact;

  if (_friction == (real_t)0.0) {
    impulseContact = calculateFrictionlessImpulse(inverseInertiaTensor);
  } else {
    impulseContact = calculateFrictionImpulse(inverseInertiaTensor);
  }

  glm::vec3 impulse = _contactToWorld * impulseContact;

  glm::vec3 impulsiveTorque = glm::cross(_relativeContactPosition[0], impulse);
  rotationChange[0] = inverseInertiaTensor[0] * impulsiveTorque;
  velocityChange[0] = _body[0]->getInverseMass() * impulse;

  _body[0]->addVelocity(velocityChange[0]);
  _body[0]->addRotation(rotationChange[0]);

  if (_body[1]) {
    glm::vec3 impulsiveTorque =
        glm::cross(impulse, _relativeContactPosition[1]);
    rotationChange[1] = inverseInertiaTensor[1] * impulsiveTorque;
    velocityChange[1] = -_body[1]->getInverseMass() * impulse;

    _body[1]->addVelocity(velocityChange[1]);
    _body[1]->addRotation(rotationChange[1]);
  }
}

inline glm::vec3
ft::Contact::calculateFrictionlessImpulse(glm::mat3 *inverseInertiaTensor) {
  glm::vec3 impulseContact;

  glm::vec3 deltaVelWorld =
      glm::cross(_relativeContactPosition[0], _contactNormal);
  deltaVelWorld = inverseInertiaTensor[0] * deltaVelWorld;
  deltaVelWorld = glm::cross(deltaVelWorld, _relativeContactPosition[0]);

  real_t deltaVelocity = glm::dot(deltaVelWorld, _contactNormal);

  deltaVelocity += _body[0]->getInverseMass();

  if (_body[1]) {
    glm::vec3 deltaVelWorld =
        glm::cross(_relativeContactPosition[1], _contactNormal);
    deltaVelWorld = inverseInertiaTensor[1] * deltaVelWorld;
    deltaVelWorld = glm::cross(deltaVelWorld, _relativeContactPosition[1]);

    deltaVelocity += glm::dot(deltaVelWorld, _contactNormal);

    deltaVelocity += _body[1]->getInverseMass();
  }

  impulseContact.x = _desiredDeltaVelocity / deltaVelocity;
  impulseContact.y = 0;
  impulseContact.z = 0;
  return impulseContact;
}

inline glm::vec3
ft::Contact::calculateFrictionImpulse(glm::mat3 *inverseInertiaTensor) {
  glm::vec3 impulseContact;
  real_t inverseMass = _body[0]->getInverseMass();

  glm::mat3 impulseToTorque = _skewMatrix(_relativeContactPosition[0]);

  glm::mat3 deltaVelWorld = impulseToTorque;
  deltaVelWorld *= inverseInertiaTensor[0];
  deltaVelWorld *= impulseToTorque;
  deltaVelWorld *= -1;

  if (_body[1]) {
    impulseToTorque = _skewMatrix(_relativeContactPosition[1]);

    glm::mat3 deltaVelWorld2 = impulseToTorque;
    deltaVelWorld2 *= inverseInertiaTensor[1];
    deltaVelWorld2 *= impulseToTorque;
    deltaVelWorld2 *= -1;

    deltaVelWorld += deltaVelWorld2;

    inverseMass += _body[1]->getInverseMass();
  }

  glm::mat3 deltaVelocity = glm::transpose(_contactToWorld);
  deltaVelocity *= deltaVelWorld;
  deltaVelocity *= _contactToWorld;

  deltaVelocity[0][0] += inverseMass;
  deltaVelocity[1][1] += inverseMass;
  deltaVelocity[2][2] += inverseMass;

  glm::mat3 impulseMatrix = glm::inverse(deltaVelocity);

  glm::vec3 velKill(_desiredDeltaVelocity, -_contactVelocity.y,
                    -_contactVelocity.z);

  impulseContact = impulseMatrix * velKill;

  real_t planarImpulse = std::sqrt(impulseContact.y * impulseContact.y +
                                   impulseContact.z * impulseContact.z);
  if (planarImpulse > impulseContact.x * _friction) {
    impulseContact.y /= planarImpulse;
    impulseContact.z /= planarImpulse;

    impulseContact.x = deltaVelocity[0][0] +
                       deltaVelocity[1][0] * _friction * impulseContact.y +
                       deltaVelocity[2][0] * _friction * impulseContact.z;
    impulseContact.x = _desiredDeltaVelocity / impulseContact.x;
    impulseContact.y *= _friction * impulseContact.x;
    impulseContact.z *= _friction * impulseContact.x;
  }
  return impulseContact;
}

void ft::Contact::applyPositionChange(glm::vec3 linearChange[2],
                                      glm::vec3 angularChange[2],
                                      real_t penetration) {
  const real_t angularLimit = (real_t)0.2f;
  real_t angularMove[2];
  real_t linearMove[2];

  real_t totalInertia = 0;
  real_t linearInertia[2];
  real_t angularInertia[2];

  for (unsigned i = 0; i < 2; i++)
    if (_body[i]) {
      glm::mat3 inverseInertiaTensor;
      _body[i]->getInverseInertiaTensorWorld(&inverseInertiaTensor);

      glm::vec3 angularInertiaWorld =
          glm::cross(_relativeContactPosition[i], _contactNormal);
      angularInertiaWorld = inverseInertiaTensor * (angularInertiaWorld);
      angularInertiaWorld =
          glm::cross(angularInertiaWorld, _relativeContactPosition[i]);
      angularInertia[i] = glm::dot(angularInertiaWorld, _contactNormal);

      linearInertia[i] = _body[i]->getInverseMass();

      totalInertia += linearInertia[i] + angularInertia[i];
    }

  for (unsigned i = 0; i < 2; i++)
    if (_body[i]) {
      real_t sign = (i == 0) ? 1 : -1;
      angularMove[i] = sign * penetration * (angularInertia[i] / totalInertia);
      linearMove[i] = sign * penetration * (linearInertia[i] / totalInertia);

      glm::vec3 projection =
          _relativeContactPosition[i] -
          glm::dot(_relativeContactPosition[i], _contactNormal) *
              _contactNormal;

      real_t maxMagnitude = angularLimit * glm::length(projection);

      if (angularMove[i] < -maxMagnitude) {
        real_t totalMove = angularMove[i] + linearMove[i];
        angularMove[i] = -maxMagnitude;
        linearMove[i] = totalMove - angularMove[i];
      } else if (angularMove[i] > maxMagnitude) {
        real_t totalMove = angularMove[i] + linearMove[i];
        angularMove[i] = maxMagnitude;
        linearMove[i] = totalMove - angularMove[i];
      }

      if (angularMove[i] == 0) {
        angularChange[i] = {};
      } else {
        glm::vec3 targetAngularDirection =
            glm::cross(_relativeContactPosition[i], _contactNormal);

        glm::mat3 inverseInertiaTensor;
        _body[i]->getInverseInertiaTensorWorld(&inverseInertiaTensor);

        angularChange[i] = inverseInertiaTensor * targetAngularDirection *
                           (angularMove[i] / angularInertia[i]);
      }

      linearChange[i] = _contactNormal * linearMove[i];

      glm::vec3 pos = _body[i]->getPosition() + linearMove[i] * _contactNormal;
      _body[i]->setPosition(pos);

      glm::quat q = _body[i]->getOrientation();
      _quanterionAddVector(q, angularChange[i], 1.0f);
      _body[i]->setOrientation(q);

      if (!_body[i]->getAwake())
        _body[i]->calculateDerivedData();
    }
}

ft::ContactResolver::ContactResolver(unsigned iterations,
                                     real_t velocityEpsilon,
                                     real_t positionEpsilon) {
  setIterations(iterations, iterations);
  setEpsilon(velocityEpsilon, positionEpsilon);
}

ft::ContactResolver::ContactResolver(unsigned velocityIterations,
                                     unsigned positionIterations,
                                     real_t velocityEpsilon,
                                     real_t positionEpsilon) {
  (void)positionIterations;
  setIterations(velocityIterations);
  setEpsilon(velocityEpsilon, positionEpsilon);
}

void ft::ContactResolver::setIterations(unsigned iterations) {
  setIterations(iterations, iterations);
}

void ft::ContactResolver::setIterations(unsigned velocityIterations,
                                        unsigned positionIterations) {
  ft::ContactResolver::_velocityIterations = velocityIterations;
  ft::ContactResolver::_positionIterations = positionIterations;
}

void ft::ContactResolver::setEpsilon(real_t velocityEpsilon,
                                     real_t positionEpsilon) {
  ft::ContactResolver::_velocityEpsilon = velocityEpsilon;
  ft::ContactResolver::_positionEpsilon = positionEpsilon;
}

void ft::ContactResolver::resolveContacts(Contact *contacts,
                                          unsigned numContacts,
                                          real_t duration) {
  if (numContacts == 0)
    return;
  if (!isValid())
    return;

  prepareContacts(contacts, numContacts, duration);

  adjustPositions(contacts, numContacts, duration);

  adjustVelocities(contacts, numContacts, duration);
}

void ft::ContactResolver::prepareContacts(Contact *contacts,
                                          unsigned numContacts,
                                          real_t duration) {
  Contact *lastContact = contacts + numContacts;
  for (Contact *contact = contacts; contact < lastContact; contact++) {
    contact->calculateInternals(duration);
  }
}

void ft::ContactResolver::adjustVelocities(Contact *c, unsigned numContacts,
                                           real_t duration) {
  glm::vec3 velocityChange[2] = {}, rotationChange[2] = {};
  glm::vec3 deltaVel = {};

  _velocityIterationsUsed = 0;
  while (_velocityIterationsUsed < _velocityIterations) {
    real_t max = _velocityEpsilon;
    unsigned index = numContacts;
    for (unsigned i = 0; i < numContacts; i++) {
      if (c[i]._desiredDeltaVelocity > max) {
        max = c[i]._desiredDeltaVelocity;
        index = i;
      }
    }

    if (index >= numContacts)
      break;

    c[index].matchAwakeState();

    c[index].applyVelocityChange(velocityChange, rotationChange);

    for (unsigned i = 0; i < numContacts; i++) {
      glm::mat3 m = glm::transpose(c[i]._contactToWorld);
      for (unsigned b = 0; b < 2; b++)
        if (c[i]._body[b]) {
          for (unsigned d = 0; d < 2; d++) {
            if (c[i]._body[b] == c[index]._body[d]) {

              deltaVel = velocityChange[d] +
                         glm::cross(rotationChange[d],
                                    c[i]._relativeContactPosition[b]);

              c[i]._contactVelocity += (b ? -1.0f : 1.0f) * (m * deltaVel);

              c[i].calculateDesiredDeltaVelocity(duration);
            }
          }
        }
    }
    _velocityIterationsUsed++;
  }
}

void ft::ContactResolver::adjustPositions(Contact::raw_ptr c,
                                          unsigned numContacts,
                                          real_t duration) {
  (void)duration;
  unsigned i, index;
  glm::vec3 linearChange[2] = {}, angularChange[2] = {};
  real_t max;
  glm::vec3 deltaPosition = {};

  _positionIterationsUsed = 0;
  while (_positionIterationsUsed < _positionIterations) {
    max = _positionEpsilon;
    index = numContacts;
    for (i = 0; i < numContacts; i++) {
      if (c[i]._penetration > max) {
        max = c[i]._penetration;
        index = i;
      }
    }

    if (index == numContacts)
      break;

    c[index].matchAwakeState();

    c[index].applyPositionChange(linearChange, angularChange, max);

    for (i = 0; i < numContacts; i++) {
      for (unsigned b = 0; b < 2; b++)
        if (c[i]._body[b]) {
          for (unsigned d = 0; d < 2; d++) {
            if (c[i]._body[b] == c[index]._body[d]) {
              deltaPosition = linearChange[d] +
                              glm::cross(angularChange[d],
                                         c[i]._relativeContactPosition[b]);

              c[i]._penetration += (real_t)(b ? 1.0f : -1.0f) *
                                   glm::dot(deltaPosition, c[i]._contactNormal);
            }
          }
        }
    }
    _positionIterationsUsed++;
  }
}
