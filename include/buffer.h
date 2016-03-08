#ifndef BUFFER_H
#define BUFFER_H

#include <vulkan/vulkan.h>

namespace vc {

class Buffer {
    friend class CommandBuffer;

private:
    VkDeviceMemory vkMemory;
    VkBuffer vkBuffer;
    VkDevice vkDevice;

public:
    Buffer(VkDevice device, size_t byteSize)
    {
        vkDevice = device;

        // allocate a buffer
        VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferInfo.size = 1024 * sizeof(float);
        bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        if (VK_SUCCESS != vkCreateBuffer(device, &bufferInfo, nullptr, &vkBuffer)) {
            throw ERROR_MALLOC;
        }

        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(device, vkBuffer, &memoryRequirements);

        std::cout << "Memory req = " << memoryRequirements.size << " bytes" << std::endl;


        // allocate memory for buffer
        VkMemoryAllocateInfo memInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        memInfo.allocationSize = memoryRequirements.size;
        memInfo.memoryTypeIndex = 0; //0 - 3 on my card, check this!
        if (VK_SUCCESS != vkAllocateMemory(device, &memInfo, nullptr, &vkMemory)) {
            throw ERROR_MALLOC;
        }

        if (VK_SUCCESS != vkBindBufferMemory(device, vkBuffer, vkMemory, 0)) {
            throw ERROR_MALLOC;
        }
    }

    void *map()
    {
        static double *pointer;
        if (VK_SUCCESS != vkMapMemory(vkDevice, vkMemory, 0, VK_WHOLE_SIZE, 0, (void **) &pointer)) {
            throw ERROR_MAP;
        }

        return pointer;
    }
};

}

#endif // BUFFER_H

