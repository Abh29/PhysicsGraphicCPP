#include "../includes/ft_collideFine.h"
#include <assert.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <limits>
#include <memory.h>

void ft::CollisionPrimitive::calculateInternals() {
  transform = body->getTransform() * offset;
}

bool ft::IntersectionTests::sphereAndHalfSpace(const CollisionSphere &sphere,
                                               const CollisionPlane &plane) {
  // Find the distance from the origin
  real_t ballDistance =
      glm::dot(plane.direction, sphere.getAxis(3)) - sphere.radius;

  // Check for the intersection
  return ballDistance <= plane.offset;
}

bool ft::IntersectionTests::sphereAndSphere(const CollisionSphere &one,
                                            const CollisionSphere &two) {
  // Find the vector between the objects
  glm::vec3 midline = one.getAxis(3) - two.getAxis(3);

  // See if it is large enough.
  return glm::length2(midline) <
         (one.radius + two.radius) * (one.radius + two.radius);
}

static inline real_t transformToAxis(const ft::CollisionBox &box,
                                     const glm::vec3 &axis) {
  return box.halfSize.x * std::abs(glm::dot(axis, box.getAxis(0))) +
         box.halfSize.y * std::abs(glm::dot(axis, box.getAxis(1))) +
         box.halfSize.z * std::abs(glm::dot(axis, box.getAxis(2)));
}

/**
 * This function checks if the two boxes overlap
 * along the given axis. The final parameter toCentre
 * is used to pass in the vector between the boxes centre
 * points, to avoid having to recalculate it each time.
 */
static inline bool overlapOnAxis(const ft::CollisionBox &one,
                                 const ft::CollisionBox &two,
                                 const glm::vec3 &axis,
                                 const glm::vec3 &toCentre) {
  // Project the half-size of one onto axis
  real_t oneProject = transformToAxis(one, axis);
  real_t twoProject = transformToAxis(two, axis);

  // Project this onto the axis
  real_t distance = std::abs(glm::dot(toCentre, axis));

  // Check for overlap
  return (distance < oneProject + twoProject);
}

// This preprocessor definition is only used as a convenience
// in the boxAndBox intersection  method.
#define TEST_OVERLAP(axis) overlapOnAxis(one, two, (axis), toCentre)

bool ft::IntersectionTests::boxAndBox(const CollisionBox &one,
                                      const CollisionBox &two) {
  // Find the vector between the two centres
  glm::vec3 toCentre = two.getAxis(3) - one.getAxis(3);

  return (
      // Check on box one's axes first
      TEST_OVERLAP(one.getAxis(0)) && TEST_OVERLAP(one.getAxis(1)) &&
      TEST_OVERLAP(one.getAxis(2)) &&

      // And on two's
      TEST_OVERLAP(two.getAxis(0)) && TEST_OVERLAP(two.getAxis(1)) &&
      TEST_OVERLAP(two.getAxis(2)) &&

      // Now on the cross products
      TEST_OVERLAP(glm::cross(one.getAxis(0), two.getAxis(0))) &&
      TEST_OVERLAP(glm::cross(one.getAxis(0), two.getAxis(1))) &&
      TEST_OVERLAP(glm::cross(one.getAxis(0), two.getAxis(2))) &&
      TEST_OVERLAP(glm::cross(one.getAxis(1), two.getAxis(0))) &&
      TEST_OVERLAP(glm::cross(one.getAxis(1), two.getAxis(1))) &&
      TEST_OVERLAP(glm::cross(one.getAxis(1), two.getAxis(2))) &&
      TEST_OVERLAP(glm::cross(one.getAxis(2), two.getAxis(0))) &&
      TEST_OVERLAP(glm::cross(one.getAxis(2), two.getAxis(1))) &&
      TEST_OVERLAP(glm::cross(one.getAxis(2), two.getAxis(2))));
}
#undef TEST_OVERLAP

