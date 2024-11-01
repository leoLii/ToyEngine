#pragma once

#include "Common/Math.hpp"
#include "Scene/Component.hpp"

#include <typeindex>

class Transform :public Component {
public:
	Transform();
	virtual ~Transform() = default;

	virtual void update(float deltaTime, uint32_t frameIndex) override;

	virtual void lateUpdate() override;

	virtual std::type_index getType() override;

	void setTranslate(const Vec3&);
	void setRotate(const float, const Vec3&);
	void setScale(const Vec3&);

	void setTransform(Mat4&);

	Mat4 getCurrMatrix() const;
	Mat4 getPrevMatrix() const;

protected:
	Vec3 translation = Vec3(0.0, 0.0, 0.0);
	Vec3 scale = Vec3(1.0, 1.0, 1.0);
	Quat rotation = Quat(1.0, 0.0, 0.0, 0.0);
	Mat4 currTransform{ 1.0 };
	Mat4 prevTransform{ 1.0 };
};