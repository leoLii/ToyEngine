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

struct ShaderReflection {

};

class ShaderModule
{
  public:
    ShaderModule(const Device&, vk::ShaderStageFlagBits, const std::string&, const std::string& entry_point = "main");

    ShaderModule(const Device&, vk::ShaderStageFlagBits, const std::vector<uint32_t>&, const std::string& entry_point = "main"); 

    ~ShaderModule();

    size_t getID() const;

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

    vk::ShaderModule getHandle() const {
        return this->handle;
    }

  private:
    const Device &device;

    vk::ShaderModule handle;

    /// Shader unique id
    size_t id;

    /// Stage of the shader (vertex, fragment, etc)
    vk::ShaderStageFlagBits stage;

    /// Name of the main function
    std::string entryPoint = "main";

    /// Human-readable name for the shader
    std::string debugName = "";

    std::string fileName = "";

    std::string content = "";

    /// Compiled source
    std::vector<uint32_t> spirv;

    std::string infoLog = "";
};
