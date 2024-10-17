#include "Camera.hpp"

Camera::Camera(CameraType type, Frustum frustum)
	:type{ type }
	, frustum{ frustum }
{
	switch (type)
	{
	case Perspect:
		projection = glm::perspective(glm::radians(frustum.fov), frustum.width / frustum.height, frustum.zNear, frustum.zFar);
		projectionJittered = projection;
		break;
	case Ortho:
		projection = glm::ortho(-frustum.width / 2.0, frustum.width / 2.0, -frustum.height / 2.0, frustum.height / 2.0);
		projectionJittered = projection;
		break;
	default:
		break;
	}
}

std::type_index Camera::getType()
{
	return typeid(Camera);
}

void Camera::lookAt(Vec3 eye, Vec3 center, Vec3 up)
{
	this->view = glm::lookAt(eye, center, up);
}

const Mat4 Camera::getViewMatrix() const
{
	return view;
}

const Mat4 Camera::getProjectionMatrix() const
{
	return projection;
}

const Mat4 Camera::getProjectionMatrixJittered() const
{
	return projectionJittered;
}

void Camera::update(float deltaTime)
{
}
