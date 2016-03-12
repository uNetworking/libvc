#include "vc.h"
using namespace vc;

#include <thread>
#include <iostream>
#include <iomanip>
#include <chrono>
using namespace std;
using namespace chrono;

#define BUFFER_SIZE 10240
#define INCREMENT_PASSES 100000
#define RUNS 5

int main()
{
    DevicePool devicePool;
    for (Device &device : devicePool.getDevices()) {
        cout << "[" << device.getName() << "]" << endl;

        try {
            Pipeline pipeline = device.pipeline("../shaders/comp.spv", {BUFFER});
            Buffer output = device.buffer(sizeof(double) * BUFFER_SIZE);

            // full buffer with 0 (very explicit)
            Buffer mappable = device.mappable(sizeof(double) * BUFFER_SIZE);
            double *inputScalars = (double *) mappable.map();
            for (int i = 0; i < BUFFER_SIZE; i++) {
                inputScalars[i] = 0;
            }
            mappable.unmap();

            CommandBuffer writeCommands = device.commandBuffer();
            writeCommands.begin();
            writeCommands.copyBuffer(mappable, output, BUFFER_SIZE * sizeof(double));
            writeCommands.end();
            device.submit(writeCommands);
            device.wait();

            // create a command buffer with 100k passes
            CommandBuffer commandBuffer = device.commandBuffer();
            commandBuffer.begin();
            commandBuffer.bindPipeline(pipeline);
            commandBuffer.bindResources(pipeline, {output});

            steady_clock::time_point start = steady_clock::now();
            for (int i = 0; i < INCREMENT_PASSES; i++) {
                commandBuffer.dispatch(10, 1, 1);
                commandBuffer.barrier();
            }
            commandBuffer.end();
            //cout << "Command buffer creation took " << duration_cast<milliseconds>(steady_clock::now() - start).count() << "ms.\n";

            for (int i = 0; i < RUNS; i++) {
                start = steady_clock::now();
                device.submit(commandBuffer);
                device.wait();
                cout << i + 1 << ": " << duration_cast<milliseconds>(steady_clock::now() - start).count() << "ms" << endl;
            }

            // download results to our mappable buffer
            CommandBuffer readCommands = device.commandBuffer();
            readCommands.begin();
            readCommands.copyBuffer(output, mappable, BUFFER_SIZE * sizeof(double));
            readCommands.end();
            device.submit(readCommands);
            device.wait();

            // check for correctness
            double *outputScalars = (double *) mappable.map();

            if (int(outputScalars[0] + 0.5) != INCREMENT_PASSES * RUNS) {
                cout << "Mismatching result!" << endl;
                return -3;
            }

            for (int i = 1; i < BUFFER_SIZE; i++) {
                if (outputScalars[i] != outputScalars[i - 1]) {
                    cout << "Corruption at " << i << ": " << outputScalars[i] << " != " << outputScalars[i - 1] << endl;
                    return -1;
                }
            }
            mappable.unmap();

        } catch(vc::Error e) {
            cout << "vc::Error thrown" << endl;
            return -2;
        }
    }

    cout << "OK" << endl;
    return 0;
}

