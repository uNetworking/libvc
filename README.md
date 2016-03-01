# libvc: Vulkan Compute for C++
This project is about wrapping the (purely) compute related functionalities of Vulkan. Even though Vulkan is built for both graphics and compute, some applications only rely on GPU computations and need no graphics presentation functionalities. Instead of targeting OpenCL, which has not been fully embraced by NVIDIA, you could aim for the use of Vulkan compute shaders to achieve a more performant OpenCL 1.2 functionality parity.  
![](vulkan.png)

## Overview
The interface is still being designed. It will feature abstractions for devices, memory, buffers, shaders, etc.
