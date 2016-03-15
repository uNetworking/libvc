#ifndef DEVICEPOOL_H
#define DEVICEPOOL_H

#include <vulkan/vulkan.h>

namespace vc {

class DevicePool {
private:
    VkInstance instance;
    std::vector<Device> devices;

public:
    DevicePool()
    {
        VkInstanceCreateInfo instanceCreateInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        if (VK_SUCCESS != vkCreateInstance(&instanceCreateInfo, nullptr, &instance)) {
            throw ERROR_INSTANCE;
        }

        uint32_t numDevices;
        if (VK_SUCCESS != vkEnumeratePhysicalDevices(instance, &numDevices, nullptr) || !numDevices) {
            throw ERROR_DEVICES;
        }

        VkPhysicalDevice *physicalDevices = new VkPhysicalDevice[numDevices];
        if (VK_SUCCESS != vkEnumeratePhysicalDevices(instance, &numDevices, physicalDevices)) {
            throw ERROR_DEVICES;
        }

        for (int i = 0; i < numDevices; i++) {
            devices.push_back(Device(physicalDevices[i]));
        }

        delete [] physicalDevices;
    }

    std::vector<Device> &getDevices()
    {
        return devices;
    }

    VkInstance &getInstance()
    {
        return instance;
    }
};

}

#endif // DEVICEPOOL_H

