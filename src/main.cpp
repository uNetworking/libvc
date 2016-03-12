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
            Pipeline pipeline = device.pipeline("/home/alexhultman/libvc/shaders/comp.spv", {BUFFER});
            Buffer output = device.buffer(sizeof(double) * 10240);

            // full buffer with 0 (very explicit)
            Buffer mappable = device.mappable(sizeof(double) * 10240);
            double *inputScalars = (double *) mappable.map();
            for (int i = 0; i < 10240; i++) {
                inputScalars[i] = 0;
            }
            mappable.unmap();

            CommandBuffer writeCommands = device.commandBuffer();
            writeCommands.begin();
            writeCommands.copyBuffer(mappable, output, 10240 * sizeof(double));
            writeCommands.end();
            device.submit(writeCommands);
            device.wait();

            // create a command buffer with 100k passes
            CommandBuffer commandBuffer = device.commandBuffer();
            commandBuffer.begin();
            commandBuffer.bindPipeline(pipeline);
            commandBuffer.bindResources(pipeline, {output});

            steady_clock::time_point start = steady_clock::now();
            for (int i = 0; i < 100000; i++) {
                commandBuffer.dispatch(10, 1, 1);
                commandBuffer.barrier();
            }
            commandBuffer.end();
            cout << "Command buffer creation took " << duration_cast<milliseconds>(steady_clock::now() - start).count() << "ms.\n";

            //fflush(stdout);
            //this_thread::sleep_for(milliseconds(500));

            // submit the same buffer twice, resultig in 200k passes
            start = steady_clock::now();
            for (int i = 0; i < 1; i++) {
                device.submit(commandBuffer);
            }
            device.wait();
            cout << "Computation took " << duration_cast<milliseconds>(steady_clock::now() - start).count() << "ms.\n";

            //fflush(stdout);
            //this_thread::sleep_for(milliseconds(500));

            // download results to our mappable buffer!
            CommandBuffer readCommands = device.commandBuffer();
            readCommands.begin();
            readCommands.copyBuffer(output, mappable, 10240 * sizeof(double));
            readCommands.end();
            device.submit(readCommands);
            device.wait();

            // check for correctness
            double *outputScalars = (double *) mappable.map();
            cout << "Scalar is: " << outputScalars[0] << endl;
            for (int i = 1; i < 10240; i++) {
                if (outputScalars[i] != outputScalars[i - 1]) {
                    cout << "Corruption at " << i << ": " << outputScalars[i] << " != " << outputScalars[i - 1] << endl;
                    break;
                } else if (i == 10240 - 1) {
                    cout << "Everything matches!" << endl;
                }
            }
            mappable.unmap();


            /*
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
            buffer2.destroy();*/

        } catch(vc::Error e) {
            cout << "vc::Error thrown" << endl;
        }
    }

    cout << "Done" << endl;
    return 0;
}

