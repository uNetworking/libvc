#ifndef DEVICE_H
#define DEVICE_H

#include <vulkan/vulkan.h>
#include "constants.h"

namespace vc {

class CommandBuffer;

class Device {
protected:
    VkPhysicalDevice physicalDevice;
    VkPhysicalDeviceProperties physicalDeviceProperties;
    VkDevice device;
    VkQueue queue;
    CommandBuffer *implicitCommandBuffer;

    int memoryTypeMappable = -1,
        memoryTypeLocal = -1,
        computeQueueFamily = -1;

public:
    Device(VkPhysicalDevice physicalDevice);
    void destroy();
    void submit(VkCommandBuffer commandBuffer);
    void wait();
    const char *getName();
    uint32_t getVendorId();
};

}

#endif // DEVICE_H

