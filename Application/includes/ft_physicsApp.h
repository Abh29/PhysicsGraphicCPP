#ifndef FT_PHYSICS_APPLICATION
#define FT_PHYSICS_APPLICATION

#include "ft_collideFine.h"
#include "ft_contacts.h"
#include "ft_headers.h"
#include "ft_rigidObject.h"

namespace ft {

class PhysicsApplication {
public:
  using pointer = std::shared_ptr<PhysicsApplication>;
  using raw_ptr = PhysicsApplication *;

  PhysicsApplication() = default;
  virtual ~PhysicsApplication() = default;
  virtual void update(real_t duration) = 0;
};

class MassAggregateApplication : public PhysicsApplication {};

class RigidBodyApplication : public PhysicsApplication {
public:
  using pointer = std::shared_ptr<RigidBodyApplication>;
  using raw_ptr = RigidBodyApplication *;

  RigidBodyApplication(uint32_t maxContacts = 256);
  ~RigidBodyApplication(){/*delete _contacts;*/};

  void play();
  void pause();

protected:
  const uint32_t _maxContacts;
  // ft::Contact::raw_ptr _contacts;
  std::vector<ft::Contact> _contacts;
  ft::CollisionData _collisionData;
  ft::ContactResolver _resolver;
  bool _pauseSimulation = false;
};

class SimpleRigidApplication : public RigidBodyApplication {

public:
  using pointer = std::shared_ptr<SimpleRigidApplication>;
  using raw_ptr = SimpleRigidApplication *;

  SimpleRigidApplication(uint32_t maxContacts = 256);
  void update(real_t duration) override;

  inline std::vector<ft::RigidBox::pointer> &getBoxes();
  inline std::vector<ft::RigidBall::pointer> &getBalls();
  void addRigidBox(const RigidBox::pointer &box);
  void addRigidBall(const RigidBall::pointer &ball);
  void removeRigidBox(RigidBox::pointer box);
  void removeRigidBall(RigidBall::pointer ball);
  void addCollisionPlane(const CollisionPlane::pointer &plane);
  void removeCollisionPlane(CollisionPlane::pointer plane);

protected:
  void generateContacts();
  void updateObjects(real_t duration);

  std::vector<ft::RigidBox::pointer> _boxes;
  std::vector<ft::RigidBall::pointer> _balls;
  std::vector<ft::CollisionPlane::pointer> _planes;
};

} // namespace ft
#endif // !FT_PHYSICS_APPLICATION
