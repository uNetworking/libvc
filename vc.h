#ifndef VC_H
#define VC_H

#include <vulkan/vulkan.h>
#include <vector>
#include <fstream>

// only for debug
#include <chrono>
#include <iostream>

namespace vc {

enum Error{
    ERROR_INSTANCE,
    ERROR_DEVICES,
    ERROR_MALLOC,
    ERROR_MAP,
    ERROR_SHADER
};

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

class Device;

class Shader {
    friend class Device;
private:
    VkPipeline vkPipeline;

public:
    Shader(VkPipeline pipeline)
    {
        vkPipeline = pipeline;
    }
};

class Device {
private:
    VkPhysicalDevice vkPhysicalDevice;
    VkPhysicalDeviceProperties deviceProperties;
    VkDevice device;
    VkQueue queue;
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

public:
    Device(VkPhysicalDevice physicalDevice)
    {
        vkPhysicalDevice = physicalDevice;
        vkGetPhysicalDeviceProperties(vkPhysicalDevice, &deviceProperties);

        VkDeviceCreateInfo deviceCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};

        // very placeholderish!
        VkDeviceQueueCreateInfo queueCreateInfo = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
        queueCreateInfo.queueCount = 1;
        float priorities[] = {1.0f};
        queueCreateInfo.pQueuePriorities = priorities;
        queueCreateInfo.queueFamilyIndex = 0; // should be checked!

        VkPhysicalDeviceFeatures features;

        deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
        deviceCreateInfo.pEnabledFeatures = &features;
        deviceCreateInfo.queueCreateInfoCount = 1;

        if (VK_SUCCESS != vkCreateDevice(vkPhysicalDevice, &deviceCreateInfo, nullptr, &device)) {
            throw ERROR_DEVICES;
        }

        vkGetDeviceQueue(device, 0, 0, &queue);


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

        // this should be a function of the CommandBuffer object: begin()
        VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        if (VK_SUCCESS != vkBeginCommandBuffer(commandBuffer, &beginInfo)) {
            throw ERROR_DEVICES;
        }
    }

    // bind shader to command buffer
    void useShader(Shader &shader)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, shader.vkPipeline);
    }

    // drain, split this up
    void drain()
    {
        // this should be a function in CommandBuffer: end()
        if (VK_SUCCESS != vkEndCommandBuffer(commandBuffer)) {
            throw ERROR_DEVICES;
        }

        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

        // Device.submit(CommandBuffer)
        VkSubmitInfo submitInfo = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        if (VK_SUCCESS != vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE)) {
            throw ERROR_DEVICES;
        }

        // Device.finish() / drain()
        if (VK_SUCCESS != vkQueueWaitIdle(queue)) {
            throw ERROR_DEVICES;
        }

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << "Computation took " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms.\n";
    }

    // this will dispatch the previously "used" shader
    void dispatch(int x, int y, int z)
    {
        vkCmdDispatch(commandBuffer, x, y, z);
    }

    // compileShader? createShader?
    Shader loadShader(const char *fileName)
    {
        std::ifstream fin(fileName, std::ifstream::ate);
        size_t byteLength = fin.tellg();
        fin.seekg(0, std::ifstream::beg);
        char *data = new char[byteLength];
        fin.read(data, byteLength);
        fin.close();

        VkShaderModuleCreateInfo shaderModuleCreateInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        shaderModuleCreateInfo.codeSize = byteLength;
        shaderModuleCreateInfo.pCode = (uint32_t *) data;

        VkShaderModule shaderModule;
        if (VK_SUCCESS != vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shaderModule)) {
            throw ERROR_SHADER;
        }


        // also create the pipeline here!
        VkPipelineShaderStageCreateInfo pipelineShaderInfo = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        pipelineShaderInfo.module = shaderModule;
        pipelineShaderInfo.pName = "main";
        pipelineShaderInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;

        VkComputePipelineCreateInfo pipelineInfo = {VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
        pipelineInfo.stage = pipelineShaderInfo;


        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        pipelineLayoutCreateInfo.setLayoutCount = 1;


        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        // todo: specify layout resources
        descriptorSetLayoutCreateInfo.bindingCount = 1;

        VkDescriptorSetLayoutBinding bindings;


        // a vector of the bindings in this layout set!
        descriptorSetLayoutCreateInfo.pBindings = &bindings;
        bindings.binding = 0;
        bindings.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        bindings.descriptorCount = 1;
        bindings.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;


        VkDescriptorSetLayout setLayouts;
        if (VK_SUCCESS != vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, nullptr, &setLayouts)) {
            throw ERROR_SHADER;
        }


        pipelineLayoutCreateInfo.pSetLayouts = &setLayouts;


        VkPipelineLayout pipelineLayout;
        if (VK_SUCCESS != vkCreatePipelineLayout(device,&pipelineLayoutCreateInfo, nullptr, &pipelineLayout)) {
            throw ERROR_SHADER;
        }


        // needed to not crash when using shaders with buffers!
        pipelineInfo.layout = pipelineLayout;

        // this is a place where things can hang if the shader is somehow bad
        VkPipeline pipeline;
        if (VK_SUCCESS != vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline)) {
            throw ERROR_DEVICES;
        }

        return Shader(pipeline);
    }

    const char *getName()
    {
        return deviceProperties.deviceName;
    }

    uint32_t getVendorId()
    {
        return deviceProperties.vendorID;
    }

    Memory memory(size_t byteSize)
    {
        VkMemoryAllocateInfo memInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
        memInfo.allocationSize = byteSize;
        memInfo.memoryTypeIndex = 0; //0 - 3 on my card, check this!
        VkDeviceMemory memory;
        if (VK_SUCCESS != vkAllocateMemory(device, &memInfo, nullptr, &memory)) {
            throw ERROR_MALLOC;
        }

        return Memory(memory, device);
    }
};

class DevicePool {
private:
    const int MAX_DEVICES = 10; // you dont need this, can get from the driver
    VkInstance vkInstance;
    std::vector<Device> devices;

public:
    DevicePool()
    {
        VkInstanceCreateInfo vkInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
        if (VK_SUCCESS != vkCreateInstance(&vkInfo, nullptr, &vkInstance)) {
            throw ERROR_INSTANCE;
        }

        uint32_t numDevices;
        VkPhysicalDevice vkPhysicalDevices[MAX_DEVICES];
        if (VK_SUCCESS != vkEnumeratePhysicalDevices(vkInstance, &numDevices, vkPhysicalDevices)) {
            throw ERROR_DEVICES;
        }

        for (int i = 0; i < numDevices; i++) {
            devices.push_back(Device(vkPhysicalDevices[i]));
        }
    }

    std::vector<Device> &getDevices()
    {
        return devices;
    }

    VkInstance &getInstance()
    {
        return vkInstance;
    }
};

}

#endif // VC_H
