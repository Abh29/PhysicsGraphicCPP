#include "../includes/ft_camera.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/trigonometric.hpp>

ft::Camera::Camera(float fov, float aspect, float nearZ, float farZ,
                   glm::vec3 eye, glm::vec3 direction, glm::vec3 up)
    : _eyePosition(eye), _targetPosition(direction), _upDirection(up),
      _fov(fov), _aspect(aspect), _nearZ(nearZ), _farZ(farZ) {
  _frontVec = glm::normalize(_targetPosition - _eyePosition);
  _rightVec = glm::normalize(glm::cross(_frontVec, _upDirection));
  _cameraUp = glm::normalize(glm::cross(_frontVec, _rightVec));
  _viewMatrix = glm::lookAt(_eyePosition, _targetPosition, _upDirection);
  _projMatrix = glm::perspective(glm::radians(fov), aspect, nearZ, farZ);
}

glm::mat4 &ft::Camera::getProjMatrix() { return _projMatrix; }

const glm::mat4 &ft::Camera::getViewMatrix() const { return _viewMatrix; }

glm::mat4 &ft::Camera::getViewMatrix() { return _viewMatrix; }

const glm::mat4 &ft::Camera::getProjMatrix() const { return _projMatrix; }

glm::vec3 &ft::Camera::getEyePosition() { return _eyePosition; }

glm::vec3 &ft::Camera::getTargetPosition() { return _targetPosition; }
glm::vec3 &ft::Camera::getUpDirection() { return _upDirection; }
glm::vec3 &ft::Camera::getFrontVec() { return _frontVec; }
glm::vec3 &ft::Camera::getRightVec() { return _rightVec; }
glm::vec3 &ft::Camera::getCameraUp() { return _cameraUp; }
float ft::Camera::getFov() const { return _fov; }
float ft::Camera::getNearZ() const { return _nearZ; }
float ft::Camera::getFarZ() const { return _farZ; }
float ft::Camera::getAspect() const { return _aspect; }

void ft::Camera::vRotate(float deg) {
  auto m = glm::rotate(glm::mat4(1.0f), glm::radians(deg), _rightVec);
  _frontVec = glm::vec3(m * glm::vec4(_frontVec, 1.0f));
  _cameraUp = glm::vec3(m * glm::vec4(_cameraUp, 1.0f));
  _targetPosition = _eyePosition + _frontVec;
  _viewMatrix = glm::lookAt(_eyePosition, _targetPosition, _upDirection);
}

void ft::Camera::hRotate(float deg) {
  auto m = glm::rotate(glm::mat4(1.0f), glm::radians(deg), _cameraUp);
  _frontVec = glm::vec3(m * glm::vec4(_frontVec, 1.0f));
  _rightVec = glm::vec3(m * glm::vec4(_rightVec, 1.0f));
  _targetPosition = _eyePosition + _frontVec;
  _viewMatrix = glm::lookAt(_eyePosition, _targetPosition, _upDirection);
}

void ft::Camera::rotate(float deg, glm::vec3 v) {
  auto rM = glm::mat3(glm::rotate(glm::mat4(1.0f), glm::radians(deg), v));
  _targetPosition = rM * _targetPosition;
  _viewMatrix = glm::lookAt(_eyePosition, _targetPosition, _upDirection);
}

void ft::Camera::translate(glm::vec3 v) {
  _eyePosition += v;
  _viewMatrix = glm::lookAt(_eyePosition, _targetPosition, _upDirection);
}

void ft::Camera::forward(float step) {
  _eyePosition += (_frontVec)*step;
  _targetPosition = _eyePosition + _frontVec;
  _viewMatrix = glm::lookAt(_eyePosition, _targetPosition, _upDirection);
}

void ft::Camera::translateSide(float step) {
  _eyePosition += (_rightVec)*step;
  _targetPosition = _eyePosition + _frontVec;
  _viewMatrix = glm::lookAt(_eyePosition, _targetPosition, _upDirection);
}

void ft::Camera::translateUp(float step) {
  _eyePosition += (_cameraUp)*step;
  _targetPosition = _eyePosition + _frontVec;
  _viewMatrix = glm::lookAt(_eyePosition, _targetPosition, _upDirection);
}

