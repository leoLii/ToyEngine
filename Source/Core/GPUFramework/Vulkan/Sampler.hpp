#pragma once

#include "VkCommon.hpp"

class Device;

class Sampler {
public:
	Sampler(const Device&);
	~Sampler();

	vk::Sampler getHandle() const;

private:
	const Device& device;

	vk::Sampler handle;
};