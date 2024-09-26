//
//  CommandPool.hpp
//  ToyEngine
//
//  Created by Li Han on 9/25/24.
//

#pragma once

#include "VkCommon.hpp"

#include <vector>

class Device;
//class CommandBuffer;

class CommandPool{
public:
//    CommandPool(Device &device, uint32_t queue_family_index, RenderFrame *render_frame = nullptr,
//                    size_t                   thread_index = 0,
//                    CommandBuffer::ResetMode reset_mode   = CommandBuffer::ResetMode::ResetPool);

    CommandPool(Device &device, uint32_t queueFamilyIndex, size_t thread_index = 0);

    
    CommandPool(const CommandPool &) = delete;

    CommandPool(CommandPool &&other) = delete;

    ~CommandPool();

    CommandPool &operator=(const CommandPool &) = delete;

    CommandPool &operator=(CommandPool &&) = delete;

    uint32_t getQueueFamilyIndex() const{
        return this->queueFamilyIndex;
    }
    
    VkCommandPool getHandle() const {
        return this->handle;
    }
    
    //RenderFrame *get_render_frame();

    size_t getThreadIndex() const;

    VkResult resetPool();

//    CommandBuffer &requestCommandBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    //const CommandBuffer::ResetMode get_reset_mode() const;
protected:
    Device& device;
    
    VkCommandPool handle{VK_NULL_HANDLE};
    
    size_t threadIndex{0};

    uint32_t queueFamilyIndex{0};

//    std::vector<std::unique_ptr<CommandBuffer>> primaryCommandBuffers;
//
//    uint32_t activePrimaryCommandBufferCount{0};
//
//    std::vector<std::unique_ptr<CommandBuffer>> secondaryCommandBuffers;
//
//    uint32_t activeSecondaryCommandBufferCount{0};

    //CommandBuffer::ResetMode reset_mode{CommandBuffer::ResetMode::ResetPool};

    VkResult reset_command_buffers();
    
};
