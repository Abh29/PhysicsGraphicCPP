#ifndef INCLUDE_INCLUDES_FT_COMPONENT_H_
#define INCLUDE_INCLUDES_FT_COMPONENT_H_

#include "ft_headers.h"

namespace ft {

struct Component {

  using pointer = std::shared_ptr<Component>;
  using raw_ptr = Component *;

  virtual ~Component() = default;
  virtual void update(float duration) = 0;
};

}; // namespace ft

#endif // INCLUDE_INCLUDES_FT_COMPONENT_H_
