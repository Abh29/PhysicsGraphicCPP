#include "../includes/ft_contacts.h"
#include <assert.h>
#include <cstdlib>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/matrix.hpp>
#include <memory.h>

// todo! test this
static glm::mat3 _skewMatrix(const glm::vec3 &v) {
  return glm::mat3(0.0f, -v.z, v.y, v.z, 0.0f, -v.x, -v.y, v.x, 0.0f);
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

void ft::Contact::setBodyData(RigidBody *one, RigidBody *two, real_t friction,
                              real_t restitution) {
  ft::Contact::_body[0] = one;
  ft::Contact::_body[1] = two;
  ft::Contact::_friction = friction;
  ft::Contact::_restitution = restitution;
}

void ft::Contact::matchAwakeState() {
  // Collisions with the world never cause a body to wake up.
  if (!_body[1])
    return;

  bool body0awake = _body[0]->getAwake();
  bool body1awake = _body[1]->getAwake();

  // Wake up only the sleeping one
  if (body0awake ^ body1awake) {
    if (body0awake)
      _body[1]->setAwake();
    else
      _body[0]->setAwake();
  }
}

/*
 * Swaps the bodies in the current contact, so body 0 is at body 1 and
 * vice versa. This also changes the direction of the contact normal,
 * but doesn't update any calculated internal data. If you are calling
 * this method manually, then call calculateInternals afterwards to
 * make sure the internal data is up to date.
 */
void ft::Contact::swapBodies() {
  _contactNormal *= -1;

  RigidBody *temp = _body[0];
  _body[0] = _body[1];
  _body[1] = temp;
}

/*
 * Constructs an arbitrary orthonormal basis for the contact.  This is
 * stored as a 3x3 matrix, where each vector is a column (in other
 * words the matrix transforms contact space into world space). The x
 * direction is generated from the contact normal, and the y and z
 * directionss are set so they are at right angles to it.
 */
inline void ft::Contact::calculateContactBasis() {
  glm::vec3 contactTangent[2];

  // Check whether the Z-axis is nearer to the X or Y axis
  if (std::abs(_contactNormal.x) > std::abs(_contactNormal.y)) {
    // Scaling factor to ensure the results are normalised
    const real_t s =
        (real_t)1.0f / std::sqrt(_contactNormal.z * _contactNormal.z +
                                 _contactNormal.x * _contactNormal.x);

    // The new X-axis is at right angles to the world Y-axis
    contactTangent[0].x = _contactNormal.z * s;
    contactTangent[0].y = 0;
    contactTangent[0].z = -_contactNormal.x * s;

    // The new Y-axis is at right angles to the new X- and Z- axes
    contactTangent[1].x = _contactNormal.y * contactTangent[0].x;
    contactTangent[1].y = _contactNormal.z * contactTangent[0].x -
                          _contactNormal.x * contactTangent[0].z;
    contactTangent[1].z = -_contactNormal.y * contactTangent[0].x;
  } else {
    // Scaling factor to ensure the results are normalised
    const real_t s =
        (real_t)1.0 / std::sqrt(_contactNormal.z * _contactNormal.z +
                                _contactNormal.y * _contactNormal.y);

    // The new X-axis is at right angles to the world X-axis
    contactTangent[0].x = 0;
    contactTangent[0].y = -_contactNormal.z * s;
    contactTangent[0].z = _contactNormal.y * s;

    // The new Y-axis is at right angles to the new X- and Z- axes
    contactTangent[1].x = _contactNormal.y * contactTangent[0].z -
                          _contactNormal.z * contactTangent[0].y;
    contactTangent[1].y = -_contactNormal.x * contactTangent[0].z;
    contactTangent[1].z = _contactNormal.x * contactTangent[0].y;
  }

  // Make a matrix from the three vectors.
  // TODO: refactor this
  _contactToWorld = {_contactNormal, contactTangent[0], contactTangent[1]};
}

glm::vec3 ft::Contact::calculateLocalVelocity(unsigned bodyIndex,
                                              real_t duration) {
  RigidBody *thisBody = _body[bodyIndex];

  // Work out the velocity of the contact point.
  glm::vec3 velocity =
      glm::cross(thisBody->getRotation(), _relativeContactPosition[bodyIndex]);

  velocity += thisBody->getVelocity();

  // Turn the velocity into contact-coordinates.
  // TODO!
  // glm::vec3 contactVelocity = _contactToWorld.transformTranspose(velocity);

  // Calculate the ammount of velocity that is due to forces without
  // reactions.
  glm::vec3 accVelocity = thisBody->getLastFrameAcceleration() * duration;

  // Calculate the velocity in contact-coordinates.
  // todo!
  // accVelocity = _contactToWorld.transformTranspose(accVelocity);

  // We ignore any component of acceleration in the contact normal
  // direction, we are only interested in planar acceleration
  accVelocity.x = 0;

  // Add the planar velocities - if there's enough friction they will
  // be removed during velocity resolution
  _contactVelocity += accVelocity;

  // And return it
  return _contactVelocity;
}

void ft::Contact::calculateDesiredDeltaVelocity(real_t duration) {
  const static real_t velocityLimit = (real_t)0.25f;

  // Calculate the acceleration induced velocity accumulated this frame
  real_t velocityFromAcc = 0;

  if (_body[0]->getAwake()) {
    velocityFromAcc +=
        glm::dot(_body[0]->getLastFrameAcceleration(), _contactNormal) *
        duration;
  }

  if (_body[1] && _body[1]->getAwake()) {
    velocityFromAcc -=
        glm::dot(_body[1]->getLastFrameAcceleration(), _contactNormal) *
        duration;
  }

  // If the velocity is very slow, limit the restitution
  real_t thisRestitution = _restitution;
  if (std::abs(_contactVelocity.x) < velocityLimit) {
    thisRestitution = (real_t)0.0f;
  }

  // Combine the bounce velocity with the removed
  // acceleration velocity.
  _desiredDeltaVelocity =
      -_contactVelocity.x -
      thisRestitution * (_contactVelocity.x - velocityFromAcc);
}

void ft::Contact::calculateInternals(real_t duration) {
  // Check if the first object is NULL, and swap if it is.
  if (!_body[0])
    swapBodies();
  assert(_body[0]);

  // Calculate an set of axis at the contact point.
  calculateContactBasis();

  // Store the relative position of the contact relative to each body
  _relativeContactPosition[0] = _contactPoint - _body[0]->getPosition();
  if (_body[1]) {
    _relativeContactPosition[1] = _contactPoint - _body[1]->getPosition();
  }

  // Find the relative velocity of the bodies at the contact point.
  _contactVelocity = calculateLocalVelocity(0, duration);
  if (_body[1]) {
    _contactVelocity -= calculateLocalVelocity(1, duration);
  }

  // Calculate the desired change in velocity for resolution
  calculateDesiredDeltaVelocity(duration);
}

void ft::Contact::applyVelocityChange(glm::vec3 velocityChange[2],
                                      glm::vec3 rotationChange[2]) {
  // Get hold of the inverse mass and inverse inertia tensor, both in
  // world coordinates.
  glm::mat3 inverseInertiaTensor[2];
  _body[0]->getInverseInertiaTensorWorld(&inverseInertiaTensor[0]);
  if (_body[1])
    _body[1]->getInverseInertiaTensorWorld(&inverseInertiaTensor[1]);

  // We will calculate the impulse for each contact axis
  glm::vec3 impulseContact;

  if (_friction == (real_t)0.0) {
    // Use the short format for frictionless contacts
    impulseContact = calculateFrictionlessImpulse(inverseInertiaTensor);
  } else {
    // Otherwise we may have impulses that aren't in the direction of the
    // contact, so we need the more complex version.
    impulseContact = calculateFrictionImpulse(inverseInertiaTensor);
  }

  // Convert impulse to world coordinates
  glm::vec3 impulse = _contactToWorld * impulseContact;

  // Split in the impulse into linear and rotational components
  glm::vec3 impulsiveTorque = glm::cross(_relativeContactPosition[0], impulse);
  rotationChange[0] = inverseInertiaTensor[0] * impulsiveTorque;
  velocityChange[0] = _body[0]->getInverseMass() * impulse;

  // Apply the changes
  _body[0]->addVelocity(velocityChange[0]);
  _body[0]->addRotation(rotationChange[0]);

  if (_body[1]) {
    // Work out body one's linear and angular changes
    glm::vec3 impulsiveTorque =
        glm::cross(impulse, _relativeContactPosition[1]);
    rotationChange[1] = inverseInertiaTensor[1] * impulsiveTorque;
    velocityChange[1] = -_body[1]->getInverseMass() * impulse;

    // And apply them.
    _body[1]->addVelocity(velocityChange[1]);
    _body[1]->addRotation(rotationChange[1]);
  }
}

inline glm::vec3
ft::Contact::calculateFrictionlessImpulse(glm::mat3 *inverseInertiaTensor) {
  glm::vec3 impulseContact;

  // Build a vector that shows the change in velocity in
  // world space for a unit impulse in the direction of the contact
  // normal.
  glm::vec3 deltaVelWorld =
      glm::cross(_relativeContactPosition[0], _contactNormal);
  deltaVelWorld = inverseInertiaTensor[0] * deltaVelWorld;
  deltaVelWorld = glm::cross(deltaVelWorld, _relativeContactPosition[0]);

  // Work out the change in velocity in contact coordiantes.
  real_t deltaVelocity = glm::dot(deltaVelWorld, _contactNormal);

  // Add the linear component of velocity change
  deltaVelocity += _body[0]->getInverseMass();

  // Check if we need to the second body's data
  if (_body[1]) {
    // Go through the same transformation sequence again
    glm::vec3 deltaVelWorld =
        glm::cross(_relativeContactPosition[1], _contactNormal);
    deltaVelWorld = inverseInertiaTensor[1] * deltaVelWorld;
    deltaVelWorld = glm::cross(deltaVelWorld, _relativeContactPosition[1]);

    // Add the change in velocity due to rotation
    deltaVelocity += glm::dot(deltaVelWorld, _contactNormal);

    // Add the change in velocity due to linear motion
    deltaVelocity += _body[1]->getInverseMass();
  }

  // Calculate the required size of the impulse
  impulseContact.x = _desiredDeltaVelocity / deltaVelocity;
  impulseContact.y = 0;
  impulseContact.z = 0;
  return impulseContact;
}

inline glm::vec3
ft::Contact::calculateFrictionImpulse(glm::mat3 *inverseInertiaTensor) {
  glm::vec3 impulseContact;
  real_t inverseMass = _body[0]->getInverseMass();

  // The equivalent of a cross product in matrices is multiplication
  // by a skew symmetric matrix - we build the matrix for converting
  // between linear and angular quantities.
  // todo! implement
  glm::mat3 impulseToTorque = _skewMatrix(_relativeContactPosition[0]);

  // Build the matrix to convert contact impulse to change in velocity
  // in world coordinates.
  glm::mat3 deltaVelWorld = impulseToTorque;
  deltaVelWorld *= inverseInertiaTensor[0];
  deltaVelWorld *= impulseToTorque;
  deltaVelWorld *= -1;

  // Check if we need to add body two's data
  if (_body[1]) {
    // Set the cross product matrix
    impulseToTorque = _skewMatrix(_relativeContactPosition[1]);

    // Calculate the velocity change matrix
    glm::mat3 deltaVelWorld2 = impulseToTorque;
    deltaVelWorld2 *= inverseInertiaTensor[1];
    deltaVelWorld2 *= impulseToTorque;
    deltaVelWorld2 *= -1;

    // Add to the total delta velocity.
    deltaVelWorld += deltaVelWorld2;

    // Add to the inverse mass
    inverseMass += _body[1]->getInverseMass();
  }

  // Do a change of basis to convert into contact coordinates.
  glm::mat3 deltaVelocity = glm::transpose(_contactToWorld);
  deltaVelocity *= deltaVelWorld;
  deltaVelocity *= _contactToWorld;

  // Add in the linear velocity change
  deltaVelocity[0][0] += inverseMass;
  deltaVelocity[1][1] += inverseMass;
  deltaVelocity[2][2] += inverseMass;

  // Invert to get the impulse needed per unit velocity
  glm::mat3 impulseMatrix = glm::inverse(deltaVelocity);

  // Find the target velocities to kill
  glm::vec3 velKill(_desiredDeltaVelocity, -_contactVelocity.y,
                    -_contactVelocity.z);

  // Find the impulse to kill target velocities
  impulseContact = impulseMatrix * velKill;

  // Check for exceeding friction
  real_t planarImpulse = std::sqrt(impulseContact.y * impulseContact.y +
                                   impulseContact.z * impulseContact.z);
  if (planarImpulse > impulseContact.x * _friction) {
    // We need to use dynamic friction
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

  // We need to work out the inertia of each object in the direction
  // of the contact normal, due to angular inertia only.
  for (unsigned i = 0; i < 2; i++)
    if (_body[i]) {
      glm::mat3 inverseInertiaTensor;
      _body[i]->getInverseInertiaTensorWorld(&inverseInertiaTensor);

      // Use the same procedure as for calculating frictionless
      // velocity change to work out the angular inertia.
      glm::vec3 angularInertiaWorld =
          glm::cross(_relativeContactPosition[i], _contactNormal);
      angularInertiaWorld = inverseInertiaTensor * (angularInertiaWorld);
      angularInertiaWorld =
          glm::cross(angularInertiaWorld, _relativeContactPosition[i]);
      angularInertia[i] = glm::dot(angularInertiaWorld, _contactNormal);

      // The linear component is simply the inverse mass
      linearInertia[i] = _body[i]->getInverseMass();

      // Keep track of the total inertia from all components
      totalInertia += linearInertia[i] + angularInertia[i];

      // We break the loop here so that the totalInertia value is
      // completely calculated (by both iterations) before
      // continuing.
    }

  // Loop through again calculating and applying the changes
  for (unsigned i = 0; i < 2; i++)
    if (_body[i]) {
      // The linear and angular movements required are in proportion to
      // the two inverse inertias.
      real_t sign = (i == 0) ? 1 : -1;
      angularMove[i] = sign * penetration * (angularInertia[i] / totalInertia);
      linearMove[i] = sign * penetration * (linearInertia[i] / totalInertia);

      // To avoid angular projections that are too great (when mass is large
      // but inertia tensor is small) limit the angular move.
      glm::vec3 projection =
          _relativeContactPosition[i] -
          glm::dot(_relativeContactPosition[i], _contactNormal) *
              _contactNormal;

      // Use the small angle approximation for the sine of the angle (i.e.
      // the magnitude would be sine(angularLimit) * projection.magnitude
      // but we approximate sine(angularLimit) to angularLimit).
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

      // We have the linear amount of movement required by turning
      // the rigid body (in angularMove[i]). We now need to
      // calculate the desired rotation to achieve that.
      if (angularMove[i] == 0) {
        // Easy case - no angular movement means no rotation.
        angularChange[i] = {};
      } else {
        // Work out the direction we'd like to rotate in.
        glm::vec3 targetAngularDirection =
            glm::cross(_relativeContactPosition[i], _contactNormal);

        glm::mat3 inverseInertiaTensor;
        _body[i]->getInverseInertiaTensorWorld(&inverseInertiaTensor);

        // Work out the direction we'd need to rotate to achieve that
        angularChange[i] = inverseInertiaTensor * targetAngularDirection *
                           (angularMove[i] / angularInertia[i]);
      }

      // Velocity change is easier - it is just the linear movement
      // along the contact normal.
      linearChange[i] = _contactNormal * linearMove[i];

      // Now we can start to apply the values we've calculated.
      // Apply the linear movement
      glm::vec3 pos = _body[i]->getPosition() + linearMove[i] * _contactNormal;
      _body[i]->setPosition(pos);

      // And the change in orientation
      glm::quat q = _body[i]->getOrientation();
      _quanterionAddVector(q, angularChange[i], 1.0f);
      _body[i]->setOrientation(q);

      // We need to calculate the derived data for any body that is
      // asleep, so that the changes are reflected in the object's
      // data. Otherwise the resolution will not change the position
      // of the object, and the next collision detection round will
      // have the same penetration.
      if (!_body[i]->getAwake())
        _body[i]->calculateDerivedData();
    }
}

// Contact resolver implementation

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
  // Make sure we have something to do.
  if (numContacts == 0)
    return;
  if (!isValid())
    return;

  // Prepare the contacts for processing
  prepareContacts(contacts, numContacts, duration);

  // Resolve the interpenetration problems with the contacts.
  adjustPositions(contacts, numContacts, duration);

  // Resolve the velocity problems with the contacts.
  adjustVelocities(contacts, numContacts, duration);
}

void ft::ContactResolver::prepareContacts(Contact *contacts,
                                          unsigned numContacts,
                                          real_t duration) {
  // Generate contact velocity and axis information.
  Contact *lastContact = contacts + numContacts;
  for (Contact *contact = contacts; contact < lastContact; contact++) {
    // Calculate the internal contact data (inertia, basis, etc).
    contact->calculateInternals(duration);
  }
}

void ft::ContactResolver::adjustVelocities(Contact *c, unsigned numContacts,
                                           real_t duration) {
  glm::vec3 velocityChange[2], rotationChange[2];
  glm::vec3 deltaVel;

  // iteratively handle impacts in order of severity.
  _velocityIterationsUsed = 0;
  while (_velocityIterationsUsed < _velocityIterations) {
    // Find contact with maximum magnitude of probable velocity change.
    real_t max = _velocityEpsilon;
    unsigned index = numContacts;
    for (unsigned i = 0; i < numContacts; i++) {
      if (c[i]._desiredDeltaVelocity > max) {
        max = c[i]._desiredDeltaVelocity;
        index = i;
      }
    }
    if (index == numContacts)
      break;

    // Match the awake state at the contact
    c[index].matchAwakeState();

    // Do the resolution on the contact that came out top.
    c[index].applyVelocityChange(velocityChange, rotationChange);

    // With the change in velocity of the two bodies, the update of
    // contact velocities means that some of the relative closing
    // velocities need recomputing.
    for (unsigned i = 0; i < numContacts; i++) {
      // Check each body in the contact
      for (unsigned b = 0; b < 2; b++)
        if (c[i]._body[b]) {
          // Check for a match with each body in the newly
          // resolved contact
          for (unsigned d = 0; d < 2; d++) {
            if (c[i]._body[b] == c[index]._body[d]) {
              deltaVel = velocityChange[d] +
                         glm::cross(rotationChange[d],
                                    c[i]._relativeContactPosition[b]);

              // The sign of the change is negative if we're dealing
              // with the second body in a contact.
              c[i]._contactVelocity += (b ? 1.0f : -1.0f) *
                                       glm::transpose(c[i]._contactToWorld) *
                                       deltaVel;
              c[i].calculateDesiredDeltaVelocity(duration);
            }
          }
        }
    }
    _velocityIterationsUsed++;
  }
}

void ft::ContactResolver::adjustPositions(Contact *c, unsigned numContacts,
                                          real_t duration) {
  (void)duration;
  unsigned i, index;
  glm::vec3 linearChange[2], angularChange[2];
  real_t max;
  glm::vec3 deltaPosition;

  // iteratively resolve interpenetrations in order of severity.
  _positionIterationsUsed = 0;
  while (_positionIterationsUsed < _positionIterations) {
    // Find biggest penetration
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

    // Match the awake state at the contact
    c[index].matchAwakeState();

    // Resolve the penetration.
    c[index].applyPositionChange(linearChange, angularChange, max);

    // Again this action may have changed the penetration of other
    // bodies, so we update contacts.
    for (i = 0; i < numContacts; i++) {
      // Check each body in the contact
      for (unsigned b = 0; b < 2; b++)
        if (c[i]._body[b]) {
          // Check for a match with each body in the newly
          // resolved contact
          for (unsigned d = 0; d < 2; d++) {
            if (c[i]._body[b] == c[index]._body[d]) {
              deltaPosition = linearChange[d] +
                              glm::cross(angularChange[d],
                                         c[i]._relativeContactPosition[b]);

              // The sign of the change is positive if we're
              // dealing with the second body in a contact
              // and negative otherwise (because we're
              // subtracting the resolution)..
              c[i]._penetration +=
                  glm::dot(deltaPosition, c[i]._contactNormal) * (b ? 1 : -1);
            }
          }
        }
    }
    _positionIterationsUsed++;
  }
}
