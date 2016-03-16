#ifndef COMMANDBUFFER_H
#define COMMANDBUFFER_H

#include "device.h"
#include <vector>

namespace vc {

class Program;
class Arguments;

class CommandBuffer : protected Device {
private:
    VkCommandBuffer commandBuffer;
    VkCommandPool commandPool;
    void sharedConstructor();

public:
    CommandBuffer(Device &device);
    CommandBuffer(Device &device, Program &program, Arguments &arguments);
    void destroy();
    operator VkCommandBuffer();
    void begin();
    void barrier();
    void dispatch(int x = 1, int y = 1, int z = 1);
    void end();
};

}

#endif // COMMANDBUFFER_H

