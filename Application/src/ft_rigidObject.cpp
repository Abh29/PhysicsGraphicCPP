#include "../includes/ft_rigidObject.h"
#include <glm/fwd.hpp>

/*********************************RigidBall***************************/

void ft::RigidBall::setState(const glm::vec3 &position,
                             const glm::quat &orientation, const float radius,
                             const glm::vec3 &velocity) {

  body->setPosition(position);
  body->setOrientation(orientation);
  body->setVelocity(velocity);
  body->setRotation(glm::vec3(0.0f));

  RigidBall::radius = radius;

  real_t mass = 4.0f * 0.3333f * 3.1415f * radius * radius * radius;
  body->setMass(mass);

  real_t coeff = 0.4f * mass * radius * radius;
  glm::mat3 tensor = getMatrixFromInertiaTensor(coeff, coeff, coeff);
  body->setInertiaTensor(tensor);

  body->setLinearDamping(0.95f);
  body->setAngularDamping(0.8f);
  body->clearAccumulators();
  body->setAcceleration(0, -10.0f, 0);

  // body->setCanSleep(false);
  body->setAwake();

  body->calculateDerivedData();
}

glm::mat3 ft::RigidBall::getMatrixFromInertiaTensor(float ix, float iy,
                                                    float iz, float ixy,
                                                    float ixz, float iyz) {

  glm::mat3 matrix = glm::mat3(0.0f); // Initialize the matrix to all zeros

  // Set the diagonal elements
  matrix[0][0] = ix;
  matrix[1][1] = iy;
  matrix[2][2] = iz;

  // Set the off-diagonal elements
  matrix[0][1] = matrix[1][0] = -ixy;
  matrix[0][2] = matrix[2][0] = -ixz;
  matrix[1][2] = matrix[2][1] = -iyz;

  return matrix;
}

/*********************************RigidBox****************************/

void ft::RigidBox::setState(const glm::vec3 &position,
                            const glm::quat &orientation,
                            const glm::vec3 &extent,
                            const glm::vec3 &velocity) {

  body->setPosition(position);
  body->setOrientation(orientation);
  body->setVelocity(velocity);
  body->setRotation(glm::vec3(0, 0, 0));
  halfSize = extent;

  real_t mass = halfSize.x * halfSize.y * halfSize.z * 8.0f;
  body->setMass(mass);

  glm::vec3 squares = halfSize * halfSize;

  // set inertial tensor
  glm::mat3 tensor =
      getMatrixFromInertiaTensor(0.3f * mass * (squares.y + squares.z),
                                 0.3f * mass * (squares.x + squares.z),
                                 0.3f * mass * (squares.x + squares.y));
  body->setInertiaTensor(tensor);
  body->setLinearDamping(0.95f);
  body->setAngularDamping(0.8f);
  body->clearAccumulators();
  body->setAcceleration(0, -10.0f, 0);

  body->setAwake();

  body->calculateDerivedData();
}

glm::mat3 ft::RigidBox::getMatrixFromInertiaTensor(float ix, float iy, float iz,
                                                   float ixy, float ixz,
                                                   float iyz) {

  glm::mat3 matrix = glm::mat3(0.0f); // Initialize the matrix to all zeros

  // Set the diagonal elements
  matrix[0][0] = ix;
  matrix[1][1] = iy;
  matrix[2][2] = iz;

  // Set the off-diagonal elements
  matrix[0][1] = matrix[1][0] = -ixy;
  matrix[0][2] = matrix[2][0] = -ixz;
  matrix[1][2] = matrix[2][1] = -iyz;

  return matrix;
}
