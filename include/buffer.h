#ifndef BUFFER_H
#define BUFFER_H

#include "device.h"
#include "commandbuffer.h"
#include <cstring>

namespace vc {

class Buffer : protected Device {
private:
    VkDeviceMemory memory;
    VkBuffer buffer;

public:
    Buffer(Device &device, size_t byteSize, bool mappable = false);
    void fill(uint32_t value);
    void enqueueCopy(Buffer src, Buffer dst, size_t byteSize, VkCommandBuffer commandBuffer);
    void download(void *hostPtr);
    operator VkBuffer();
    void destroy();
    void unmap();
    void *map();
};

}

#endif // BUFFER_H

