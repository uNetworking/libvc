#include "buffer.h"

namespace vc {

Buffer::Buffer(Device &device, size_t byteSize, bool mappable) : Device(device)
{
    // create buffer
    VkBufferCreateInfo bufferCreateInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferCreateInfo.size = byteSize;
    bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    if (VK_SUCCESS != vkCreateBuffer(this->device, &bufferCreateInfo, nullptr, &buffer)) {
        throw ERROR_MALLOC;
    }

    // get memory requirements
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(this->device, buffer, &memoryRequirements);

    // allocate memory for the buffer
    VkMemoryAllocateInfo memoryAllocateInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = mappable ? memoryTypeMappable : memoryTypeLocal;
    if (VK_SUCCESS != vkAllocateMemory(this->device, &memoryAllocateInfo, nullptr, &memory)) {
        throw ERROR_MALLOC;
    }

    // bind memory to the buffer
    if (VK_SUCCESS != vkBindBufferMemory(this->device, buffer, memory, 0)) {
        throw ERROR_MALLOC;
    }
}

void Buffer::fill(uint32_t value)
{
    implicitCommandBuffer->begin();
    vkCmdFillBuffer(*implicitCommandBuffer, buffer, 0, VK_WHOLE_SIZE, value);
    implicitCommandBuffer->end();
    submit(*implicitCommandBuffer);
    wait();
}

void Buffer::enqueueCopy(Buffer src, Buffer dst, size_t byteSize, VkCommandBuffer commandBuffer)
{
    VkBufferCopy bufferCopy = {0, 0, byteSize};
    vkCmdCopyBuffer(commandBuffer, src.buffer, dst.buffer, 1, &bufferCopy);
}

void Buffer::download(void *hostPtr)
{
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(this->device, buffer, &memoryRequirements);

    Buffer mappable(*this, memoryRequirements.size, memoryTypeMappable);

    implicitCommandBuffer->begin();
    enqueueCopy(*this, mappable, memoryRequirements.size, *implicitCommandBuffer);
    implicitCommandBuffer->end();
    submit(*implicitCommandBuffer);
    wait();

    memcpy(hostPtr, mappable.map(), memoryRequirements.size);
    mappable.unmap();
    mappable.destroy();
}

Buffer::operator VkBuffer()
{
    return buffer;
}

void Buffer::destroy()
{
    vkFreeMemory(device, memory, nullptr);
    vkDestroyBuffer(device, buffer, nullptr);
}

void Buffer::unmap()
{
    vkUnmapMemory(device, memory);
}

void *Buffer::map()
{
    double *pointer;
    if (VK_SUCCESS != vkMapMemory(device, memory, 0, VK_WHOLE_SIZE, 0, (void **) &pointer)) {
        throw ERROR_MAP;
    }

    return pointer;
}

}
