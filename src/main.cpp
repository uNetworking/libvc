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

        try {
            // Compile shader and allocate a buffer
            Pipeline pipeline = device.pipeline("/home/alexhultman/libvc/shaders/comp.spv", {BUFFER});
            Buffer buffer = device.buffer(sizeof(float) * 256);

            // Build the command buffer
            CommandBuffer commandBuffer = device.commandBuffer();
            commandBuffer.begin();
            commandBuffer.bindPipeline(pipeline);
            commandBuffer.bindResources(pipeline, {buffer});
            commandBuffer.dispatch(1, 1, 1);
            commandBuffer.end();

            // Submit and wait for command buffer to finish
            device.submit(commandBuffer);
            device.drain();

            // Map the memory of the buffer and print it
            cout << "Vector:";
            float *reals = (float *) buffer.map();
            for (int i = 0; i < 256; i++) {
                cout << " " << reals[i];
            }
            cout << endl;

        } catch(vc::Error e) {
            cout << "vc::Error thrown" << endl;
        }
    }

    cout << "Done" << endl;
    return 0;
}

