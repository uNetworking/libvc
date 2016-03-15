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
        // Compile shader and allocate a buffer
        Pipeline pipeline = device.pipeline("shaders/comp.spv", {BUFFER});
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
```
