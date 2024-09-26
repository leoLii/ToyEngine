//
//  SpirvReflection.hpp
//  ToyEngine
//
//  Created by Li Han on 9/25/24.
//

#pragma once

#include "VkCommon.hpp"

#include <spirv_cross/spirv_glsl.hpp>
#include <vector>

class ShaderResource;
class ShaderVariant;

class SPIRVReflection
{
  public:
    /// @brief Reflects shader resources from SPIRV code
    /// @param stage The Vulkan shader stage flag
    /// @param spirv The SPIRV code of shader
    /// @param[out] resources The list of reflected shader resources
    /// @param variant ShaderVariant used for reflection to specify the size of the runtime arrays in Storage Buffers
    bool reflect_shader_resources(VkShaderStageFlagBits        stage,
                                  const std::vector<uint32_t> &spirv,
                                  std::vector<ShaderResource> &resources,
                                  const ShaderVariant         &variant);

  private:
    void parse_shader_resources(const spirv_cross::Compiler &compiler,
                                VkShaderStageFlagBits        stage,
                                std::vector<ShaderResource> &resources,
                                const ShaderVariant         &variant);

    void parse_push_constants(const spirv_cross::Compiler &compiler,
                              VkShaderStageFlagBits        stage,
                              std::vector<ShaderResource> &resources,
                              const ShaderVariant         &variant);

    void parse_specialization_constants(const spirv_cross::Compiler &compiler,
                                        VkShaderStageFlagBits        stage,
                                        std::vector<ShaderResource> &resources,
                                        const ShaderVariant         &variant);
};
