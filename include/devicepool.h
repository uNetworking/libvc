#ifndef DEVICEPOOL_H
#define DEVICEPOOL_H

#include <vulkan/vulkan.h>

namespace vc {

class DevicePool {
private:
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
        if (VK_SUCCESS != vkEnumeratePhysicalDevices(vkInstance, &numDevices, nullptr) || !numDevices) {
            throw ERROR_DEVICES;
        }

        VkPhysicalDevice vkPhysicalDevices[numDevices];
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

#endif // DEVICEPOOL_H

