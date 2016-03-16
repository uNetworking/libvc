#ifndef PROGRAM_H
#define PROGRAM_H

#include "device.h"
#include <fstream>
#include <vector>

namespace vc {

class Program : protected Device {
protected:
    VkShaderModule shaderModule;
    VkPipelineLayout pipelineLayout;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipeline pipeline;

public:
    Program(Device &device, const char *fileName, std::vector<ResourceType> resourceTypes);
    void bindTo(VkCommandBuffer commandBuffer);
};

}

#endif // PROGRAM_H

