#include "../includes/ft_component.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

/********************************RigidBoxComponent*******************************/

ft::RigidBoxComponent::RigidBoxComponent(const Model::pointer &model,
                                         const RigidBox::pointer &box)
    : Component(), _model(model), _box(box) {}

void ft::RigidBoxComponent::update(float duration) {
  (void)duration;
  if (_pause || !_box->isUpdated())
    return;

  auto translation = glm::translate(glm::mat4(1.0f), _box->body->getPosition());
  auto rotation = glm::mat4_cast(_box->body->getOrientation());

  _model->translate(translation).rotate(rotation);
}

void ft::RigidBoxComponent::backwardUpdate(float duration) {
  (void)duration;

  _box->setState(_model->getCentroid(),
                 glm::quat_cast(_model->getState().rotation), _box->halfSize,
                 {0.0f, 1.0f, 0.0f});
}

void ft::RigidBoxComponent::setPause(bool pause) { _pause = pause; }

bool ft::RigidBoxComponent::getPause() const { return _pause; }

ft::Model::pointer ft::RigidBoxComponent::getModel() const { return _model; }

ft::RigidBox::pointer ft::RigidBoxComponent::getBox() const { return _box; }

/********************************RigidBallComponent*******************************/

ft::RigidBallComponent::RigidBallComponent(const Model::pointer &model,
                                           const RigidBall::pointer &ball)
    : Component(), _model(model), _ball(ball) {}

void ft::RigidBallComponent::update(float duration) {
  (void)duration;
  if (_pause || !_ball->isUpdated())
    return;

  auto translation =
      glm::translate(glm::mat4(1.0f), _ball->body->getPosition());
  auto rotation = glm::mat4_cast(_ball->body->getOrientation());

  _model->translate(translation).rotate(rotation);
}

void ft::RigidBallComponent::backwardUpdate(float duration) {
  (void)duration;

  _ball->setState(_model->getCentroid(),
                  glm::quat_cast(_model->getState().rotation), _ball->radius,
                  {0.0f, 1.0f, 0.0f});
}

void ft::RigidBallComponent::setPause(bool pause) { _pause = pause; }

bool ft::RigidBallComponent::getPause() const { return _pause; }

ft::Model::pointer ft::RigidBallComponent::getModel() const { return _model; }

ft::RigidBall::pointer ft::RigidBallComponent::getBall() const { return _ball; }
