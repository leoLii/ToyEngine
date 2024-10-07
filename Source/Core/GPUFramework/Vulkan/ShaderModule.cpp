//
//  ShaderModule.cpp
//  ToyEngine
//
//  Created by Li Han on 9/25/24.
//

#include "ShaderModule.hpp"
#include "Device.hpp"

#include "Common/Logging.hpp"

#include <sstream>
#include <fstream>
#include <string>

#include <shaderc/shaderc.hpp>

shaderc_shader_kind translateShaderStage(vk::ShaderStageFlagBits stage)
{
	switch (stage)
	{
		case vk::ShaderStageFlagBits::eVertex: return shaderc_glsl_vertex_shader;
		case vk::ShaderStageFlagBits::eTessellationControl: return shaderc_glsl_tess_control_shader;
		case vk::ShaderStageFlagBits::eTessellationEvaluation: return shaderc_glsl_tess_evaluation_shader;
		case vk::ShaderStageFlagBits::eGeometry: return shaderc_glsl_geometry_shader;
		case vk::ShaderStageFlagBits::eFragment: return shaderc_glsl_fragment_shader;
		case vk::ShaderStageFlagBits::eCompute: return shaderc_glsl_compute_shader;
		case vk::ShaderStageFlagBits::eRaygenNV: return shaderc_glsl_raygen_shader;
		case vk::ShaderStageFlagBits::eAnyHitNV: return shaderc_glsl_anyhit_shader;
		case vk::ShaderStageFlagBits::eClosestHitNV: return shaderc_glsl_closesthit_shader;
		case vk::ShaderStageFlagBits::eMissNV: return shaderc_glsl_miss_shader;
		case vk::ShaderStageFlagBits::eIntersectionNV: return shaderc_glsl_intersection_shader;
		case vk::ShaderStageFlagBits::eCallableNV: return shaderc_glsl_callable_shader;
		case vk::ShaderStageFlagBits::eTaskNV: return shaderc_glsl_task_shader;
		case vk::ShaderStageFlagBits::eMeshNV: return shaderc_glsl_mesh_shader;
		default: assert(false && "Unknown shader stage"); return shaderc_glsl_infer_from_source;
	}
}

bool GLSLtoSPV(const vk::ShaderStageFlagBits shaderType, std::string const& glslShader, std::vector<uint32_t>& spvShader)
{
	shaderc::Compiler compiler;
	shaderc::CompileOptions options;

	// Enable optimization for performance
	options.SetOptimizationLevel(shaderc_optimization_level_performance);

	shaderc_shader_kind kind = translateShaderStage(shaderType);

	shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(glslShader, kind, "shader");

	if (result.GetCompilationStatus() != shaderc_compilation_status_success)
	{
		LOGE("Shader compilation failed: {}\n", result.GetErrorMessage());
		return false;
	}

	spvShader.assign(result.cbegin(), result.cend());
	return true;
}

ShaderModule::ShaderModule(const Device& device, vk::ShaderStageFlagBits stage, const std::string& content, const std::string& entryPoint) :
	device{ device },
	stage{ stage },
	content{ content },
	entryPoint{ entryPoint }
{
	debugName = fmt::format("{} [variant {:X}] [entrypoint {}]", fileName, id, entryPoint);

	if (entryPoint.empty())
	{
		throw VulkanException{ vk::Result::eErrorInitializationFailed };
	}

	if (content.empty())
	{
		throw VulkanException{ vk::Result::eErrorInitializationFailed };
	}

	if (!GLSLtoSPV(stage, content, spirv))
	{
		throw std::runtime_error("Could not convert GLSL shader to SPIR-V -> terminating");
	}

	vk::ShaderModuleCreateInfo shaderInfo;
	shaderInfo.codeSize = spirv.size()*4;
	shaderInfo.pCode = spirv.data();

	handle = device.getHandle().createShaderModule(shaderInfo);

	std::hash<std::string> hasher{};
	id = hasher(std::string{ reinterpret_cast<const char*>(spirv.data()), reinterpret_cast<const char*>(spirv.data() + spirv.size()) });
}

ShaderModule::ShaderModule(const Device& device, vk::ShaderStageFlagBits stage, const std::vector<uint32_t>& binary, const std::string& entryPoint) :
	device{ device },
	stage{ stage },
	spirv{ binary },
	entryPoint{ entryPoint }
{
	debugName = fmt::format("{} [variant {:X}] [entrypoint {}]", fileName, id, entryPoint);

	if (entryPoint.empty())
	{
		throw VulkanException{ vk::Result::eErrorInitializationFailed };
	}

	vk::ShaderModuleCreateInfo shaderInfo;
	shaderInfo.codeSize = spirv.size()*4;
	shaderInfo.pCode = spirv.data();

	handle = device.getHandle().createShaderModule(shaderInfo);

	std::hash<std::string> hasher{};
	id = hasher(std::string{ reinterpret_cast<const char*>(spirv.data()), reinterpret_cast<const char*>(spirv.data() + spirv.size()) });
}

ShaderModule::~ShaderModule() {
	device.getHandle().destroyShaderModule(handle);
}

size_t ShaderModule::getID() const
{
	return id;
}

vk::ShaderStageFlagBits ShaderModule::getStage() const
{
	return stage;
}

const std::string& ShaderModule::getEntryPoint() const
{
	return entryPoint;
}

const std::string& ShaderModule::getInfoLog() const
{
	return infoLog;
}

const std::vector<uint32_t>& ShaderModule::getBinary() const
{
	return spirv;
}
