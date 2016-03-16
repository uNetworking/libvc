#include "arguments.h"

namespace vc {

Arguments::Arguments(Program &function, std::vector<Buffer> resources) : Program(function)
{
    // how many of each type
    VkDescriptorPoolSize descriptorPoolSizes[] = {
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, (uint32_t) resources.size()}
    };

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    descriptorPoolCreateInfo.poolSizeCount = 1;
    descriptorPoolCreateInfo.maxSets = 1;
    descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes;
    if (VK_SUCCESS != vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &descriptorPool)) {
        throw ERROR_SHADER;
    }

    // allocate the descriptor set (according to the function's layout)
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    descriptorSetAllocateInfo.descriptorSetCount = 1; // 1?
    descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;
    descriptorSetAllocateInfo.descriptorPool = descriptorPool;

    if (VK_SUCCESS != vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet)) {
        throw ERROR_SHADER;
    }

    // bind to this

    // buffers to bind
    VkDescriptorBufferInfo *descriptorBufferInfo = new VkDescriptorBufferInfo[resources.size()];
    for (uint32_t i = 0; i < resources.size(); i++) {
        descriptorBufferInfo[i].buffer = resources[i];
        descriptorBufferInfo[i].offset = 0;
        descriptorBufferInfo[i].range = VK_WHOLE_SIZE;
    }

    // bind stuff here
    VkWriteDescriptorSet writeDescriptorSet = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    writeDescriptorSet.dstSet = descriptorSet;//pipeline.getDescriptorSet();
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.descriptorCount = resources.size();
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeDescriptorSet.pBufferInfo = descriptorBufferInfo;
    vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
    delete [] descriptorBufferInfo;
}

void Arguments::bindTo(VkCommandBuffer commandBuffer)
{
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
}

void Arguments::destroy()
{
    vkFreeDescriptorSets(device, descriptorPool, 1, &descriptorSet);
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
}

}
