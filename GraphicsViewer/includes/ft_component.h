#ifndef INCLUDE_INCLUDES_FT_COMPONENT_H_
#define INCLUDE_INCLUDES_FT_COMPONENT_H_

#include "ft_body.h"
#include "ft_headers.h"
#include "ft_model.h"
#include "ft_rigidObject.h"
#include <memory>

namespace ft {

struct Component {

  using pointer = std::shared_ptr<Component>;
  using raw_ptr = Component *;

  virtual ~Component() = default;
  virtual void update(float duration) = 0;
};

class RigidBoxComponent : public Component {

public:
  using pointer = std::shared_ptr<RigidBoxComponent>;
  using raw_ptr = RigidBoxComponent *;

  RigidBoxComponent(const Model::pointer &, const RigidBox::pointer &);
  ~RigidBoxComponent() override = default;

  void update(float duration) override;

  void backwardUpdate(float duration);
  void setPause(bool pause);
  bool getPause() const;

  Model::pointer getModel() const;
  RigidBox::pointer getBox() const;

protected:
  Model::pointer _model;
  RigidBox::pointer _box;
  bool _pause = false;
};

class RigidBallComponent : public Component {
public:
  using pointer = std::shared_ptr<RigidBallComponent>;
  using raw_ptr = RigidBallComponent *;

  RigidBallComponent(const Model::pointer &, const RigidBall::pointer &);
  ~RigidBallComponent() override = default;

  void backwardUpdate(float duration);
  void update(float duration) override;
  void setPause(bool pause);
  bool getPause() const;

  Model::pointer getModel() const;
  RigidBall::pointer getBall() const;

protected:
  Model::pointer _model;
  RigidBall::pointer _ball;
  bool _pause = false;
};

}; // namespace ft

#endif // INCLUDE_INCLUDES_FT_COMPONENT_H_
