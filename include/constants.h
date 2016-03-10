#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <vulkan/vulkan.h>

namespace vc {

enum Error{
    ERROR_INSTANCE,
    ERROR_DEVICES,
    ERROR_MALLOC,
    ERROR_MAP,
    ERROR_SHADER
};

enum ResourceType {
    BUFFER = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
};

}

#endif // CONSTANTS_H

