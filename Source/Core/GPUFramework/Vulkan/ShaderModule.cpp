//
//  ShaderModule.cpp
//  ToyEngine
//
//  Created by Li Han on 9/25/24.
//

#include "ShaderModule.hpp"

#include <sstream>
#include <fstream>
#include <string>
#include <fmt/format.h>

#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>

EShLanguage translateShaderStage(vk::ShaderStageFlagBits stage)
{
    switch (stage)
    {
    case vk::ShaderStageFlagBits::eVertex: return EShLangVertex;
    case vk::ShaderStageFlagBits::eTessellationControl: return EShLangTessControl;
    case vk::ShaderStageFlagBits::eTessellationEvaluation: return EShLangTessEvaluation;
    case vk::ShaderStageFlagBits::eGeometry: return EShLangGeometry;
    case vk::ShaderStageFlagBits::eFragment: return EShLangFragment;
    case vk::ShaderStageFlagBits::eCompute: return EShLangCompute;
    case vk::ShaderStageFlagBits::eRaygenNV: return EShLangRayGenNV;
    case vk::ShaderStageFlagBits::eAnyHitNV: return EShLangAnyHitNV;
    case vk::ShaderStageFlagBits::eClosestHitNV: return EShLangClosestHitNV;
    case vk::ShaderStageFlagBits::eMissNV: return EShLangMissNV;
    case vk::ShaderStageFlagBits::eIntersectionNV: return EShLangIntersectNV;
    case vk::ShaderStageFlagBits::eCallableNV: return EShLangCallableNV;
    case vk::ShaderStageFlagBits::eTaskNV: return EShLangTaskNV;
    case vk::ShaderStageFlagBits::eMeshNV: return EShLangMeshNV;
    default: assert(false && "Unknown shader stage"); return EShLangVertex;
    }
}

bool GLSLtoSPV(const vk::ShaderStageFlagBits shaderType, std::string const& glslShader, std::vector<unsigned int>& spvShader)
{
    EShLanguage stage = translateShaderStage(shaderType);

    const char* shaderStrings[1];
    shaderStrings[0] = glslShader.data();

    glslang::TShader shader(stage);
    shader.setStrings(shaderStrings, 1);

    // Enable SPIR-V and Vulkan rules when parsing GLSL
    EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

    if (!shader.parse(GetDefaultResources(), 100, false, messages))
    {
        puts(shader.getInfoLog());
        puts(shader.getInfoDebugLog());
        return false;  // something didn't work
    }

    glslang::TProgram program;
    program.addShader(&shader);

    //
    // Program-level processing...
    //

    if (!program.link(messages))
    {
        puts(shader.getInfoLog());
        puts(shader.getInfoDebugLog());
        fflush(stdout);
        return false;
    }

    glslang::GlslangToSpv(*program.getIntermediate(stage), spvShader);
    return true;
}

vk::ShaderModule createShaderModule(vk::Device const& device, vk::ShaderStageFlagBits shaderStage, std::string const& shaderText)
{
    std::vector<unsigned int> shaderSPV;
    if (!GLSLtoSPV(shaderStage, shaderText, shaderSPV))
    {
        throw std::runtime_error("Could not convert glsl shader to spir-v -> terminating");
    }

    return device.createShaderModule(vk::ShaderModuleCreateInfo(vk::ShaderModuleCreateFlags(), shaderSPV));
}

ShaderModule::ShaderModule(Device& device, vk::ShaderStageFlagBits stage, const std::string& content, const std::string& entryPoint) :
    device{ device },
    stage{ stage },
    content{ content },
    entryPoint{ entryPoint }
{
    debugName = fmt::format("{} [variant {:X}] [entrypoint {}]",
                             fileName, id, entryPoint);

    if (entryPoint.empty())
    {
        throw VulkanException{VK_ERROR_INITIALIZATION_FAILED};
    }

    if (content.empty())
    {
        throw VulkanException{VK_ERROR_INITIALIZATION_FAILED};
    }

    std::hash<std::string> hasher{};
    id = hasher(std::string{reinterpret_cast<const char *>(spirv.data()),
                            reinterpret_cast<const char *>(spirv.data() + spirv.size())});
}

size_t ShaderModule::get_id() const
{
    return id;
}

vk::ShaderStageFlagBits ShaderModule::getStage() const
{
    return stage;
}

const std::string &ShaderModule::getEntryPoint() const
{
    return entryPoint;
}

const std::string &ShaderModule::getInfoLog() const
{
    return infoLog;
}

const std::vector<uint32_t> &ShaderModule::getBinary() const
{
    return spirv;
}
