#include "../includes/ft_joint.h"
#include <cstdlib>
#include <glm/geometric.hpp>

unsigned ft::Joint::addContact(Contact::raw_ptr contact, unsigned limit) const {
  (void)limit;
  // Calculate the position of each connection point in world coordinates
  glm::vec3 a_pos_world = _body[0]->getPointInWorldSpace(_position[0]);
  glm::vec3 b_pos_world = _body[1]->getPointInWorldSpace(_position[1]);

  // Calculate the length of the joint
  glm::vec3 a_to_b = b_pos_world - a_pos_world;
  glm::vec3 normal = glm::normalize(a_to_b);

  real_t length = glm::length(a_to_b);

  // Check if it is violated
  if (std::abs(length) > error) {
    contact->_body[0] = _body[0];
    contact->_body[1] = _body[1];
    contact->_contactNormal = normal;
    contact->_contactPoint = (a_pos_world + b_pos_world) * 0.5f;
    contact->_penetration = length - error;
    contact->_friction = 1.0f;
    contact->_restitution = 0;
    return 1;
  }

  return 0;
}

void ft::Joint::set(RigidBody::raw_ptr a, const glm::vec3 &a_pos,
                    RigidBody::raw_ptr b, const glm::vec3 &b_pos,
                    real_t error) {
  _body[0] = a;
  _body[1] = b;

  _position[0] = a_pos;
  _position[1] = b_pos;

  Joint::error = error;
}