bool ft::IntersectionTests::boxAndHalfSpace(const CollisionBox &box,
                                            const CollisionPlane &plane) {
  // Work out the projected radius of the box onto the plane direction
  real_t projectedRadius = transformToAxis(box, plane.direction);

  // Work out how far the box is from the origin
  real_t boxDistance =
      glm::dot(plane.direction, box.getAxis(3)) - projectedRadius;

  // Check for the intersection
  return boxDistance <= plane.offset;
}

unsigned
ft::CollisionDetector::sphereAndTruePlane(const CollisionSphere &sphere,
                                          const CollisionPlane &plane,
                                          CollisionData *data) {
  // Make sure we have contacts
  if (data->contactsLeft <= 0)
    return 0;

  // Cache the sphere position
  glm::vec3 position = sphere.getAxis(3);

  // Find the distance from the plane
  real_t centreDistance = glm::dot(plane.direction, position) - plane.offset;

  // Check if we're within radius
  if (centreDistance * centreDistance > sphere.radius * sphere.radius) {
    return 0;
  }

  // Check which side of the plane we're on
  glm::vec3 normal = plane.direction;
  real_t penetration = -centreDistance;
  if (centreDistance < 0) {
    normal *= -1;
    penetration = -penetration;
  }
  penetration += sphere.radius;

  // Create the contact - it has a normal in the plane direction.
  Contact *contact = data->contacts;
  contact->_contactNormal = normal;
  contact->_penetration = penetration;
  contact->_contactPoint = position - plane.direction * centreDistance;
  contact->setBodyData(sphere.body, NULL, data->friction, data->restitution);

  data->addContacts(1);
  return 1;
}

unsigned
ft::CollisionDetector::sphereAndHalfSpace(const CollisionSphere &sphere,
                                          const CollisionPlane &plane,
                                          CollisionData *data) {
  // Make sure we have contacts
  if (data->contactsLeft <= 0)
    return 0;

  // Cache the sphere position
  glm::vec3 position = sphere.getAxis(3);

  // Find the distance from the plane
  real_t ballDistance =
      glm::dot(plane.direction, position) - sphere.radius - plane.offset;

  if (ballDistance >= 0)
    return 0;

  // Create the contact - it has a normal in the plane direction.
  Contact *contact = data->contacts;
  contact->_contactNormal = plane.direction;
  contact->_penetration = -ballDistance;
  contact->_contactPoint =
      position - plane.direction * (ballDistance + sphere.radius);
  contact->setBodyData(sphere.body, NULL, data->friction, data->restitution);

  data->addContacts(1);
  return 1;
}

unsigned ft::CollisionDetector::sphereAndSphere(const CollisionSphere &one,
                                                const CollisionSphere &two,
                                                CollisionData *data) {
  // Make sure we have contacts
  if (data->contactsLeft <= 0)
    return 0;

  // Cache the sphere positions
  glm::vec3 positionOne = one.getAxis(3);
  glm::vec3 positionTwo = two.getAxis(3);

  // Find the vector between the objects
  glm::vec3 midline = positionOne - positionTwo;
  real_t size = glm::length(midline);

  // See if it is large enough.
  if (size <= 0.0f || size >= one.radius + two.radius) {
    return 0;
  }

  // We manually create the normal, because we have the
  // size to hand.
  glm::vec3 normal = midline * (((real_t)1.0) / size);

  Contact *contact = data->contacts;
  contact->_contactNormal = normal;
  contact->_contactPoint = positionOne + midline * (real_t)0.5;
  contact->_penetration = (one.radius + two.radius - size);
  contact->setBodyData(one.body, two.body, data->friction, data->restitution);

  data->addContacts(1);
  return 1;
}

/*
 * This function checks if the two boxes overlap
 * along the given axis, returning the ammount of overlap.
 * The final parameter toCentre
 * is used to pass in the vector between the boxes centre
 * points, to avoid having to recalculate it each time.
 */
