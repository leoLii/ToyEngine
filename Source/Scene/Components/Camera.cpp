#include "Camera.hpp"

#include "Scene/Node.hpp"

#include <Common/Math.hpp>

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
	this->view = glm::lookAt(eye, center, up);
	direction = glm::normalize(center - eye);
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

void Camera::move(Vec2 input, float speed)
{
	Vec3 right = glm::normalize(glm::cross(direction, up));
	Vec3 forward = direction;
	Vec3 movement = (right * input.x + forward * -input.y) * speed;
	attachNode->getTransform().setTranslate(movement);
	lookAt(attachNode->getTransform().getPosition(), attachNode->getTransform().getPosition() + direction, up);
}

void Camera::rotate(Vec2 input, float speed)
{
	float yaw = input.x * speed;
	float pitch = input.y * speed;
	Vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction = glm::normalize(front);
	lookAt(attachNode->getTransform().getPosition(), attachNode->getTransform().getPosition() + direction, up);
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

std::array<Vec4, 6> Camera::getFrustumPlanes()
{
	std::array<Vec4, 6> planes;
	Mat4 clipMatrix = projection * view;

	// 左平面
	planes[0] = glm::vec4(
		clipMatrix[0][3] + clipMatrix[0][0],
		clipMatrix[1][3] + clipMatrix[1][0],
		clipMatrix[2][3] + clipMatrix[2][0],
		clipMatrix[3][3] + clipMatrix[3][0]
	);

	// 右平面
	planes[1] = glm::vec4(
		clipMatrix[0][3] - clipMatrix[0][0],
		clipMatrix[1][3] - clipMatrix[1][0],
		clipMatrix[2][3] - clipMatrix[2][0],
		clipMatrix[3][3] - clipMatrix[3][0]
	);

	// 下平面
	planes[2] = glm::vec4(
		clipMatrix[0][3] + clipMatrix[0][1],
		clipMatrix[1][3] + clipMatrix[1][1],
		clipMatrix[2][3] + clipMatrix[2][1],
		clipMatrix[3][3] + clipMatrix[3][1]
	);

	// 上平面
	planes[3] = glm::vec4(
		clipMatrix[0][3] - clipMatrix[0][1],
		clipMatrix[1][3] - clipMatrix[1][1],
		clipMatrix[2][3] - clipMatrix[2][1],
		clipMatrix[3][3] - clipMatrix[3][1]
	);

	// 近平面
	planes[4] = glm::vec4(
		clipMatrix[0][3] + clipMatrix[0][2],
		clipMatrix[1][3] + clipMatrix[1][2],
		clipMatrix[2][3] + clipMatrix[2][2],
		clipMatrix[3][3] + clipMatrix[3][2]
	);

	// 远平面
	planes[5] = glm::vec4(
		clipMatrix[0][3] - clipMatrix[0][2],
		clipMatrix[1][3] - clipMatrix[1][2],
		clipMatrix[2][3] - clipMatrix[2][2],
		clipMatrix[3][3] - clipMatrix[3][2]
	);

	// 归一化
	for (int i = 0; i < 6; i++) {
		float length = glm::length(glm::vec3(planes[i]));
		planes[i] /= length;
	}

	return planes;
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
	for (int i = 0; i < taaSamples; i++) {
		float jitterX = HaltonSequence(i + 1, 2) - 0.5f;
		float jitterY = HaltonSequence(i + 1, 3) - 0.5f;
		jitterSamples.push_back(glm::vec2(jitterX, jitterY));
	}
}

void Camera::projectionJitter(uint32_t frameIndex)
{
	uint32_t width = frustum.width;
	uint32_t height = frustum.height;
	currJitter = jitterSamples[frameIndex % taaSamples] / Vec2(width, height);
	projectionJittered[2][0] += currJitter.x;// 对x方向应用抖动
	projectionJittered[2][1] += currJitter.y;  // 对y方向应用抖动
}
