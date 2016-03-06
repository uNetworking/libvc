#include "vc.h"
using namespace vc;

#include <thread>
#include <iostream>
#include <iomanip>
#include <chrono>
using namespace std;
using namespace chrono;

int main()
{
    DevicePool devicePool;
    for (Device &device : devicePool.getDevices()) {
        cout << "Found device: " << device.getName() << " from vendor: 0x" << hex << device.getVendorId() << dec << endl;

        Memory mem = device.memory(1000 * sizeof(int));
        int *map = (int *) mem.map();

        for (int i = 0; i < 1000; i++) {
            map[i] = i;
        }

        mem.unmap();

        try {

        // load / compile shader
        Pipeline pipeline = device.pipeline("/home/alexhultman/comp.spv", {BUFFER});

        // bind shader to current command buffer (currently hidden in device)
        device.useShader(pipeline);

        // bind resources in order - take vector of resources
        device.useResources();

        // enquque a range to the current command buffer
        device.dispatch(1000, 1, 1);

        // end the command buffer and submit it, wait for it to finish, time it
        device.drain();

        } catch(vc::Error e) {
            cout << "Exception thrown" << endl;
        }
    }


//    /*VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
//    bufferInfo.size = 102400000;
//    VkBuffer buffer;
//    if (VK_SUCCESS != vkCreateBuffer(device, &bufferInfo, nullptr, &buffer)) {
//        cout << "Could not create buffer!" << endl;
//    }*/

//    //

    //this_thread::sleep_for(seconds(1));
    cout << "Done" << endl;
    return 0;
}

