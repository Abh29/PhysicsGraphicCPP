#ifndef FTGRAPHICS_FT_CAMERA_H
#define FTGRAPHICS_FT_CAMERA_H

#include "ft_defines.h"
#include "ft_headers.h"
#include <glm/fwd.hpp>

namespace ft {

class Camera {

public:
  using pointer = std::shared_ptr<Camera>;
  Camera(float fov, float aspect, float nearZ, float farZ, glm::vec3 eye,
         glm::vec3 target, glm::vec3 up);
  ~Camera() = default;

  [[nodiscard]] const glm::mat4 &getViewMatrix() const;
  glm::mat4 &getViewMatrix();
  [[nodiscard]] const glm::mat4 &getProjMatrix() const;
  glm::mat4 &getProjMatrix();
  glm::vec3 &getEyePosition();
  glm::vec3 &getTargetPosition();
  glm::vec3 &getUpDirection();
  glm::vec3 &getFrontVec();
  glm::vec3 &getRightVec();
  glm::vec3 &getCameraUp();
  float getFov() const;
  float getNearZ() const;
  float getFarZ() const;
  float getAspect() const;

  void hardSet(glm::vec3 eye, glm::vec3 target, glm::vec3 up);
  void updateAspect(float aspect);

  void hRotate(float deg);
  void vRotate(float deg);
  void rotate(float deg, glm::vec3 v);
  void rotateWorldX(float deg);
  void rotateWorldY(float deg);
  void rotateWorldZ(float deg);
  void forward(float step);
  void translateUp(float step);
  void translateSide(float step);
  void translate(glm::vec3 v);

private:
  glm::vec3 _eyePosition;
  glm::vec3 _targetPosition;
  glm::vec3 _upDirection;
  glm::vec3 _frontVec;
  glm::vec3 _rightVec;
  glm::vec3 _cameraUp;
  float _fov;
  float _aspect;
  float _nearZ;
  float _farZ;
  glm::mat4 _viewMatrix;
  glm::mat4 _projMatrix;
};

class CameraBuilder {
public:
  CameraBuilder() = default;
  ~CameraBuilder() = default;

  CameraBuilder &setFOV(float fov);
  CameraBuilder &setAspect(float aspect);
  CameraBuilder &setZNearFar(float zNear, float zFar);
  CameraBuilder &setEyePosition(glm::vec3 v);
  CameraBuilder &setTarget(glm::vec3 v);
  CameraBuilder &setUpDirection(glm::vec3 v);
  Camera::pointer build();

private:
  glm::vec3 _eyePosition = {0, 0, 0};
  glm::vec3 _target = {1, 0, 0};
  glm::vec3 _upDirection = {0, 0, 1};
  float _fov = 90;
  float _aspect = 1.0f;
  float _nearZ = 1.0f;
  float _farZ = 10.0f;
};

} // namespace ft

#endif // FTGRAPHICS_FT_CAMERA_H
