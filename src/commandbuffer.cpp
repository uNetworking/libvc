#include "commandbuffer.h"
#include "arguments.h"

namespace vc {

void CommandBuffer::sharedConstructor()
{
    VkCommandPoolCreateInfo commandPoolCreateInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolCreateInfo.queueFamilyIndex = this->computeQueueFamily;
    if (VK_SUCCESS != vkCreateCommandPool(this->device, &commandPoolCreateInfo, nullptr, &commandPool)) {
        throw ERROR_COMMAND;
    }

    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    commandBufferAllocateInfo.commandBufferCount = 1;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandPool = commandPool;
    if (VK_SUCCESS != vkAllocateCommandBuffers(this->device, &commandBufferAllocateInfo, &commandBuffer)) {
        throw ERROR_COMMAND;
    }
}

CommandBuffer::CommandBuffer(Device &device, Program &program, Arguments &arguments) : Device(device)
{
    sharedConstructor();
    arguments.bindTo(*this);
    program.bindTo(*this);
}

CommandBuffer::CommandBuffer(Device &device) : Device(device)
{
    sharedConstructor();
}

CommandBuffer::CommandBuffer(Device &device, Program &program, Arguments &arguments);

void CommandBuffer::destroy()
{
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    vkDestroyCommandPool(device, commandPool, nullptr);
}

CommandBuffer::operator VkCommandBuffer()
{
    return commandBuffer;
}

void CommandBuffer::begin()
{
    VkCommandBufferBeginInfo commandBufferBeginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    if (VK_SUCCESS != vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo)) {
        throw ERROR_COMMAND;
    }
}

void CommandBuffer::barrier()
{
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                         VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);
}

void CommandBuffer::dispatch(int x, int y, int z)
{
    vkCmdDispatch(commandBuffer, x, y, z);
}

void CommandBuffer::end()
{
    if (VK_SUCCESS != vkEndCommandBuffer(commandBuffer)) {
        throw ERROR_COMMAND;
    }
}

}
