#pragma once

#include "Common/Math.hpp"

#include "Scene/Component.hpp"

#include <vector>

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
	const Mat4 getViewMatrixPrev() const;
	const Mat4 getProjectionMatrix() const;
	const Mat4 getProjectionMatrixJittered() const;

	const Mat4 getPV() const;
	const Mat4 getPVJittered() const;
	const Mat4 getPVPrev() const;

	Vec2 getPrevJitter() const;
	Vec2 getCurrJitter() const;

	virtual void update(float deltaTime, uint32_t frameIndex) override;

	virtual void lateUpdate() override;

protected:
	CameraType type;
	Frustum frustum;
	Vec2 prevJitter;
	Vec2 currJitter;
	Mat4 view{ 1.0 };
	Mat4 projection{ 1.0 };
	Mat4 prevView{ 1.0 };
	Mat4 projectionJittered{ 1.0 };
	Mat4 viewProjection{ 1.0 };

	std::vector<Vec2> jitterSamples;

	uint32_t taaSamples = 16;

private:
	void projectionJitter(uint32_t);
	void generateTAAJitterSamples();
};