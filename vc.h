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

enum ResourceType {
    BUFFER = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
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

class Pipeline {
    friend class Device;
private:
    VkPipeline vkPipeline;

public:
    Pipeline(VkPipeline pipeline)
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
    void useShader(Pipeline &shader)
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

    void useResources()
    {


    }

    Pipeline pipeline(const char *fileName, std::vector<ResourceType> resourceTypes)
    {
        std::ifstream fin(fileName, std::ifstream::ate);
        size_t byteLength = fin.tellg();
        fin.seekg(0, std::ifstream::beg);
        char *data = new char[byteLength];
        fin.read(data, byteLength);
        fin.close();

//        Valid Usage
//        • sType must be VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO
//        • pNext must be NULL
//        • flags must be 0
//        • pCode must be a pointer to an array of codeSize
//        4
//        uint32_t values
//        • codeSize must be greater than 0
//        • codeSize must be a multiple of 4
//        • pCode must point to valid SPIR-V code, formatted and packed as described by the SPIR-V Specification v1.0
//        • pCode must adhere to the validation rules described by the Validation Rules within a Module section of the
//        SPIR-V Environment appendix
//        • pCode must declare the Shader capability
//        • pCode must not declare any capability that is not supported by the API, as described by the Capabilities section of
//        the SPIR-V Environment appendix
//        • If pCode declares any of the capabilities that are listed as not required by the implementation, the relevant feature
//        must be enabled, as listed in the SPIR-V Environment appendix
        VkShaderModule shaderModule;
        VkShaderModuleCreateInfo shaderModuleCreateInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        shaderModuleCreateInfo.codeSize = byteLength;
        shaderModuleCreateInfo.pCode = (uint32_t *) data;
        if (VK_SUCCESS != vkCreateShaderModule(device, &shaderModuleCreateInfo, nullptr, &shaderModule)) {
            throw ERROR_SHADER;
        }

//        Valid Usage
//        • sType must be VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO
//        • pNext must be NULL
//        • flags must be 0
//        • stage must be a valid VkShaderStageFlagBits value
//        • module must be a valid VkShaderModule handle
//        • pName must be a null-terminated string
//        • If pSpecializationInfo is not NULL, pSpecializationInfo must be a pointer to a valid
//        VkSpecializationInfo structure
//        • If the geometry shaders feature is not enabled, stage must not be VK_SHADER_STAGE_GEOMETRY_BIT
//        • If the tessellation shaders feature is not enabled, stage must not be VK_SHADER_STAGE_TESSELLATION_
//        CONTROL_BIT or VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT
//        • stage must not be VK_SHADER_STAGE_ALL_GRAPHICS, or VK_SHADER_STAGE_ALL
//        • pName must be the name of an OpEntryPoint in module with an execution model that matches stage
//        • If the identified entry point includes any variable in its interface that is declared with the ClipDistance
//        BuiltIn decoration, that variable must not have an array size greater than
//        VkPhysicalDeviceLimits::maxClipDistances
//        • If the identified entry point includes any variable in its interface that is declared with the CullDistance
//        BuiltIn decoration, that variable must not have an array size greater than
//        VkPhysicalDeviceLimits::maxCullDistances
//        • If the identified entry point includes any variables in its interface that are declared with the ClipDistance or
//        CullDistance BuiltIn decoration, those variables must not have array sizes which sum to more than
//        VkPhysicalDeviceLimits::maxCombinedClipAndCullDistances
//        • If the identified entry point includes any variable in its interface that is declared with the SampleMask
//        BuiltIn decoration, that variable must not have an array size greater than
//        VkPhysicalDeviceLimits::maxSampleMaskWords
//        • If stage is VK_SHADER_STAGE_VERTEX_BIT, the identified entry point must not include any input variable
//        in its interface that is decorated with CullDistance
//        • If stage is VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT or VK_SHADER_STAGE_
//        TESSELLATION_EVALUATION_BIT, and the identified entry point has an OpExecutionMode instruction
//        that specifies a patch size with OutputVertices, the patch size must be greater than 0 and less than or equal
//        to VkPhysicalDeviceLimits::maxTessellationPatchSize
//        • If stage is VK_SHADER_STAGE_GEOMETRY_BIT, the identified entry point must have an
//        OpExecutionMode instruction that specifies a maximum output vertex count that is greater than 0 and less
//        than or equal to VkPhysicalDeviceLimits::maxGeometryOutputVertices
//        • If stage is VK_SHADER_STAGE_GEOMETRY_BIT, the identified entry point must have an
//        OpExecutionMode instruction that specifies an invocation count that is greater than 0 and less than or equal to
//        VkPhysicalDeviceLimits::maxGeometryShaderInvocations
//        • If stage is VK_SHADER_STAGE_GEOMETRY_BIT, and the identified entry point writes to Layer for any
//        primitive, it must write the same value to Layer for all vertices of a given primitive
//        • If stage is VK_SHADER_STAGE_GEOMETRY_BIT, and the identified entry point writes to ViewportIndex
//        for any primitive, it must write the same value to ViewportIndex for all vertices of a given primitive
//        • If stage is VK_SHADER_STAGE_FRAGMENT_BIT, the identified entry point must not include any output
//        variables in its interface decorated with CullDistance
//        • If stage is VK_SHADER_STAGE_FRAGMENT_BIT, and the identified entry point writes to FragDepth in any
//        execution path, it must write to FragDepth in all execution paths
        VkPipelineShaderStageCreateInfo pipelineShaderInfo = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        pipelineShaderInfo.module = shaderModule;
        pipelineShaderInfo.pName = "main";
        pipelineShaderInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;




//        Valid Usage
//        • descriptorType must be a valid VkDescriptorType value
//        • If descriptorType is VK_DESCRIPTOR_TYPE_SAMPLER or VK_DESCRIPTOR_TYPE_COMBINED_
//        IMAGE_SAMPLER, and descriptorCount is not 0 and pImmutableSamplers is not NULL,
//        pImmutableSamplers must be a pointer to an array of descriptorCount valid VkSampler handles
//        • If descriptorCount is not 0, stageFlags must be a valid combination of VkShaderStageFlagBits
//        values
        VkDescriptorSetLayoutBinding *bindings = new VkDescriptorSetLayoutBinding[resourceTypes.size()];
        for (int i = 0; i < resourceTypes.size(); i++) {
            bindings[i].binding = i;
            bindings[i].descriptorType = (VkDescriptorType) resourceTypes[i];
            bindings[i].descriptorCount = 1;
            bindings[i].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
            bindings[i].pImmutableSamplers = nullptr;
        }

//        Valid Usage
//        • sType must be VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO
//        • pNext must be NULL
//        • flags must be 0
//        • If bindingCount is not 0, pBindings must be a pointer to an array of bindingCount valid
//        VkDescriptorSetLayoutBinding structures
        VkDescriptorSetLayout setLayouts;
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        descriptorSetLayoutCreateInfo.pBindings = bindings;
        descriptorSetLayoutCreateInfo.bindingCount = resourceTypes.size();
        if (VK_SUCCESS != vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, nullptr, &setLayouts)) {
            throw ERROR_SHADER;
        }




//        Valid Usage
//        • device must be a valid VkDevice handle
//        • pCreateInfo must be a pointer to a valid VkPipelineLayoutCreateInfo structure
//        • If pAllocator is not NULL, pAllocator must be a pointer to a valid VkAllocationCallbacks structure
//        • pPipelineLayout must be a pointer to a VkPipelineLayout handle
        VkPipelineLayout pipelineLayout;
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = &setLayouts;
        if (VK_SUCCESS != vkCreatePipelineLayout(device,&pipelineLayoutCreateInfo, nullptr, &pipelineLayout)) {
            throw ERROR_SHADER;
        }

//        Valid Usage
//        • device must be a valid VkDevice handle
//        • If pipelineCache is not VK_NULL_HANDLE, pipelineCache must be a valid VkPipelineCache handle
//        • pCreateInfos must be a pointer to an array of createInfoCount valid
//        VkComputePipelineCreateInfo structures
//        • If pAllocator is not NULL, pAllocator must be a pointer to a valid VkAllocationCallbacks structure
//        • pPipelines must be a pointer to an array of createInfoCount VkPipeline handles
//        • The value of createInfoCount must be greater than 0
//        • If pipelineCache is a valid handle, it must have been created, allocated or retrieved from device
//        • Each of device and pipelineCache that are valid handles must have been created, allocated or retrieved from
//        the same VkPhysicalDevice
//        • If the value of the flags member of any given element of pCreateInfos contains the VK_PIPELINE_
//        CREATE_DERIVATIVE_BIT flag, and the basePipelineIndex member of that same element is not -1, the
//        value of basePipelineIndex must be less than the index into pCreateInfos that corresponds to that element
        VkPipeline pipeline;
        VkComputePipelineCreateInfo pipelineInfo = {VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};
        pipelineInfo.stage = pipelineShaderInfo;
        pipelineInfo.layout = pipelineLayout;
        if (VK_SUCCESS != vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline)) {
            throw ERROR_DEVICES;
        }






        // bind resources to slots
        VkDescriptorSet descriptorSet;
        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        descriptorSetAllocateInfo.descriptorSetCount = 1;
        descriptorSetAllocateInfo.pSetLayouts = &setLayouts;

        if (VK_SUCCESS != vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &descriptorSet)) {
            throw ERROR_SHADER;
        }

        // bind resources
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);




        return Pipeline(pipeline);
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
