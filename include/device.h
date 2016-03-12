#ifndef DEVICE_H
#define DEVICE_H

#include <vulkan/vulkan.h>

namespace vc {

class Device {
private:
    VkPhysicalDevice physicalDevice;
    VkPhysicalDeviceProperties deviceProperties;
    VkDevice device;
    VkQueue queue;

    // memory types
    int memoryTypeMappable = -1, memoryTypeLocal = -1;

public:
    Device(VkPhysicalDevice physicalDevice) : physicalDevice(physicalDevice)
    {
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

        VkDeviceCreateInfo deviceCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};

        // very placeholderish!
        VkDeviceQueueCreateInfo queueCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        queueCreateInfo.queueCount = 1;
        float priorities[] = {1.0f};
        queueCreateInfo.pQueuePriorities = priorities;
        queueCreateInfo.queueFamilyIndex = 0; // should be checked!

        VkPhysicalDeviceFeatures features;

        deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
        deviceCreateInfo.pEnabledFeatures = &features;
        deviceCreateInfo.queueCreateInfoCount = 1;

        if (VK_SUCCESS != vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device)) {
            throw ERROR_DEVICES;
        }

        vkGetDeviceQueue(device, 0, 0, &queue);

        // physical device prop part of DevicePool?
        VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);
        for (int i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++) {
            if (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT && memoryTypeMappable == -1) {
                memoryTypeMappable = i;
            }

            if (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT && memoryTypeLocal == -1) {
                memoryTypeLocal = i;
            }
        }
    }

    CommandBuffer commandBuffer()
    {
        return CommandBuffer(device);
    }

    Buffer buffer(size_t byteSize)
    {
        return Buffer(device, byteSize, memoryTypeLocal);
    }

    Buffer mappable(size_t byteSize)
    {
        return Buffer(device, byteSize, memoryTypeMappable);
    }

    Pipeline pipeline(const char *fileName, std::vector<ResourceType> resourceTypes)
    {
        return Pipeline(device, fileName, resourceTypes);
    }

    void submit(CommandBuffer commandBuffer)
    {
        VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submitInfo.commandBufferCount = 1;
        VkCommandBuffer commandBuffers[1] = {commandBuffer};
        submitInfo.pCommandBuffers = commandBuffers;
        if (VK_SUCCESS != vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE)) {
            throw ERROR_DEVICES;
        }
    }

    void wait()
    {
        if (VK_SUCCESS != vkQueueWaitIdle(queue)) {
            throw ERROR_DEVICES;
        }
    }

    const char *getName()
    {
        return deviceProperties.deviceName;
    }

    uint32_t getVendorId()
    {
        return deviceProperties.vendorID;
    }
};

}

#endif // DEVICE_H