void ft::Camera::hardSet(glm::vec3 eye, glm::vec3 target, glm::vec3 up) {
  _eyePosition = eye;
  _targetPosition = target;
  _upDirection = up;
  _frontVec = glm::normalize(_targetPosition - _eyePosition);
  _rightVec = glm::normalize(glm::cross(_frontVec, _upDirection));
  _cameraUp = glm::normalize(glm::cross(_frontVec, _rightVec));
  _viewMatrix = glm::lookAt(_eyePosition, _targetPosition, _upDirection);
}

void ft::Camera::setFov(float fov) {
  _fov = fov;
  _projMatrix = glm::perspective(glm::radians(_fov), _aspect, _nearZ, _farZ);
}

void ft::Camera::rotateWorldX(float deg) {
  glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(deg), {1, 0, 0});
  _eyePosition = rot * glm::vec4(_eyePosition, 1.0f);
  _upDirection = rot * glm::vec4(_upDirection, 1.0f);
  _targetPosition = rot * glm::vec4(_targetPosition, 1.0f);
  _frontVec = glm::normalize(_targetPosition - _eyePosition);
  _rightVec = glm::normalize(glm::cross(_frontVec, _upDirection));
  _cameraUp = glm::normalize(glm::cross(_frontVec, _rightVec));
  _viewMatrix = glm::lookAt(_eyePosition, _targetPosition, _upDirection);
  //_viewMatrix = glm::rotate(_viewMatrix, glm::radians(deg), {1, 0, 0});
  // _eyePosition = {_viewMatrix[0][3], _viewMatrix[1][3], _viewMatrix[2][3]};
}

void ft::Camera::rotateWorldY(float deg) {
  // _viewMatrix = glm::rotate(_viewMatrix, glm::radians(deg), {0, 1, 0});
  glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(deg), {0, 1, 0});
  _eyePosition = rot * glm::vec4(_eyePosition, 1.0f);
  _upDirection = rot * glm::vec4(_upDirection, 1.0f);
  _targetPosition = rot * glm::vec4(_targetPosition, 1.0f);
  _frontVec = glm::normalize(_targetPosition - _eyePosition);
  _rightVec = glm::normalize(glm::cross(_frontVec, _upDirection));
  _cameraUp = glm::normalize(glm::cross(_frontVec, _rightVec));
  _viewMatrix = glm::lookAt(_eyePosition, _targetPosition, _upDirection);
}

void ft::Camera::rotateWorldZ(float deg) {
  glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(deg), {0, 0, 1});
  _eyePosition = rot * glm::vec4(_eyePosition, 1.0f);
  _upDirection = rot * glm::vec4(_upDirection, 1.0f);
  _targetPosition = rot * glm::vec4(_targetPosition, 1.0f);
  _frontVec = glm::normalize(_targetPosition - _eyePosition);
  _rightVec = glm::normalize(glm::cross(_frontVec, _upDirection));
  _cameraUp = glm::normalize(glm::cross(_frontVec, _rightVec));
  _viewMatrix = glm::lookAt(_eyePosition, _targetPosition, _upDirection);
  // _viewMatrix *= rot;
  //  _viewMatrix = glm::rotate(_viewMatrix, glm::radians(deg), {0, 0, 1});
}

void ft::Camera::updateAspect(float aspect) {
  _aspect = aspect;
  _projMatrix = glm::perspective(glm::radians(_fov), _aspect, _nearZ, _farZ);
}

/**********************************CameraBuilder*******************************/

ft::CameraBuilder &ft::CameraBuilder::setFOV(float fov) {
  _fov = fov;
  return *this;
}

ft::CameraBuilder &ft::CameraBuilder::setAspect(float aspect) {
  _aspect = aspect;
  return *this;
}

ft::CameraBuilder &ft::CameraBuilder::setZNearFar(float zNear, float zFar) {
  _nearZ = zNear;
  _farZ = zFar;
  return *this;
}

ft::CameraBuilder &ft::CameraBuilder::setEyePosition(glm::vec3 v) {
  _eyePosition = v;
  return *this;
}

ft::CameraBuilder &ft::CameraBuilder::setTarget(glm::vec3 v) {
  _target = v;
  return *this;
}

ft::CameraBuilder &ft::CameraBuilder::setUpDirection(glm::vec3 v) {
  _upDirection = v;
  return *this;
}

ft::Camera::pointer ft::CameraBuilder::build() {
  return std::make_shared<Camera>(_fov, _aspect, _nearZ, _farZ, _eyePosition,
                                  _target, _upDirection);
}
