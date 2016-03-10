#ifndef PIPELINE_H
#define PIPELINE_H

#include <vulkan/vulkan.h>

namespace vc {

class Pipeline {
    // todo: this should not be a friend
    friend class CommandBuffer;
private:
    VkPipeline pipeline;
    VkPipelineLayout vkPipelineLayout;
    VkDescriptorSetLayout vkDescriptorSetLayout;
    VkDevice device;

public:
    Pipeline(VkDevice device, VkPipeline pipeline, VkPipelineLayout pipelineLayout, VkDescriptorSetLayout setLayouts) : device(device), pipeline(pipeline)
    {
        vkPipelineLayout = pipelineLayout;
        vkDescriptorSetLayout = setLayouts;
    }

    operator VkPipeline()
    {
        return pipeline;
    }
};

}

#endif // PIPELINE_H

