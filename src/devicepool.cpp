#include "devicepool.h"

namespace vc {

DevicePool::DevicePool()
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

    for (uint32_t i = 0; i < numDevices; i++) {
        devices.push_back(Device(physicalDevices[i]));
    }

    delete [] physicalDevices;
}

std::vector<Device> &DevicePool::getDevices()
{
    return devices;
}

VkInstance &DevicePool::getInstance()
{
    return instance;
}

}
