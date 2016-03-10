#ifndef BUFFER_H
#define BUFFER_H

#include <vulkan/vulkan.h>

namespace vc {

class Buffer {
private:
    VkDeviceMemory memory;
    VkBuffer buffer;
    VkDevice device;

public:
    Buffer(VkDevice device, size_t byteSize, int memoryType) : device(device)
    {
        // create buffer
        VkBufferCreateInfo bufferCreateInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferCreateInfo.size = byteSize;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        if (VK_SUCCESS != vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer)) {
            throw ERROR_MALLOC;
        }

        // get memory requirements
        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

        // allocate memory for the buffer
        VkMemoryAllocateInfo memoryAllocateInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        memoryAllocateInfo.allocationSize = memoryRequirements.size;
        memoryAllocateInfo.memoryTypeIndex = memoryType;
        if (VK_SUCCESS != vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &memory)) {
            throw ERROR_MALLOC;
        }

        // bind memory to the buffer
        if (VK_SUCCESS != vkBindBufferMemory(device, buffer, memory, 0)) {
            throw ERROR_MALLOC;
        }
    }

    // we should be able to convert to the underlying vulkan representation
    operator VkBuffer()
    {
        return buffer;
    }

    void destroy()
    {
        vkFreeMemory(device, memory, nullptr);
        vkDestroyBuffer(device, buffer, nullptr);
    }

    void unmap()
    {
        vkUnmapMemory(device, memory);
    }

    void *map()
    {
        double *pointer;
        if (VK_SUCCESS != vkMapMemory(device, memory, 0, VK_WHOLE_SIZE, 0, (void **) &pointer)) {
            throw ERROR_MAP;
        }

        return pointer;
    }
};

}

#endif // BUFFER_H