static inline real_t penetrationOnAxis(const ft::CollisionBox &one,
                                       const ft::CollisionBox &two,
                                       const glm::vec3 &axis,
                                       const glm::vec3 &toCentre) {
  // Project the half-size of one onto axis
  real_t oneProject = transformToAxis(one, axis);
  real_t twoProject = transformToAxis(two, axis);

  // Project this onto the axis
  real_t distance = std::abs(glm::dot(toCentre, axis));

  // Return the overlap (i.e. positive indicates
  // overlap, negative indicates separation).
  return oneProject + twoProject - distance;
}

static inline bool tryAxis(const ft::CollisionBox &one,
                           const ft::CollisionBox &two, glm::vec3 axis,
                           const glm::vec3 &toCentre, unsigned index,

                           // These values may be updated
                           real_t &smallestPenetration,
                           unsigned &smallestCase) {
  // Make sure we have a normalized axis, and don't check almost parallel axes
  if (glm::length2(axis) < 0.0001)
    return true;
  axis = glm::normalize(axis);

  real_t penetration = penetrationOnAxis(one, two, axis, toCentre);

  if (penetration < 0)
    return false;
  if (penetration < smallestPenetration) {
    smallestPenetration = penetration;
    smallestCase = index;
  }
  return true;
}

void fillPointFaceBoxBox(const ft::CollisionBox &one,
                         const ft::CollisionBox &two, const glm::vec3 &toCentre,
                         ft::CollisionData *data, unsigned best, real_t pen) {
  // This method is called when we know that a vertex from
  // box two is in contact with box one.

  ft::Contact *contact = data->contacts;

  // We know which axis the collision is on (i.e. best),
  // but we need to work out which of the two faces on
  // this axis.
  glm::vec3 normal = one.getAxis(best);
  if (glm::dot(one.getAxis(best), toCentre) > 0) {
    normal = normal * -1.0f;
  }

  // Work out which vertex of box two we're colliding with.
  // Using toCentre doesn't work!
  glm::vec3 vertex = two.halfSize;
  if (glm::dot(two.getAxis(0), normal) < 0)
    vertex.x = -vertex.x;
  if (glm::dot(two.getAxis(1), normal) < 0)
    vertex.y = -vertex.y;
  if (glm::dot(two.getAxis(2), normal) < 0)
    vertex.z = -vertex.z;

  // Create the contact data
  contact->_contactNormal = normal;
  contact->_penetration = pen;
  contact->_contactPoint = two.getTransform() * glm::vec4(vertex, 1.0f);
  contact->setBodyData(one.body, two.body, data->friction, data->restitution);
}

static inline glm::vec3
contactPoint(const glm::vec3 &pOne, const glm::vec3 &dOne, real_t oneSize,
             const glm::vec3 &pTwo, const glm::vec3 &dTwo, real_t twoSize,

             // If this is true, and the contact point is outside
             // the edge (in the case of an edge-face contact) then
             // we use one's midpoint, otherwise we use two's.
             bool useOne) {
  glm::vec3 toSt, cOne, cTwo;
  real_t dpStaOne, dpStaTwo, dpOneTwo, smOne, smTwo;
  real_t denom, mua, mub;

  smOne = glm::length2(dOne);
  smTwo = glm::length2(dTwo);
  dpOneTwo = glm::dot(dTwo, dOne);

  toSt = pOne - pTwo;
  dpStaOne = glm::dot(dOne, toSt);
  dpStaTwo = glm::dot(dTwo, toSt);

  denom = smOne * smTwo - dpOneTwo * dpOneTwo;

  // Zero denominator indicates parrallel lines
  if (std::abs(denom) < 0.0001f) {
    return useOne ? pOne : pTwo;
  }

  mua = (dpOneTwo * dpStaTwo - smTwo * dpStaOne) / denom;
  mub = (smOne * dpStaTwo - dpOneTwo * dpStaOne) / denom;

  // If either of the edges has the nearest point out
  // of bounds, then the edges aren't crossed, we have
  // an edge-face contact. Our point is on the edge, which
  // we know from the useOne parameter.
  if (mua > oneSize || mua < -oneSize || mub > twoSize || mub < -twoSize) {
    return useOne ? pOne : pTwo;
  } else {
    cOne = pOne + dOne * mua;
    cTwo = pTwo + dTwo * mub;

    return cOne * 0.5f + cTwo * 0.5f;
  }
}

