#pragma once

#include "Common/UUID.hpp"
#include "Core/GPUFramework/Vulkan/VkCommon.hpp"
#include "Core/GPUFramework/Vulkan/GraphicsPipeline.hpp"

#include <vector>
#include <array>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <map>

class GraphicsPass;
class MaterialInstance;

enum class PropertyType {
	TEXTURE,
	FLOAT,
	VEC2,
	VEC3,
	VEC4
};

struct Property {
	uint32_t bindingIndex;
	PropertyType propertyType;
};

struct MaterialConfig {
	vk::PolygonMode polygonMode = vk::PolygonMode::eFill;
	vk::CullModeFlags cullMode = vk::CullModeFlagBits::eBack;
	vk::CompareOp compareOp = vk::CompareOp::eGreaterOrEqual;
	vk::Bool32 depthWrite = true;
	vk::Bool32 depthTest = true;
};

class Material {
public:
	Material(std::string, MaterialConfig& config);
	~Material();

	void setShader(std::array<std::string, 2>);
	void setProperty(std::string, Property);

	void addPass(uint32_t, GraphicsPass*);

	std::vector<GraphicsPass*>& getPasses() {
		return passes;
	}

protected:
	uuid id;
	std::string name;
	MaterialConfig config;
	std::array<std::string, 2> shaders;
	std::unordered_map<std::string, Property> properties;
	std::vector<GraphicsPass*> passes;
	std::vector<MaterialInstance*> instances;
};
