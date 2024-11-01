#include "Transform.hpp"

Transform::Transform()
{
}

void Transform::update(float deltaTime, uint32_t frameIndex)
{
	currTransform = glm::translate(Mat4(1.0), translation) *
		glm::mat4_cast(rotation) *
		glm::scale(Mat4(1.0), scale);
}

void Transform::lateUpdate()
{
	prevTransform = currTransform;
}

std::type_index Transform::getType()
{
	return typeid(Transform);
}

void Transform::setTranslate(const Vec3& direction)
{
	this->translation += direction;
}

void Transform::setRotate(const float angle, const Vec3& axis)
{
	Quat q = glm::angleAxis(glm::radians(angle), axis);
	this->rotation = q * this->rotation;
}

void Transform::setScale(const Vec3& scale)
{
	this->scale *= scale;
}

void Transform::setTransform(Mat4& matrix)
{
	currTransform *= matrix;
}

Mat4 Transform::getCurrMatrix() const
{
	return currTransform;
}
