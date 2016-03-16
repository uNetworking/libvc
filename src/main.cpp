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
        cout << "[" << device.getName() << "]" << endl;

        try {
            Buffer buffer(device, sizeof(double) * 10240);
            buffer.fill(0);

            Program program(device, "/home/alexhultman/libvc/shaders/comp.spv", {BUFFER});
            Arguments args(program, {buffer});

            CommandBuffer commands(device, program, args);
            for (int i = 0; i < 100000; i++) {
                commands.dispatch(10);
                commands.barrier();
            }
            commands.end();

            // time the execution on the GPU
            for (int i = 0; i < 5; i++) {
                steady_clock::time_point start = steady_clock::now();
                device.submit(commands);
                device.wait();
                cout << duration_cast<milliseconds>(steady_clock::now() - start).count() << "ms" << endl;
            }

            double results[10240];
            buffer.download(results);

            buffer.destroy();
            args.destroy();
            commands.destroy();
            device.destroy();

            cout << "Scalar is: " << results[0] << endl;
            for (int i = 1; i < 10240; i++) {
                if (results[i] != results[i - 1]) {
                    cout << "Corruption at " << i << ": " << results[i] << " != " << results[i - 1] << endl;
                    return -1;
                }
            }
        } catch(vc::Error e) {
            cout << "vc::Error thrown" << endl;
        }
    }

    cout << "OK" << endl;
    return 0;
}
