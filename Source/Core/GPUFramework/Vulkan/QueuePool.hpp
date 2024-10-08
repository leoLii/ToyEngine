#pragma once

#include "VkCommon.hpp"

#include <queue>

class Device;
class Window;

enum QueueFunction {
    QUEUE_GRAPHICS_BIT = 1 << 0,
    QUEUE_COMPUTE_BIT = 1 << 1,
    QUEUE_TRANSFER_BIT = 1 << 2,
    QUEUE_PRESENT_BIT = 1 << 3,
};

using QueueFunctionFlags = uint32_t;

struct Queue {
    vk::Queue handle;
    uint32_t familyIndex;
    QueueFunctionFlags functions;
};

class QueuePool {
public:
    QueuePool() = delete;
    QueuePool(const Device&, const Window&);

    // Device will destroy queues automaticaly
    ~QueuePool() = default;

    const Queue& acquireQueue(const QueueFunctionFlags);

    void returnQueue(const Queue&);

protected:
    const Device& device;

    const Window& window;

    std::atomic<size_t> availableCount = 0;

    std::queue<Queue> queues;
};