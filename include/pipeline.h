#ifndef PIPELINE_H
#define PIPELINE_H

#include <vulkan/vulkan.h>

namespace vc {

class Pipeline {
    friend class CommandBuffer;
private:
    VkPipeline vkPipeline;
    VkPipelineLayout vkPipelineLayout;
    VkDescriptorSetLayout vkDescriptorSetLayout;
    VkDevice device;

public:
    Pipeline(VkDevice device, VkPipeline pipeline, VkPipelineLayout pipelineLayout, VkDescriptorSetLayout setLayouts) : device(device)
    {
        vkPipeline = pipeline;
        vkPipelineLayout = pipelineLayout;
        vkDescriptorSetLayout = setLayouts;
    }
};

}

#endif // PIPELINE_H

