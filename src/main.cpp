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
            Pipeline pipeline = device.pipeline("/home/alexhultman/libvc/shaders/comp.spv", {BUFFER, BUFFER});
            Buffer buffer1 = device.buffer(sizeof(float) * 256);
            Buffer buffer2 = device.buffer(sizeof(float) * 256);

            // Build the command buffer
            CommandBuffer commandBuffer = device.commandBuffer();
            commandBuffer.begin();
            commandBuffer.bindPipeline(pipeline);
            commandBuffer.bindResources(pipeline, {buffer1, buffer2});
            commandBuffer.dispatch(1, 1, 1);
            commandBuffer.end();

            // Submit and wait for command buffer to finish
            std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
            device.submit(commandBuffer);
            device.wait();
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            std::cout << "Computation took " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms.\n";

            // Map the memory of the buffer and print it
            cout << "Vector1:";
            float *reals = (float *) buffer1.map();
            for (int i = 0; i < 256; i++) {
                cout << " " << reals[i];
            }
            cout << endl;

            buffer1.unmap();
            buffer1.destroy();

            cout << "Vector2:";
            reals = (float *) buffer2.map();
            for (int i = 0; i < 256; i++) {
                cout << " " << reals[i];
            }
            cout << endl;

            buffer2.unmap();
            buffer2.destroy();

        } catch(vc::Error e) {
            cout << "vc::Error thrown" << endl;
        }
    }

    cout << "Done" << endl;
    return 0;
}

