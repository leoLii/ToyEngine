//
//  ShaderModule.hpp
//  ToyEngine
//
//  Created by Li Han on 9/25/24.
//

#pragma once

#include "VkCommon.hpp"

#include <unordered_map>
#include <String>

class Device;

class ShaderModule
{
  public:
    ShaderModule(Device& device, vk::ShaderStageFlagBits stage, const std::string& content, const std::string& entry_point);

    ShaderModule(const ShaderModule &) = delete;

    ShaderModule(ShaderModule &&other);

    ShaderModule &operator=(const ShaderModule &) = delete;

    ShaderModule &operator=(ShaderModule &&) = delete;

    size_t get_id() const;

    vk::ShaderStageFlagBits getStage() const;

    const std::string &getEntryPoint() const;

    const std::string &getInfoLog() const;

    const std::vector<uint32_t> &getBinary() const;

    inline const std::string &getDebugName() const
    {
        return debugName;
    }

    inline void setDebugName(const std::string &name)
    {
        debugName = name;
    }

  private:
    Device &device;

    /// Shader unique id
    size_t id;

    /// Stage of the shader (vertex, fragment, etc)
    vk::ShaderStageFlagBits stage;

    /// Name of the main function
    std::string entryPoint;

    /// Human-readable name for the shader
    std::string debugName;

    std::string fileName;

    std::string content;

    /// Compiled source
    std::vector<uint32_t> spirv;

    std::string infoLog;
};