// This preprocessor definition is only used as a convenience
// in the boxAndBox contact generation method.
#define CHECK_OVERLAP(axis, index)                                             \
  if (!tryAxis(one, two, (axis), toCentre, (index), pen, best))                \
    return 0;

unsigned ft::CollisionDetector::boxAndBox(const CollisionBox &one,
                                          const CollisionBox &two,
                                          CollisionData *data) {
  // if (!IntersectionTests::boxAndBox(one, two)) return 0;

  // Find the vector between the two centres
  glm::vec3 toCentre = two.getAxis(3) - one.getAxis(3);

  // We start assuming there is no contact
  real_t pen = std::numeric_limits<real_t>::max();
  unsigned best = 0xffffff;

  // Now we check each axes, returning if it gives us
  // a separating axis, and keeping track of the axis with
  // the smallest penetration otherwise.
  CHECK_OVERLAP(one.getAxis(0), 0);
  CHECK_OVERLAP(one.getAxis(1), 1);
  CHECK_OVERLAP(one.getAxis(2), 2);

  CHECK_OVERLAP(two.getAxis(0), 3);
  CHECK_OVERLAP(two.getAxis(1), 4);
  CHECK_OVERLAP(two.getAxis(2), 5);

  // Store the best axis-major, in case we run into almost
  // parallel edge collisions later
  unsigned bestSingleAxis = best;

  CHECK_OVERLAP(glm::cross(one.getAxis(0), two.getAxis(0)), 6);
  CHECK_OVERLAP(glm::cross(one.getAxis(0), two.getAxis(1)), 7);
  CHECK_OVERLAP(glm::cross(one.getAxis(0), two.getAxis(2)), 8);
  CHECK_OVERLAP(glm::cross(one.getAxis(1), two.getAxis(0)), 9);
  CHECK_OVERLAP(glm::cross(one.getAxis(1), two.getAxis(1)), 10);
  CHECK_OVERLAP(glm::cross(one.getAxis(1), two.getAxis(2)), 11);
  CHECK_OVERLAP(glm::cross(one.getAxis(2), two.getAxis(0)), 12);
  CHECK_OVERLAP(glm::cross(one.getAxis(2), two.getAxis(1)), 13);
  CHECK_OVERLAP(glm::cross(one.getAxis(2), two.getAxis(2)), 14);

  // Make sure we've got a result.
  assert(best != 0xffffff);

  // We now know there's a collision, and we know which
  // of the axes gave the smallest penetration. We now
  // can deal with it in different ways depending on
  // the case.
  if (best < 3) {
    // We've got a vertex of box two on a face of box one.
    fillPointFaceBoxBox(one, two, toCentre, data, best, pen);
    data->addContacts(1);
    return 1;
  } else if (best < 6) {
    // We've got a vertex of box one on a face of box two.
    // We use the same algorithm as above, but swap around
    // one and two (and therefore also the vector between their
    // centres).
    fillPointFaceBoxBox(two, one, toCentre * -1.0f, data, best - 3, pen);
    data->addContacts(1);
    return 1;
  } else {
    // We've got an edge-edge contact. Find out which axes
    best -= 6;
    unsigned oneAxisIndex = best / 3;
    unsigned twoAxisIndex = best % 3;
    glm::vec3 oneAxis = one.getAxis(oneAxisIndex);
    glm::vec3 twoAxis = two.getAxis(twoAxisIndex);
    glm::vec3 axis = glm::normalize(glm::cross(oneAxis, twoAxis));

    // The axis should point from box one to box two.
    if (glm::dot(axis, toCentre) > 0)
      axis = axis * -1.0f;

    // We have the axes, but not the edges: each axis has 4 edges parallel
    // to it, we need to find which of the 4 for each object. We do
    // that by finding the point in the centre of the edge. We know
    // its component in the direction of the box's collision axis is zero
    // (its a mid-point) and we determine which of the extremes in each
    // of the other axes is closest.
    glm::vec3 ptOnOneEdge = one.halfSize;
    glm::vec3 ptOnTwoEdge = two.halfSize;
    for (unsigned i = 0; i < 3; i++) {
      if (i == oneAxisIndex)
        ptOnOneEdge[i] = 0;
      else if (glm::dot(one.getAxis(i), axis) > 0)
        ptOnOneEdge[i] = -ptOnOneEdge[i];

      if (i == twoAxisIndex)
        ptOnTwoEdge[i] = 0;
      else if (glm::dot(two.getAxis(i), axis) < 0)
        ptOnTwoEdge[i] = -ptOnTwoEdge[i];
    }

    // Move them into world coordinates (they are already oriented
    // correctly, since they have been derived from the axes).
    // todo: test this 0.0f vs 1.0f
    ptOnOneEdge = one.transform * glm::vec4(ptOnOneEdge, 0.0f);
    ptOnTwoEdge = two.transform * glm::vec4(ptOnTwoEdge, 0.0f);

    // So we have a point and a direction for the colliding edges.
    // We need to find out point of closest approach of the two
    // line-segments.
    glm::vec3 vertex = contactPoint(
        ptOnOneEdge, oneAxis, one.halfSize[oneAxisIndex], ptOnTwoEdge, twoAxis,
        two.halfSize[twoAxisIndex], bestSingleAxis > 2);

    // We can fill the contact.
    Contact *contact = data->contacts;

    contact->_penetration = pen;
    contact->_contactNormal = axis;
    contact->_contactPoint = vertex;
    contact->setBodyData(one.body, two.body, data->friction, data->restitution);
    data->addContacts(1);
    return 1;
  }
  return 0;
}
#undef CHECK_OVERLAP

