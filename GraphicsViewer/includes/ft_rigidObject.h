#ifndef FT_SIMPLE_RIGID_OBJECT
#define FT_SIMPLE_RIGID_OBJECT

#include "ft_body.h"
#include "ft_collideFine.h"
#include "ft_headers.h"

namespace ft {

class RigidBall : public ft::CollisionSphere {
public:
  using pointer = std::shared_ptr<RigidBall>;
  using raw_ptr = RigidBall *;

  void setState(const glm::vec3 &position, const glm::quat &orientation,
                const float radius, const glm::vec3 &velocity);

private:
  glm::mat3 getMatrixFromInertiaTensor(float ix, float iy, float iz,
                                       float ixy = 0, float ixz = 0,
                                       float iyz = 0);
};

class RigidBox : public ft::CollisionBox {
public:
  using pointer = std::shared_ptr<RigidBox>;
  using raw_ptr = RigidBox *;

  void setState(const glm::vec3 &position, const glm::quat &orientation,
                const glm::vec3 &extents, const glm::vec3 &velocity);

  inline void setOverlap(bool overlap) { _isOverlapping = overlap; }
  inline bool isOverlapping() const { return _isOverlapping; }

private:
  glm::mat3 getMatrixFromInertiaTensor(float ix, float iy, float iz,
                                       float ixy = 0, float ixz = 0,
                                       float iyz = 0);

  bool _isOverlapping = false;
};

}; // namespace ft
#endif // !FT_SIMPLE_RIGID_OBJECT
