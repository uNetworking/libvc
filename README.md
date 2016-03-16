# Vulkan Compute for C++
```libvc``` is a GPGPU engine based on Vulkan. It eats SPIR-V compute shaders and executes them without any graphical context, much like ```OpenCL```. The interface is very abstract and allows you to get to work with your shaders as quickly as possible. With the current state of compute shaders, you should be able to achieve ~OpenCL 1.2 feature parity.

Compute shaders are constantly being evolved and extended with new versions of OpenGL, GLSL and/or Vulkan. They are here to stay and will only become more competent with time.

* SSBO's allows for passing mutable linear global memory to your shaders.
* Vulkan command buffers allows for recording commands once and replaying them multiple times, minimizing CPU overhead.
* NVIDIA refuse to embrace OpenCL in favor of their proprietary CUDA alternative - but they do embrace Vulkan and OpenGL.

![](images/vulkan.png) ![](images/spir.png)

## Overview
The interface is still being designed. It will feature abstractions for devices, memory, buffers, shaders, etc.

```c++
DevicePool devicePool;
for (Device &device : devicePool.getDevices()) {
    cout << "Found device: " << device.getName() << " from vendor: 0x"
         << hex << device.getVendorId() << dec << endl;

    try {
            // Allocate a buffer & clear it
            Buffer buffer(device, sizeof(double) * 10240);
            buffer.fill(0);

            // Compile the compute shader & prepare to use the buffer as argument
            Program program(device, "shaders/comp.spv", {BUFFER});
            Arguments args(program, {buffer});

            // Create and build the command buffer, makig use of the program and arguments
            CommandBuffer commands(device, program, args);
            for (int i = 0; i < 100000; i++) {
                commands.dispatch(10);
                commands.barrier();
            }
            commands.end();

            // Time the execution on the GPU
            for (int i = 0; i < 5; i++) {
                steady_clock::time_point start = steady_clock::now();
                device.submit(commands);
                device.wait();
                cout << duration_cast<milliseconds>(steady_clock::now() - start).count() << "ms" << endl;
            }

            // Download results and show the first scalar
            double results[10240];
            buffer.download(results);
            cout << "Result: " << results[0] << endl;

    } catch(vc::Error e) {
        cout << "vc::Error thrown" << endl;
    }
}
```