unsigned ft::CollisionDetector::boxAndPoint(const CollisionBox &box,
                                            const glm::vec3 &point,
                                            CollisionData *data) {
  // Transform the point into box coordinates
  glm::vec3 relPt = box.transform * glm::vec4(point, 1.0f);

  glm::vec3 normal;

  // Check each axis, looking for the axis on which the
  // penetration is least deep.
  real_t min_depth = box.halfSize.x - std::abs(relPt.x);
  if (min_depth < 0)
    return 0;
  normal = box.getAxis(0) * ((relPt.x < 0) ? -1.0f : 1.0f);

  real_t depth = box.halfSize.y - std::abs(relPt.y);
  if (depth < 0)
    return 0;
  else if (depth < min_depth) {
    min_depth = depth;
    normal = box.getAxis(1) * ((relPt.y < 0) ? -1.0f : 1.0f);
  }

  depth = box.halfSize.z - std::abs(relPt.z);
  if (depth < 0)
    return 0;
  else if (depth < min_depth) {
    min_depth = depth;
    normal = box.getAxis(2) * ((relPt.z < 0) ? -1.0f : 1.0f);
  }

  // Compile the contact
  Contact *contact = data->contacts;
  contact->_contactNormal = normal;
  contact->_contactPoint = point;
  contact->_penetration = min_depth;

  // Note that we don't know what rigid body the point
  // belongs to, so we just use NULL. Where this is called
  // this value can be left, or filled in.
  contact->setBodyData(box.body, NULL, data->friction, data->restitution);

  data->addContacts(1);
  return 1;
}

