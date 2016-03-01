#ifndef VC_H
#define VC_H

#include <vulkan/vulkan.h>
#include <vector>

namespace vc {

enum {
    ERROR_INSTANCE,
    ERROR_DEVICES,
    ERROR_MALLOC,
    ERROR_MAP
};

class Memory {
private:
    VkDeviceMemory vkMemory;
    VkDevice vkDevice;

public:
    Memory(VkDeviceMemory memory, VkDevice device)
    {
        vkMemory = memory;
        vkDevice = device;
    }

    void *map()
    {
        static double *pointer;
        if (VK_SUCCESS != vkMapMemory(vkDevice, vkMemory, 0, VK_WHOLE_SIZE, 0, (void **) &pointer)) {
            throw ERROR_MAP;
        }

        return pointer;
    }

    void unmap()
    {
        vkUnmapMemory(vkDevice, vkMemory);
    }
};

class Device {
private:
    VkPhysicalDevice vkPhysicalDevice;
    VkPhysicalDeviceProperties deviceProperties;
    VkDevice device;

public:
    Device(VkPhysicalDevice physicalDevice)
    {
        vkPhysicalDevice = physicalDevice;
        vkGetPhysicalDeviceProperties(vkPhysicalDevice, &deviceProperties);

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

        if (VK_SUCCESS != vkCreateDevice(vkPhysicalDevice, &deviceCreateInfo, nullptr, &device)) {
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

    Memory memory(size_t byteSize)
    {
        VkMemoryAllocateInfo memInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        memInfo.allocationSize = byteSize;
        memInfo.memoryTypeIndex = 0; //0 - 3 on my card, check this!
        VkDeviceMemory memory;
        if (VK_SUCCESS != vkAllocateMemory(device, &memInfo, nullptr, &memory)) {
            throw ERROR_MALLOC;
        }

        return Memory(memory, device);
    }
};

class DevicePool {
private:
    const int MAX_DEVICES = 10;
    VkInstance vkInstance;
    std::vector<Device> devices;

public:
    DevicePool()
    {
        VkInstanceCreateInfo vkInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        if (VK_SUCCESS != vkCreateInstance(&vkInfo, nullptr, &vkInstance)) {
            throw ERROR_INSTANCE;
        }

        uint32_t numDevices;
        VkPhysicalDevice vkPhysicalDevices[MAX_DEVICES];
        if (VK_SUCCESS != vkEnumeratePhysicalDevices(vkInstance, &numDevices, vkPhysicalDevices)) {
            throw ERROR_DEVICES;
        }

        for (int i = 0; i < numDevices; i++) {
            devices.push_back(Device(vkPhysicalDevices[i]));
        }
    }

    std::vector<Device> &getDevices()
    {
        return devices;
    }

    VkInstance &getInstance()
    {
        return vkInstance;
    }
};

}

#endif // VC_H
