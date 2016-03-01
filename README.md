# libvc: Vulkan Compute for C++
This project is about wrapping the (purely) compute related functionalities of Vulkan. Even though Vulkan is built for both graphics and compute, some applications only rely on GPU computations and need no graphics presentation functionalities. Instead of targeting OpenCL, which has not been fully embraced by NVIDIA, you could aim for the use of Vulkan compute shaders to achieve a more performant OpenCL 1.2 functionality parity.  
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