unsigned ft::CollisionDetector::boxAndSphere(const CollisionBox &box,
                                             const CollisionSphere &sphere,
                                             CollisionData *data) {
  // Transform the centre of the sphere into box coordinates
  glm::vec3 centre = sphere.getAxis(3);
  glm::vec3 relCentre = box.transform * glm::vec4(centre, 1.0f);

  // Early out check to see if we can exclude the contact
  if (std::abs(relCentre.x) - sphere.radius > box.halfSize.x ||
      std::abs(relCentre.y) - sphere.radius > box.halfSize.y ||
      std::abs(relCentre.z) - sphere.radius > box.halfSize.z) {
    return 0;
  }

  glm::vec3 closestPt(0, 0, 0);
  real_t dist;

  // Clamp each coordinate to the box.
  dist = relCentre.x;
  if (dist > box.halfSize.x)
    dist = box.halfSize.x;
  if (dist < -box.halfSize.x)
    dist = -box.halfSize.x;
  closestPt.x = dist;

  dist = relCentre.y;
  if (dist > box.halfSize.y)
    dist = box.halfSize.y;
  if (dist < -box.halfSize.y)
    dist = -box.halfSize.y;
  closestPt.y = dist;

  dist = relCentre.z;
  if (dist > box.halfSize.z)
    dist = box.halfSize.z;
  if (dist < -box.halfSize.z)
    dist = -box.halfSize.z;
  closestPt.z = dist;

  // Check we're in contact
  dist = glm::length2((closestPt - relCentre));
  if (dist > sphere.radius * sphere.radius)
    return 0;

  // Compile the contact
  glm::vec3 closestPtWorld = box.transform * glm::vec4(closestPt, 1.0f);

  Contact *contact = data->contacts;
  contact->_contactNormal = glm::normalize(closestPtWorld - centre);
  contact->_contactPoint = closestPtWorld;
  contact->_penetration = sphere.radius - std::sqrt(dist);
  contact->setBodyData(box.body, sphere.body, data->friction,
                       data->restitution);

  data->addContacts(1);
  return 1;
}

unsigned ft::CollisionDetector::boxAndHalfSpace(const CollisionBox &box,
                                                const CollisionPlane &plane,
                                                CollisionData *data) {
  // Make sure we have contacts
  if (data->contactsLeft <= 0)
    return 0;

  // Check for intersection
  if (!IntersectionTests::boxAndHalfSpace(box, plane)) {
    return 0;
  }

  // We have an intersection, so find the intersection points. We can make
  // do with only checking vertices. If the box is resting on a plane
  // or on an edge, it will be reported as four or two contact points.

  // Go through each combination of + and - for each half-size
  static real_t mults[8][3] = {{1, 1, 1},   {-1, 1, 1},  {1, -1, 1},
                               {-1, -1, 1}, {1, 1, -1},  {-1, 1, -1},
                               {1, -1, -1}, {-1, -1, -1}};

  Contact *contact = data->contacts;
  unsigned contactsUsed = 0;
  for (unsigned i = 0; i < 8; i++) {

    // Calculate the position of each vertex
    glm::vec3 vertexPos(mults[i][0], mults[i][1], mults[i][2]);
    // this is a hadamard product
    vertexPos = vertexPos * box.halfSize;
    vertexPos = box.transform * glm::vec4(vertexPos, 1.0f);

    // Calculate the distance from the plane
    real_t vertexDistance = glm::dot(vertexPos, plane.direction);

    // Compare this to the plane's distance
    if (vertexDistance <= plane.offset) {
      // Create the contact data.

      // The contact point is halfway between the vertex and the
      // plane - we multiply the direction by half the separation
      // distance and add the vertex location.
      contact->_contactPoint = plane.direction;
      contact->_contactPoint *= (vertexDistance - plane.offset);
      contact->_contactPoint += vertexPos;
      contact->_contactNormal = plane.direction;
      contact->_penetration = plane.offset - vertexDistance;

      // Write the appropriate data
      contact->setBodyData(box.body, NULL, data->friction, data->restitution);

      // Move onto the next contact
      contact++;
      contactsUsed++;
      if (contactsUsed == (unsigned)data->contactsLeft)
        return contactsUsed;
    }
  }

  data->addContacts(contactsUsed);
  return contactsUsed;
}
