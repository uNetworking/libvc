#ifndef COMMANDBUFFER_H
#define COMMANDBUFFER_H

#include <vulkan/vulkan.h>

namespace vc {

class CommandBuffer {
    friend class Device;

private:
    VkDevice device;
    VkCommandBuffer commandBuffer;
    VkCommandPool commandPool;

public:
    CommandBuffer(VkDevice device) : device(device)
    {
        // create command pool
        VkCommandPoolCreateInfo commandPoolInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandPoolInfo.queueFamilyIndex = 0;
        if (VK_SUCCESS != vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool)) {
            throw ERROR_DEVICES;
        }

        // create command buffer (these should be exposed separately as objects that can be submitted to devices)
        VkCommandBufferAllocateInfo commandBufferInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        commandBufferInfo.commandBufferCount = 1;
        commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferInfo.commandPool = commandPool;
        if (VK_SUCCESS != vkAllocateCommandBuffers(device, &commandBufferInfo, &commandBuffer)) {
            throw ERROR_DEVICES;
        }

    }

    void begin()
    {
        // this should be a function of the CommandBuffer object: begin()
        VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        if (VK_SUCCESS != vkBeginCommandBuffer(commandBuffer, &beginInfo)) {
            throw ERROR_DEVICES;
        }
    }

    void end()
    {
        // this should be a function in CommandBuffer: end()
        if (VK_SUCCESS != vkEndCommandBuffer(commandBuffer)) {
            throw ERROR_DEVICES;
        }
    }

    // this will dispatch the previously "used" shader
    void dispatch(int x, int y, int z)
    {
        vkCmdDispatch(commandBuffer, x, y, z);
    }

    // bind shader to command buffer
    void bindPipeline(Pipeline &shader)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, shader.vkPipeline);
    }

    // should be part of the pipeline!
    void bindResources(Pipeline &pipeline, std::vector<Buffer> resources)
    {
        // how many of each type
        VkDescriptorPoolSize descriptorPoolSizes[] = {
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1}
        };

        VkDescriptorPool descriptorPool;
        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        descriptorPoolCreateInfo.poolSizeCount = 1;
        descriptorPoolCreateInfo.maxSets = 1;
        descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes;
        if (VK_SUCCESS != vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &descriptorPool)) {
            throw ERROR_SHADER;
        }



        // allocate the descriptor set
        VkDescriptorSet descriptorSet;
        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        descriptorSetAllocateInfo.descriptorSetCount = 1;
        descriptorSetAllocateInfo.pSetLayouts = &pipeline.vkDescriptorSetLayout;
        descriptorSetAllocateInfo.descriptorPool = descriptorPool;

        if (VK_SUCCESS != vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet)) {
            throw ERROR_SHADER;
        }


        // buffers to bind
        VkDescriptorBufferInfo descriptorBufferInfo;
        descriptorBufferInfo.buffer = resources[0].vkBuffer;
        descriptorBufferInfo.offset = 0;
        descriptorBufferInfo.range = VK_WHOLE_SIZE;

        // bind stuff here
        VkWriteDescriptorSet writeDescriptorSet = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        writeDescriptorSet.dstSet = descriptorSet;
        writeDescriptorSet.dstBinding = 0;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;
        vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);

        // use bindings
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.vkPipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
    }
};

}

#endif // COMMANDBUFFER_H

