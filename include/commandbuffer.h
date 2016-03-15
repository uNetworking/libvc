#ifndef COMMANDBUFFER_H
#define COMMANDBUFFER_H

// needs clean-ups!

#include <vulkan/vulkan.h>
#include <vector>

namespace vc {

class CommandBuffer {
private:
    VkDevice device;
    VkCommandBuffer commandBuffer;
    VkCommandPool commandPool;

public:
    CommandBuffer(VkDevice device, int queueFamilyIndex) : device(device)
    {
        VkCommandPoolCreateInfo commandPoolCreateInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
        if (VK_SUCCESS != vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool)) {
            throw ERROR_COMMAND;
        }

        VkCommandBufferAllocateInfo commandBufferAllocateInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        commandBufferAllocateInfo.commandBufferCount = 1;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandPool = commandPool;
        if (VK_SUCCESS != vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer)) {
            throw ERROR_COMMAND;
        }
    }

    void destroy()
    {
        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        vkDestroyCommandPool(device, commandPool, nullptr);
    }

    operator VkCommandBuffer()
    {
        return commandBuffer;
    }

    void begin()
    {
        VkCommandBufferBeginInfo commandBufferBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        if (VK_SUCCESS != vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo)) {
            throw ERROR_COMMAND;
        }
    }

    void barrier()
    {
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                             VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);
    }

    void end()
    {
        if (VK_SUCCESS != vkEndCommandBuffer(commandBuffer)) {
            throw ERROR_COMMAND;
        }
    }

    void dispatch(int x, int y, int z)
    {
        vkCmdDispatch(commandBuffer, x, y, z);
    }

    void bindPipeline(Pipeline &pipeline)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    }

    // a descriptor set should probably be its own object which can be updated and bound independently
    // this function needs to be very fast!
    void bindResources(Pipeline &pipeline, std::vector<Buffer> resources)
    {
        // buffers to bind
        VkDescriptorBufferInfo *descriptorBufferInfo = new VkDescriptorBufferInfo[resources.size()];
        for (int i = 0; i < resources.size(); i++) {
            descriptorBufferInfo[i].buffer = resources[i];
            descriptorBufferInfo[i].offset = 0;
            descriptorBufferInfo[i].range = VK_WHOLE_SIZE;
        }

        // bind stuff here
        VkWriteDescriptorSet writeDescriptorSet = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        writeDescriptorSet.dstSet = pipeline.getDescriptorSet();
        writeDescriptorSet.dstBinding = 0;
        writeDescriptorSet.descriptorCount = resources.size();
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writeDescriptorSet.pBufferInfo = descriptorBufferInfo;
        vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
        delete [] descriptorBufferInfo;

        VkDescriptorSet descriptorSets[1] = {pipeline.getDescriptorSet()};
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.getPipelineLayout(), 0, 1, descriptorSets, 0, nullptr);
    }

    void copyBuffer(Buffer src, Buffer dst, size_t byteSize)
    {
        VkBufferCopy bufferCopy = {0, 0, byteSize};
        vkCmdCopyBuffer(commandBuffer, src, dst, 1, &bufferCopy);
    }
};

}

#endif // COMMANDBUFFER_H

