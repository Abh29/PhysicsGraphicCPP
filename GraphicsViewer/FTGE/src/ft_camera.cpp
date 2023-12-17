#include "../include.h"
#include "../includes/ft_camera.h"


ft::Camera::Camera(float fov, float aspect, float nearZ, float farZ, glm::vec3 eye, glm::vec3 direction, glm::vec3 up) :
_eyePosition(eye), _targetPosition(direction), _upDirection(up),
_fov(fov), _aspect(aspect), _nearZ(nearZ), _farZ(farZ)
{
	_viewMatrix = glm::lookAt(_eyePosition, _targetPosition, _upDirection);
	_projMatrix = glm::perspective(fov, aspect, nearZ, farZ);
}

glm::mat4 &ft::Camera::getProjMatrix() { return _projMatrix;}

const glm::mat4 &ft::Camera::getViewMatrix() const { return _viewMatrix;}

glm::mat4 &ft::Camera::getViewMatrix() { return _viewMatrix;}

const glm::mat4 &ft::Camera::getProjMatrix() const { return _projMatrix;}

void ft::Camera::hRotate(float deg) {
	auto rM = glm::mat3(glm::rotate(glm::mat4(1.0f), glm::radians(deg), {0.0f, 1.0f, 0.0f}));
	std::cout << _eyePosition.x << " " << _eyePosition.y << " " << _eyePosition.z << std::endl;
	_eyePosition = rM * _eyePosition;
	std::cout << _eyePosition.x << " " << _eyePosition.y << " " << _eyePosition.z << std::endl;
	_viewMatrix = glm::lookAt(_eyePosition, _targetPosition, _upDirection);
}

void ft::Camera::vRotate(float deg) {
	auto rM = glm::mat3(glm::rotate(glm::mat4(1.0f), glm::radians(deg), {1.0f, 0.0f, 0.0f}));
	std::cout << _eyePosition.x << " " << _eyePosition.y << " " << _eyePosition.z << std::endl;
	_eyePosition = rM * _eyePosition;
	std::cout << _eyePosition.x << " " << _eyePosition.y << " " << _eyePosition.z << std::endl;
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
	_eyePosition += _targetPosition * step;
	_targetPosition += _targetPosition * step;
	_viewMatrix = glm::lookAt(_eyePosition, _targetPosition, _upDirection);
}

void ft::Camera::backward(float step) {
	_eyePosition += _targetPosition * -step;
	_targetPosition += _targetPosition * -step;
	_viewMatrix = glm::lookAt(_eyePosition, _targetPosition, _upDirection);
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