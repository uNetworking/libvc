#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include "program.h"
#include "buffer.h"
#include <vector>

namespace vc {

class Arguments : protected Program {
private:
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;

public:
    Arguments(Program &function, std::vector<Buffer> resources);
    void bindTo(VkCommandBuffer commandBuffer);
    void destroy();
};

}

#endif // ARGUMENTS_H

