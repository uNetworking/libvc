#ifndef DEVICE_H
#define DEVICE_H

#include <vulkan/vulkan.h>

namespace vc {

class Device {
private:
    VkPhysicalDevice physicalDevice;
    VkPhysicalDeviceProperties physicalDeviceProperties;
    VkDevice device;
    VkQueue queue;

    int memoryTypeMappable = -1,
        memoryTypeLocal = -1,
        computeQueueFamily = -1;

public:
    Device(VkPhysicalDevice physicalDevice) : physicalDevice(physicalDevice)
    {
        // select a queue family with compute support
        uint32_t numQueues;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numQueues, nullptr);

        VkQueueFamilyProperties *queueFamilyProperties = new VkQueueFamilyProperties[numQueues];
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numQueues, queueFamilyProperties);

        for (int i = 0; i < numQueues; i++) {
            if (queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
                computeQueueFamily = i;
                break;
            }
        }

        delete [] queueFamilyProperties;
        if (computeQueueFamily == -1) {
            throw ERROR_DEVICES;
        }

        VkDeviceQueueCreateInfo queueCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        queueCreateInfo.queueCount = 1;
        float priorities[] = {1.0f};
        queueCreateInfo.pQueuePriorities = priorities;
        queueCreateInfo.queueFamilyIndex = computeQueueFamily;

        // create the logical device
        VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
        VkDeviceCreateInfo deviceCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
        deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
        deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;
        deviceCreateInfo.queueCreateInfoCount = 1;
        if (VK_SUCCESS != vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device)) {
            throw ERROR_DEVICES;
        }

        vkGetDeviceQueue(device, computeQueueFamily, 0, &queue);
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

        // get indices of memory types we care about
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

    void destroy()
    {
        vkDestroyDevice(device, nullptr);
    }

    CommandBuffer commandBuffer()
    {
        return CommandBuffer(device, computeQueueFamily);
    }

    CommandBuffer beginCommandBuffer(Pipeline &pipeline, std::vector<Buffer> resources)
    {
        CommandBuffer cb(device, computeQueueFamily);
        cb.begin();
        cb.bindPipeline(pipeline);
        cb.bindResources(pipeline, resources);
        return cb;
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
        return physicalDeviceProperties.deviceName;
    }

    uint32_t getVendorId()
    {
        return physicalDeviceProperties.vendorID;
    }
};

}

#endif // DEVICE_H

