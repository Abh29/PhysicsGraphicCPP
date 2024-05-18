#ifndef FT_COLLISION_FINE_H
#define FT_COLLISION_FINE_H

#include "ft_contacts.h"
#include <glm/fwd.hpp>
#include <memory>

namespace ft {

// Forward declarations of primitive friends
class IntersectionTests;
class CollisionDetector;

/**
 * Represents a primitive to detect collisions against.
 */
class CollisionPrimitive {
public:
  using pointer = std::shared_ptr<CollisionPrimitive>;
  using raw_ptr = CollisionPrimitive *;

  friend class IntersectionTests;
  friend class CollisionDetector;

  /**
   * The rigid body that is represented by this primitive.
   */
  RigidBody *body;

  /**
   * The offset of this primitive from the given rigid body.
   */
  glm::mat4 offset = glm::mat4(1.0f);

  /**
   * Calculates the internals for the primitive.
   */
  void calculateInternals();

  /**
   * This is a convenience function to allow access to the
   * axis vectors in the transform for this primitive.
   */
  glm::vec3 getAxis(unsigned index) const {
    assert(index < 4);
    return transform[index];
  }

  const glm::mat4 &getTransform() const { return transform; }

protected:
  glm::mat4 transform = glm::mat4(1.0f);
};

/**
 * Represents a rigid body that can be treated as a sphere
 * for collision detection.
 */
class CollisionSphere : public CollisionPrimitive {
public:
  using pointer = std::shared_ptr<CollisionSphere>;
  using raw_ptr = CollisionSphere *;

  /**
   * The radius of the sphere.
   */
  real_t radius;
};

/**
 * The plane is not a primitive: it doesn't represent another
 * rigid body. It is used for contacts with the immovable
 * world geometry.
 */
class CollisionPlane {
public:
  using pointer = std::shared_ptr<CollisionPlane>;
  using raw_ptr = CollisionPlane *;

  /**
   * The plane normal
   */
  glm::vec3 direction;

  /**
   * The distance of the plane from the origin.
   */
  real_t offset;
};

/**
 * Represents a rigid body that can be treated as an aligned bounding
 * box for collision detection.
 */
class CollisionBox : public CollisionPrimitive {
public:
  using pointer = std::shared_ptr<CollisionBox>;
  using raw_ptr = CollisionBox *;

  /**
   * Holds the half-sizes of the box along each of its local axes.
   */
  glm::vec3 halfSize;
};

/**
 * A wrapper class that holds fast intersection tests. These
 * can be used to drive the coarse collision detection system or
 * as an early out in the full collision tests below.
 */
class IntersectionTests {
public:
  static bool sphereAndHalfSpace(const CollisionSphere &sphere,
                                 const CollisionPlane &plane);

  static bool sphereAndSphere(const CollisionSphere &one,
                              const CollisionSphere &two);

  static bool boxAndBox(const CollisionBox &one, const CollisionBox &two);

  static bool boxAndHalfSpace(const CollisionBox &box,
                              const CollisionPlane &plane);
};

/**
 * A helper structure that contains information for the detector to use
 * in building its contact data.
 */
struct CollisionData {
  Contact *contactArray;

  /** Holds the contact array to write into. */
  Contact *contacts;

  /** Holds the maximum number of contacts the array can take. */
  int contactsLeft;

  /** Holds the number of contacts found so far. */
  unsigned contactCount;

  /** Holds the friction value to write into any collisions. */
  real_t friction;

  /** Holds the restitution value to write into any collisions. */
  real_t restitution;

  /**
   * Holds the collision tolerance, even uncolliding objects this
   * close should have collisions generated.
   */
  real_t tolerance;

  /**
   * Checks if there are more contacts available in the contact
   * data.
   */
  bool hasMoreContacts() { return contactsLeft > 0; }

  /**
   * Resets the data so that it has no used contacts recorded.
   */
  void reset(unsigned maxContacts) {
    contactsLeft = maxContacts;
    contactCount = 0;
    contacts = contactArray;
  }

  /**
   * Notifies the data that the given number of contacts have
   * been added.
   */
  void addContacts(unsigned count) {
    // Reduce the number of contacts remaining, add number used
    contactsLeft -= count;
    contactCount += count;

    // Move the array forward
    contacts += count;
  }
};

class CollisionDetector {
public:
  static unsigned sphereAndHalfSpace(const CollisionSphere &sphere,
                                     const CollisionPlane &plane,
                                     CollisionData *data);

  static unsigned sphereAndTruePlane(const CollisionSphere &sphere,
                                     const CollisionPlane &plane,
                                     CollisionData *data);

  static unsigned sphereAndSphere(const CollisionSphere &one,
                                  const CollisionSphere &two,
                                  CollisionData *data);

  static unsigned boxAndHalfSpace(const CollisionBox &box,
                                  const CollisionPlane &plane,
                                  CollisionData *data);

  static unsigned boxAndBox(const CollisionBox &one, const CollisionBox &two,
                            CollisionData *data);

  static unsigned boxAndPoint(const CollisionBox &box, const glm::vec3 &point,
                              CollisionData *data);

  static unsigned boxAndSphere(const CollisionBox &box,
                               const CollisionSphere &sphere,
                               CollisionData *data);
};

} // namespace ft

#endif // FT_COLLISION_FINE_H
