#ifndef COMMANDBUFFER_H
#define COMMANDBUFFER_H

#include <vulkan/vulkan.h>
#include <vector>

namespace vc {

class CommandBuffer {
private:
    VkDevice device;
    VkCommandBuffer commandBuffer;
    VkCommandPool commandPool;

public:
    CommandBuffer(VkDevice device) : device(device)
    {
        // todo: share this pool
        VkCommandPoolCreateInfo commandPoolInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandPoolInfo.queueFamilyIndex = 0;
        if (VK_SUCCESS != vkCreateCommandPool(device, &commandPoolInfo, nullptr, &commandPool)) {
            throw ERROR_DEVICES;
        }

        VkCommandBufferAllocateInfo commandBufferInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        commandBufferInfo.commandBufferCount = 1;
        commandBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferInfo.commandPool = commandPool;
        if (VK_SUCCESS != vkAllocateCommandBuffers(device, &commandBufferInfo, &commandBuffer)) {
            throw ERROR_DEVICES;
        }
    }

    operator VkCommandBuffer()
    {
        return commandBuffer;
    }

    // todo: multiple submits by default?
    void begin()
    {
        VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        if (VK_SUCCESS != vkBeginCommandBuffer(commandBuffer, &beginInfo)) {
            throw ERROR_DEVICES;
        }
    }

    void end()
    {
        if (VK_SUCCESS != vkEndCommandBuffer(commandBuffer)) {
            throw ERROR_DEVICES;
        }
    }

    void dispatch(int x, int y, int z)
    {
        vkCmdDispatch(commandBuffer, x, y, z);
    }

    // pipeline.bind(commandBuffer)?
    void bindPipeline(Pipeline &pipeline)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    }

    // makes use of privates in the pipeline! pipeline.bind(resources)?
    void bindResources(Pipeline &pipeline, std::vector<Buffer> resources)
    {
        // how many of each type
        VkDescriptorPoolSize descriptorPoolSizes[] = {
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, resources.size()}
        };

        // share this one?
        VkDescriptorPool descriptorPool;
        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        descriptorPoolCreateInfo.poolSizeCount = 1; // 1?
        descriptorPoolCreateInfo.maxSets = 1;
        descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes;
        if (VK_SUCCESS != vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &descriptorPool)) {
            throw ERROR_SHADER;
        }


        // allocate the descriptor set
        VkDescriptorSet descriptorSet;
        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        descriptorSetAllocateInfo.descriptorSetCount = 1; // 1?

        // private!
        descriptorSetAllocateInfo.pSetLayouts = &pipeline.descriptorSetLayout;

        descriptorSetAllocateInfo.descriptorPool = descriptorPool;

        if (VK_SUCCESS != vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet)) {
            throw ERROR_SHADER;
        }


        // buffers to bind
        VkDescriptorBufferInfo descriptorBufferInfo[resources.size()];
        for (int i = 0; i < resources.size(); i++) {
            descriptorBufferInfo[i].buffer = resources[i];
            descriptorBufferInfo[i].offset = 0;
            descriptorBufferInfo[i].range = VK_WHOLE_SIZE;
        }

        // bind stuff here
        VkWriteDescriptorSet writeDescriptorSet = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
        writeDescriptorSet.dstSet = descriptorSet;
        writeDescriptorSet.dstBinding = 0;
        writeDescriptorSet.descriptorCount = resources.size();
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writeDescriptorSet.pBufferInfo = descriptorBufferInfo;
        vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);

        // use bindings
        // private!
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
    }
};

}

#endif // COMMANDBUFFER_H

