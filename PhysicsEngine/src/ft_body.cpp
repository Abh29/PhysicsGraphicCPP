#include "../includes/ft_body.h"
#include <cmath>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/matrix.hpp>
#include <limits>

static inline void _checkInverseInertiaTensor(const glm::mat3 &iitWorld) {
  (void)iitWorld;
  // TODO: Perform a validity check in an assert.
}

static inline void _quanterionAddVector(glm::quat &q, const glm::vec3 &v,
                                        const float duration) {
  // Convert angular velocity to a rotation quaternion
  float angle = glm::length(v) * duration; // Total rotation angle in radians
  glm::vec3 axis = glm::normalize(v);      // Axis of rotation

  glm::quat rotation =
      glm::angleAxis(angle, axis); // Quaternion representing the rotation

  // Update the original orientation by applying the rotation
  q = glm::normalize(rotation * q); // Apply rotation and normalize
}

/**
 * Internal function to do an intertia tensor transform by a quaternion.
 * Note that the implementation of this function was created by an
 * automated code-generator and optimizer.
 */

static inline void _transformInertiaTensor(glm::mat3 &iitWorld,
                                           const glm::quat &q,
                                           const glm::mat3 &iitBody,
                                           const glm::mat4 &rotmat) {

  (void)q;
  glm::vec3 col0(rotmat[0][0], rotmat[1][0], rotmat[2][0]);
  glm::vec3 col1(rotmat[0][1], rotmat[1][1], rotmat[2][1]);
  glm::vec3 col2(rotmat[0][2], rotmat[1][2], rotmat[2][2]);

  glm::vec3 row0(iitBody[0][0], iitBody[0][1], iitBody[0][2]);
  glm::vec3 row1(iitBody[1][0], iitBody[1][1], iitBody[1][2]);
  glm::vec3 row2(iitBody[2][0], iitBody[2][1], iitBody[2][2]);

  glm::vec3 t4 = glm::vec3(glm::dot(col0, row0), glm::dot(col0, row1),
                           glm::dot(col0, row2));

  glm::vec3 t9 = glm::vec3(glm::dot(col1, row0), glm::dot(col1, row1),
                           glm::dot(col1, row2));

  glm::vec3 t14 = glm::vec3(glm::dot(col2, row0), glm::dot(col2, row1),
                            glm::dot(col2, row2));

  // Update the world inertia tensor with transformed data
  iitWorld[0] = t4.x * col0 + t9.x * col1 + t14.x * col2;
  iitWorld[1] = t4.y * col0 + t9.y * col1 + t14.y * col2;
  iitWorld[2] = t4.z * col0 + t9.z * col1 + t14.z * col2;
}

// /**
//  * Inline function that creates a transform matrix from a
//  * position and orientation.
//  */
static inline void _calculateTransformMatrix(glm::mat4 &transformMatrix,
                                             const glm::vec3 &position,
                                             const glm::quat &orientation) {
  (void)transformMatrix;
  (void)position;
  (void)orientation;

  transformMatrix =
      glm::translate(glm::mat4(1.0f), position) * glm::mat4_cast(orientation);
}

void ft::RigidBody::calculateDerivedData() {

  _orientation = glm::normalize(_orientation);

  // Calculate the transform matrix for the body.
  _calculateTransformMatrix(_transformMatrix, _position, _orientation);

  // Calculate the inertiaTensor in world space.
  _transformInertiaTensor(_inverseInertiaTensorWorld, _orientation,
                          _inverseInertiaTensor, _transformMatrix);
}

void ft::RigidBody::integrate(real_t duration) {
  if (!_isAwake)
    return;

  // Calculate linear acceleration from force inputs.
  _lastFrameAcceleration = _acceleration;
  _lastFrameAcceleration += (_inverseMass * _forceAccum);

  // Calculate angular acceleration from torque inputs.
  glm::vec3 angularAcceleration = _inverseInertiaTensorWorld * _torqueAccum;

  // Adjust velocities
  // Update linear velocity from both acceleration and impulse.
  _velocity += (duration * _lastFrameAcceleration);

  // Update angular velocity from both acceleration and impulse.
  _rotation += (duration * angularAcceleration);

  // Impose drag.
  _velocity *= std::pow(_linearDamping, duration);
  _rotation *= std::pow(_angularDamping, duration);

  // Adjust positions
  // Update linear position.
  _position += (duration * _velocity);

  // Update angular position.
  // _orientation.addScaledVector(_rotation, duration);
  _quanterionAddVector(_orientation, _rotation, duration);

  // Normalise the orientation, and update the matrices with the new
  // position and orientation
  calculateDerivedData();

  // Clear accumulators.
  clearAccumulators();

  // Update the kinetic energy store, and possibly put the body to
  // sleep.
  if (_canSleep) {
    real_t currentMotion =
        glm::dot(_velocity, _velocity) + glm::dot(_rotation, _rotation);

    real_t bias = std::pow(0.5f, duration);
    _motion = bias * _motion + (1 - bias) * currentMotion;

    if (_motion < ft::SLEEP_EPSILON)
      setAwake(false);
    else if (_motion > 10 * ft::SLEEP_EPSILON)
      _motion = 10 * ft::SLEEP_EPSILON;
  }
}

