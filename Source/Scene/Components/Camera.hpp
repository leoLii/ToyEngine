#pragma once

#include "Common/Math.hpp"

#include "Scene/Component.hpp"

enum CameraType {
	Perspect,
	Ortho
};

struct Frustum {
	float fov;
	float width;
	float height;
	float zNear;
	float zFar;
};

class Camera : public Component {
public:
	Camera(CameraType, Frustum);
	virtual ~Camera() = default;
	virtual std::type_index getType() override;

	void lookAt(Vec3, Vec3, Vec3);

	const Mat4 getViewMatrix() const;
	const Mat4 getProjectionMatrix() const;
	const Mat4 getProjectionMatrixJittered() const;

	virtual void update(float deltaTime) override;

protected:
	CameraType type;
	Frustum frustum;
	Mat4 view{ 1.0 };
	Mat4 projection{ 1.0 };
	Mat4 projectionJittered{ 1.0 };
	Mat4 viewProjection{ 1.0 };
	float jitterX = 0.0;
	float jitterY = 0.0;
};