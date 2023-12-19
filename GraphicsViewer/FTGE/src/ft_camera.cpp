#include "../include.h"
#include "../includes/ft_camera.h"


ft::Camera::Camera(float fov, float aspect, float nearZ, float farZ, glm::vec3 eye, glm::vec3 direction, glm::vec3 up) :
_eyePosition(eye), _targetPosition(direction), _upDirection(up),
_fov(fov), _aspect(aspect), _nearZ(nearZ), _farZ(farZ)
{
	_frontVec = glm::normalize(_targetPosition - _eyePosition);
	_rightVec = glm::normalize(glm::cross(_frontVec, _upDirection));
	_cameraUp = glm::normalize(glm::cross(_frontVec, _rightVec));
	_viewMatrix = glm::lookAt(_eyePosition, _targetPosition, _upDirection);
	_projMatrix = glm::perspective(fov, aspect, nearZ, farZ);
}

glm::mat4 &ft::Camera::getProjMatrix() { return _projMatrix;}

const glm::mat4 &ft::Camera::getViewMatrix() const { return _viewMatrix;}

glm::mat4 &ft::Camera::getViewMatrix() { return _viewMatrix;}

const glm::mat4 &ft::Camera::getProjMatrix() const { return _projMatrix;}

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
	_eyePosition += (_frontVec) * step;
	_targetPosition = _eyePosition + _frontVec;
	_viewMatrix = glm::lookAt(_eyePosition, _targetPosition, _upDirection);
}

void ft::Camera::translateSide(float step) {
	_eyePosition += (_rightVec) * step;
	_targetPosition = _eyePosition + _frontVec;
	_viewMatrix = glm::lookAt(_eyePosition, _targetPosition, _upDirection);
}

void ft::Camera::translateUp(float step) {
	_eyePosition += (_cameraUp) * step;
	_targetPosition = _eyePosition + _frontVec;
	_viewMatrix = glm::lookAt(_eyePosition, _targetPosition, _upDirection);
}

void ft::Camera::hardSet(glm::vec3 eye, glm::vec3 target, glm::vec3 up) {
	_eyePosition = eye;
	_targetPosition = target;
	_upDirection = up;
	_viewMatrix = glm::lookAt(_eyePosition, _targetPosition, _upDirection);
}

void ft::Camera::rotateWorldX(float deg) {
	_viewMatrix = glm::rotate(_viewMatrix, glm::radians(deg), {1,0,0});
}

void ft::Camera::rotateWorldY(float deg) {
	_viewMatrix = glm::rotate(_viewMatrix, glm::radians(deg), {0,1,0});
}

void ft::Camera::rotateWorldZ(float deg) {
	_viewMatrix = glm::rotate(_viewMatrix, glm::radians(deg), {0,0,1});
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
	return std::make_shared<Camera>(_fov, _aspect, _nearZ, _farZ,
									_eyePosition, _target, _upDirection);
}