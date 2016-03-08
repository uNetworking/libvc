# Vulkan Compute for C++
```libvc``` is a GPGPU engine based on Vulkan. It eats SPIR-V compute shaders and executes them without any graphical context, much like ```OpenCL```. The interface is very abstract and allows you to get to work with your shaders as quickly as possible. With the current state of compute shaders, you should be able to achieve ~OpenCL 1.2 feature parity.

Compute shaders are constantly being evolved and extended with new versions of OpenGL, GLSL and/or Vulkan. They are here to stay and will only become more competent with time.

* SSBO's allows for passing mutable linear global memory to your shaders.
* Vulkan command buffers allows for recording commands once and replaying them multiple times, minimizing CPU overhead.
* NVIDIA refuse to embrace OpenCL in favor of their prioprietary CUDA alternative - but they do embrace Vulkan and OpenGL.

![](images/vulkan.png) ![](images/spir.png)

## Overview
The interface is still being designed. It will feature abstractions for devices, memory, buffers, shaders, etc.

```c++
DevicePool devicePool;
for (Device &device : devicePool.getDevices()) {
    cout << "Found device: " << device.getName() << " from vendor: 0x"
         << hex << device.getVendorId() << endl;

    // -- PREPARATIONS --

    // Load & compile a compute pipeline with 1 bound buffer
    Pipeline pipeline = device.pipeline("computeShader.spv", {
        BUFFER
    });
    
    // Allocate a 1 kb buffer, undefined content
    Buffer buffer = device.buffer(1024);
    
    // Create a command buffer and start recording commands to it
    CommandBuffer commandBuffer = device.commandBuffer();
    commandBuffer.start();

    // Use the pipeline & bind our buffer to its first binding point
    commandBuffer.usePipeline(pipeline, {buffer});

    // Enquque a dispatch (execution over a range)
    commandBuffer.dispatch(1024, 1, 1);

    // Ending the command buffer is separate from submitting it
    commandBuffer.end();
    
    // -- Start your timer here --
    
    // Submit the command buffer for execution on the GPU
    device.submit(commandBuffer);
    
    // Wait for completion
    device.drain();
    
    // -- End your timer here --
}
```
