#pragma once

#include "VkCommon.hpp"

class Device;

class Image {
public:
	Image() = delete;
	Image(const Image&) = delete;
	Image(Image&&) = delete;

	Image(vk::Image);


protected:
	Device& device;

	vk::Image handle;
};