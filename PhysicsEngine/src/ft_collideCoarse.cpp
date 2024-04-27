#include "../includes/ft_collideCoarse.h"
#include <cmath>

ft::BoundingSphere::BoundingSphere(const glm::vec3 &centre, real_t radius) {
  ft::BoundingSphere::centre = centre;
  ft::BoundingSphere::radius = radius;
}

ft::BoundingSphere::BoundingSphere(const BoundingSphere &one,
                                   const BoundingSphere &two) {
  glm::vec3 centreOffset = two.centre - one.centre;
  real_t distance = glm::length2(centreOffset);
  real_t radiusDiff = two.radius - one.radius;

  // Check if the larger sphere encloses the small one
  if (radiusDiff * radiusDiff >= distance) {
    if (one.radius > two.radius) {
      centre = one.centre;
      radius = one.radius;
    } else {
      centre = two.centre;
      radius = two.radius;
    }
  }

  // Otherwise we need to work with partially
  // overlapping spheres
  else {
    distance = std::sqrt(distance);
    radius = (distance + one.radius + two.radius) * ((real_t)0.5);

    // The new centre is based on one's centre, moved towards
    // two's centre by an ammount proportional to the spheres'
    // radii.
    centre = one.centre;
    if (distance > 0) {
      centre += centreOffset * ((radius - one.radius) / distance);
    }
  }
}

bool ft::BoundingSphere::overlaps(const BoundingSphere *other) const {
  real_t distanceSquared = glm::length2(centre - other->centre);
  return distanceSquared < (radius + other->radius) * (radius + other->radius);
}

real_t ft::BoundingSphere::getGrowth(const BoundingSphere &other) const {
  BoundingSphere newSphere(*this, other);

  // We return a value proportional to the change in surface
  // area of the sphere.
  return newSphere.radius * newSphere.radius - radius * radius;
}
