#ifndef MEMORY_H
#define MEMORY_H

#include <vulkan/vulkan.h>

namespace vc {

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

}

#endif // MEMORY_H

