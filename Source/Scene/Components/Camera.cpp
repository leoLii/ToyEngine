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

	generateTAAJitterSamples();
}

std::type_index Camera::getType()
{
	return typeid(Camera);
}

void Camera::lookAt(Vec3 eye, Vec3 center, Vec3 up)
{
	this->view = glm::lookAtRH(eye, center, up);
}

const Mat4 Camera::getViewMatrix() const
{
	return view;
}

const Mat4 Camera::getViewMatrixPrev() const
{
	return prevView;
}

const Mat4 Camera::getProjectionMatrix() const
{
	return projection;
}

const Mat4 Camera::getProjectionMatrixJittered() const
{
	return projectionJittered;
}

const Mat4 Camera::getPV() const
{
	return projection * view;
}

const Mat4 Camera::getPVJittered() const
{
	return projectionJittered * view;
}

const Mat4 Camera::getPVPrev() const
{
	return projection * prevView;
}

Vec2 Camera::getPrevJitter() const
{
	return prevJitter;
}

Vec2 Camera::getCurrJitter() const
{
	return currJitter;
}

void Camera::update(float deltaTime, uint32_t frameIndex)
{
	prevView = view;
	projectionJittered = projection;
	prevJitter = currJitter;

	//updateview
	projectionJitter(frameIndex);
}

void Camera::lateUpdate()
{
}

void Camera::generateTAAJitterSamples() 
{
	auto HaltonSequence = [](int index, int base) -> float {
		float result = 0;
		float fraction = 1.0f / base;
		while (index > 0) {
			result += (index % base) * fraction;
			index /= base;
			fraction /= base;
		}
		return result;
		};
	for (int i = 0; i < 16; i++) {
		float jitterX = HaltonSequence(i + 1, 2) - 0.5f;
		float jitterY = HaltonSequence(i + 1, 3) - 0.5f;
		jitterSamples.push_back(glm::vec2(jitterX, jitterY));
	}
}

void Camera::projectionJitter(uint32_t frameIndex)
{
	uint32_t width = 960;
	uint32_t height = 540;
	currJitter = jitterSamples[frameIndex % 16] / Vec2(width, height);
	projectionJittered[2][0] += currJitter.x;// 对x方向应用抖动
	projectionJittered[2][1] += currJitter.y;  // 对y方向应用抖动
}