void ft::RigidBody::setMass(const real_t mass) {
  assert(mass != 0);
  RigidBody::_inverseMass = ((real_t)1.0) / mass;
}

real_t ft::RigidBody::getMass() const {
  if (_inverseMass == 0) {
    return std::numeric_limits<real_t>::max();
  } else {
    return ((real_t)1.0) / _inverseMass;
  }
}

void ft::RigidBody::setInverseMass(const real_t inverseMass) {
  RigidBody::_inverseMass = inverseMass;
}

real_t ft::RigidBody::getInverseMass() const { return _inverseMass; }

bool ft::RigidBody::hasFiniteMass() const { return _inverseMass >= 0.0f; }

void ft::RigidBody::setInertiaTensor(const glm::mat3 &inertiaTensor) {
  (void)inertiaTensor;

  _inverseInertiaTensor = glm::inverse(inertiaTensor);

  // TODO: implement this
  // _inverseInertiaTensor.setInverse(inertiaTensor);
  // _checkInverseInertiaTensor(_inverseInertiaTensor);
}

void ft::RigidBody::getInertiaTensor(glm::mat3 *inertiaTensor) const {
  (void)inertiaTensor;
  // TODO: implement this
  *inertiaTensor = glm::inverse(_inverseInertiaTensor);
  // inertiaTensor->setInverse(_inverseInertiaTensor);
}

glm::mat3 ft::RigidBody::getInertiaTensor() const {
  glm::mat3 it;
  getInertiaTensor(&it);
  return it;
}

void ft::RigidBody::getInertiaTensorWorld(glm::mat3 *inertiaTensor) const {

  *inertiaTensor = glm::inverse(_inverseInertiaTensorWorld);

  // TODO: implement this
  // inertiaTensor->setInverse(inverseInertiaTensorWorld);
}

glm::mat3 ft::RigidBody::getInertiaTensorWorld() const {
  return glm::inverse(_inverseInertiaTensorWorld);
}

void ft::RigidBody::setInverseInertiaTensor(
    const glm::mat3 &inverseInertiaTensor) {
  _checkInverseInertiaTensor(inverseInertiaTensor);
  RigidBody::_inverseInertiaTensor = inverseInertiaTensor;
}

void ft::RigidBody::getInverseInertiaTensor(
    glm::mat3 *inverseInertiaTensor) const {
  *inverseInertiaTensor = RigidBody::_inverseInertiaTensor;
}

glm::mat3 ft::RigidBody::getInverseInertiaTensor() const {
  return _inverseInertiaTensor;
}

void ft::RigidBody::getInverseInertiaTensorWorld(
    glm::mat3 *inverseInertiaTensor) const {
  *inverseInertiaTensor = _inverseInertiaTensorWorld;
}

glm::mat3 ft::RigidBody::getInverseInertiaTensorWorld() const {
  return _inverseInertiaTensorWorld;
}

void ft::RigidBody::setDamping(const real_t linearDamping,
                               const real_t angularDamping) {
  RigidBody::_linearDamping = linearDamping;
  RigidBody::_angularDamping = angularDamping;
}

void ft::RigidBody::setLinearDamping(const real_t linearDamping) {
  RigidBody::_linearDamping = linearDamping;
}

real_t ft::RigidBody::getLinearDamping() const { return _linearDamping; }

void ft::RigidBody::setAngularDamping(const real_t angularDamping) {
  RigidBody::_angularDamping = angularDamping;
}

real_t ft::RigidBody::getAngularDamping() const { return _angularDamping; }

void ft::RigidBody::setPosition(const glm::vec3 &position) {
  RigidBody::_position = position;
}

void ft::RigidBody::setPosition(const real_t x, const real_t y,
                                const real_t z) {
  _position.x = x;
  _position.y = y;
  _position.z = z;
}

void ft::RigidBody::getPosition(glm::vec3 *position) const {
  *position = RigidBody::_position;
}

glm::vec3 ft::RigidBody::getPosition() const { return _position; }

void ft::RigidBody::setOrientation(const glm::quat &orientation) {
  RigidBody::_orientation = glm::normalize(orientation);
}

void ft::RigidBody::setOrientation(const real_t r, const real_t i,
                                   const real_t j, const real_t k) {
  _orientation.w = r;
  _orientation.x = i;
  _orientation.y = j;
  _orientation.z = k;
  _orientation = glm::normalize(_orientation);
}

void ft::RigidBody::getOrientation(glm::quat *orientation) const {
  *orientation = RigidBody::_orientation;
}

glm::quat ft::RigidBody::getOrientation() const { return _orientation; }

