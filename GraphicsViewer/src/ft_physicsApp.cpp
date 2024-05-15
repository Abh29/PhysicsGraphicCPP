#include "../includes/ft_physicsApp.h"
#include "ft_collideFine.h"
#include "ft_contacts.h"
#include <cstdint>
#include <glm/fwd.hpp>

// class PhysicsApplication {
// public:
//   using pointer = std::shared_ptr<PhysicsApplication>;
//   using raw_ptr = PhysicsApplication *;
//
//   virtual ~PhysicsApplication() = default;
//   virtual void update() = 0;
// };
//
// class MassAggregateApplication : public PhysicsApplication {};
//
// class RigidBodyApplication : public PhysicsApplication {
// public:
//   using pointer = std::shared_ptr<RigidBodyApplication>;
//   using raw_ptr = RigidBodyApplication *;
//
//   RigidBodyApplication(uint32_t maxContacts = 256);
//   ~RigidBodyApplication() = default;
//
//   void play();
//   void pause();
//
// protected:
//   constexpr uint32_t _maxContacts;
//   std::vector<ft::Contact> _contacts;
//   ft::CollisionData _collisionData;
//   ft::ContactResolver _resolver;
//   bool _pauseSimulation = true;
// };
//
// class SimpleRigidApplication : public RigidBodyApplication {
//
// public:
//   using pointer = std::shared_ptr<SimpleRigidApplication>;
//   using raw_ptr = SimpleRigidApplication *;
//
//   SimpleRigidApplication(uint32_t maxContacts = 256);
//
// protected:
//   void generateContacts();
//   void updateObjects();
//
//   std::vector<ft::RigidBox::pointer> _boxes;
//   std::vector<ft::RigidBall::pointer> _balls;
//   ft::CollisionPlane _plane;
// };
//
//

ft::RigidBodyApplication::RigidBodyApplication(uint32_t maxContacts)
    : _maxContacts(maxContacts), _resolver(maxContacts * 8) {
  _contacts = new ft::Contact[maxContacts];
  _collisionData.contactArray = _contacts;
}

void ft::RigidBodyApplication::play() { _pauseSimulation = false; }
void ft::RigidBodyApplication::pause() { _pauseSimulation = true; }

/************************************SimplePhysicsApplication********************************/

ft::SimpleRigidApplication::SimpleRigidApplication(uint32_t maxContacts)
    : RigidBodyApplication(maxContacts) {
  _plane.direction = glm::vec3(0.0f, 1.0f, 0.0);
  _plane.offset = 0.0f;
}

void ft::SimpleRigidApplication::update(real_t duration) {

  if (duration <= 0)
    return;

  if (_pauseSimulation)
    return;

  updateObjects(duration);

  generateContacts();

  _resolver.resolveContacts(_collisionData.contactArray,
                            _collisionData.contactCount, duration);
}

void ft::SimpleRigidApplication::updateObjects(real_t duration) {

  for (auto &b : _boxes) {
    b->body->integrate(duration);
    b->calculateInternals();
  }

  for (auto &b : _balls) {
    b->body->integrate(duration);
    b->calculateInternals();
  }
}

void ft::SimpleRigidApplication::generateContacts() {

  // set up the collision data structure
  _collisionData.reset(_maxContacts);
  _collisionData.friction = 0.9f;
  _collisionData.restitution = 0.6;
  _collisionData.tolerance = 0.15f;

  // perform collision detection
  // todo: make use of the threadpool

  // first for the boxes
  for (auto &b : _boxes) {

    if (!_collisionData.hasMoreContacts())
      return;

    // collision with the ground plane
    ft::CollisionDetector::boxAndHalfSpace(*b, _plane, &_collisionData);

    // collision with other boxes
    for (auto &bb : _boxes) {
      if (bb == b)
        continue;
      if (!_collisionData.hasMoreContacts())
        return;

      ft::CollisionDetector::boxAndBox(*b, *bb, &_collisionData);
      if (ft::IntersectionTests::boxAndBox(*b, *bb)) {
        b->setOverlap(true);
        bb->setOverlap(true);
      }
    }

    // collision with other balls
    for (auto &ba : _balls) {
      if (!_collisionData.hasMoreContacts())
        return;
      ft::CollisionDetector::boxAndSphere(*b, *ba, &_collisionData);
    }
  }

  // then for the balls
  for (auto &b : _balls) {
    if (!_collisionData.hasMoreContacts())
      return;

    // collision with the ground plane
    ft::CollisionDetector::sphereAndHalfSpace(*b, _plane, &_collisionData);

    // collision with other balls
    for (auto &ba : _balls) {
      if (ba == b)
        continue;
      if (!_collisionData.hasMoreContacts())
        return;
      ft::CollisionDetector::sphereAndSphere(*b, *ba, &_collisionData);
    }
  }
}

std::vector<ft::RigidBox::pointer> &ft::SimpleRigidApplication::getBoxes() {
  return _boxes;
}

std::vector<ft::RigidBall::pointer> &ft::SimpleRigidApplication::getBalls() {
  return _balls;
};

void ft::SimpleRigidApplication::addRigidBox(const RigidBox::pointer &box) {
  _boxes.push_back(box);
}

void ft::SimpleRigidApplication::addRigidBall(const RigidBall::pointer &ball) {
  _balls.push_back(ball);
}

void ft::SimpleRigidApplication::removeRigidBox(RigidBox::pointer box) {
  _boxes.erase(std::find(_boxes.begin(), _boxes.end(), box));
}

void ft::SimpleRigidApplication::removeRigidBall(RigidBall::pointer ball) {
  _balls.erase(std::find(_balls.begin(), _balls.end(), ball));
}
