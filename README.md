# Vulkan Compute for C++
```libvc``` is a GPGPU engine based on Vulkan. It eats SPIR-V compute shaders and executes them without any graphical context, much like ```OpenCL```. The interface is very abstract and allows you to get to work with your shaders as quickly as possible. With the current state of compute shaders, you should be able to achieve ~OpenCL 1.2 feature parity.
![](vulkan.png)

## Overview
The interface is still being designed. It will feature abstractions for devices, memory, buffers, shaders, etc.

```c++
DevicePool devicePool;
for (Device &device : devicePool.getDevices()) {
    cout << "Found device: " << device.getName() << " from vendor: 0x"
         << hex << device.getVendorId() << endl;

    Memory mem = device.memory(1000 * sizeof(int));
    int *map = (int *) mem.map();

    for (int i = 0; i < 1000; i++) {
        map[i] = i;
    }

    mem.unmap();
    
    // load / compile shader
    Shader shader = device.loadShader("/home/alexhultman/computeShader.spr");

    // bind shader to current command buffer (currently hidden in device)
    device.useShader(shader);

    // enquque a range to the current command buffer
    device.dispatch(1000, 1, 1);

    // end the command buffer and submit it, wait for it to finish, time it
    device.drain();
}
```