void ft::RigidBody::getOrientation(glm::mat3 *matrix) const {
  *matrix = _transformMatrix;
}

void ft::RigidBody::getTransform(glm::mat4 *transform) const {
  *transform = _transformMatrix;
}

glm::mat4 ft::RigidBody::getTransform() const { return _transformMatrix; }

glm::vec3 ft::RigidBody::getPointInLocalSpace(const glm::vec3 &point) const {
  (void)point;
  return glm::vec3(glm::inverse(_transformMatrix) * glm::vec4(point, 1.0f));
  // return transformMatrix.transformInverse(point);
}

glm::vec3 ft::RigidBody::getPointInWorldSpace(const glm::vec3 &point) const {
  return glm::vec3(_transformMatrix * glm::vec4(point, 1.0f));
}

glm::vec3
ft::RigidBody::getDirectionInLocalSpace(const glm::vec3 &direction) const {
  (void)direction;
  return glm::vec3(glm::inverse(_transformMatrix) * glm::vec4(direction, 0.0f));
  // return transformMatrix.transformInverseDirection(direction);
}

glm::vec3
ft::RigidBody::getDirectionInWorldSpace(const glm::vec3 &direction) const {
  return glm::vec3(glm::vec4(direction, 1.0f) * _transformMatrix);
}

void ft::RigidBody::setVelocity(const glm::vec3 &velocity) {
  RigidBody::_velocity = velocity;
}

void ft::RigidBody::setVelocity(const real_t x, const real_t y,
                                const real_t z) {
  _velocity.x = x;
  _velocity.y = y;
  _velocity.z = z;
}

void ft::RigidBody::getVelocity(glm::vec3 *velocity) const {
  *velocity = RigidBody::_velocity;
}

glm::vec3 ft::RigidBody::getVelocity() const { return _velocity; }

void ft::RigidBody::addVelocity(const glm::vec3 &deltaVelocity) {
  _velocity += deltaVelocity;
}

void ft::RigidBody::setRotation(const glm::vec3 &rotation) {
  ft::RigidBody::_rotation = rotation;
}

void ft::RigidBody::setRotation(const real_t x, const real_t y,
                                const real_t z) {
  _rotation.x = x;
  _rotation.y = y;
  _rotation.z = z;
}

void ft::RigidBody::getRotation(glm::vec3 *rotation) const {
  *rotation = ft::RigidBody::_rotation;
}

glm::vec3 ft::RigidBody::getRotation() const { return _rotation; }

void ft::RigidBody::addRotation(const glm::vec3 &deltaRotation) {
  _rotation += deltaRotation;
}

void ft::RigidBody::setAwake(const bool awake) {
  if (awake) {
    _isAwake = true;

    // Add a bit of motion to avoid it falling asleep immediately.
    _motion = ft::SLEEP_EPSILON * 2.0f;
  } else {
    _isAwake = false;
    _velocity = {};
    _rotation = {};
  }
}

void ft::RigidBody::setCanSleep(const bool canSleep) {
  ft::RigidBody::_canSleep = canSleep;

  if (!canSleep && !_isAwake)
    setAwake();
}

void ft::RigidBody::getLastFrameAcceleration(glm::vec3 *acceleration) const {
  *acceleration = _lastFrameAcceleration;
}

glm::vec3 ft::RigidBody::getLastFrameAcceleration() const {
  return _lastFrameAcceleration;
}

void ft::RigidBody::clearAccumulators() {
  _forceAccum = {};
  _torqueAccum = {};
}

void ft::RigidBody::addForce(const glm::vec3 &force) {
  _forceAccum += force;
  _isAwake = true;
}

void ft::RigidBody::addForceAtBodyPoint(const glm::vec3 &force,
                                        const glm::vec3 &point) {
  // Convert to coordinates relative to center of mass.
  glm::vec3 pt = getPointInWorldSpace(point);
  addForceAtPoint(force, pt);
}

void ft::RigidBody::addForceAtPoint(const glm::vec3 &force,
                                    const glm::vec3 &point) {
  // Convert to coordinates relative to center of mass.
  glm::vec3 pt = point;
  pt -= _position;

  _forceAccum += force;
  _torqueAccum = glm::cross(pt, force);

  _isAwake = true;
}

void ft::RigidBody::addTorque(const glm::vec3 &torque) {
  _torqueAccum += torque;
  _isAwake = true;
}

void ft::RigidBody::setAcceleration(const glm::vec3 &acceleration) {
  ft::RigidBody::_acceleration = acceleration;
}

void ft::RigidBody::setAcceleration(const real_t x, const real_t y,
                                    const real_t z) {
  _acceleration.x = x;
  _acceleration.y = y;
  _acceleration.z = z;
}

void ft::RigidBody::getAcceleration(glm::vec3 *acceleration) const {
  *acceleration = ft::RigidBody::_acceleration;
}

glm::vec3 ft::RigidBody::getAcceleration() const { return _acceleration; }
