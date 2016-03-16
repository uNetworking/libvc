#ifndef DEVICEPOOL_H
#define DEVICEPOOL_H

#include "device.h"
#include <vector>

namespace vc {

class DevicePool {
private:
    VkInstance instance;
    std::vector<Device> devices;

public:
    DevicePool();
    std::vector<Device> &getDevices();
    VkInstance &getInstance();
};

}

#endif // DEVICEPOOL_H

