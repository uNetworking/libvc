#include <CL/cl.h>

#include <iostream>
#include <chrono>
using namespace std;
using namespace chrono;

#define BUFFER_SIZE 10240
#define INCREMENT_PASSES 100000
#define RUNS 5

const char *source = "kernel void f(global double *output) { output[get_global_id(0)] += 1.0; }";

int main(int argc, char** argv)
{
    double data[BUFFER_SIZE] = {};

    cl_platform_id platforms[5];
    cl_uint numPlatforms = 0;
    clGetPlatformIDs(5, platforms, &numPlatforms);

    cl_device_id device_id;
    cl_uint numDevices = 0;
    for (int i = 0; i < 5; i++) {
        if (CL_SUCCESS == clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 1, &device_id, &numDevices)) {
            char deviceName[512];
            clGetDeviceInfo(device_id, CL_DEVICE_NAME, 512, deviceName, NULL);
            cout << "[" << deviceName << "]" << endl;
            break;
        }
    }

    cl_context context = clCreateContext(0, 1, &device_id, nullptr, nullptr, nullptr);
    cl_command_queue commands = clCreateCommandQueue(context, device_id, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, nullptr);
    cl_program program = clCreateProgramWithSource(context, 1, (const char **) & source, NULL, nullptr);
    clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    cl_kernel kernel = clCreateKernel(program, "f", nullptr);
    cl_mem output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(data), nullptr, nullptr);
    clEnqueueWriteBuffer(commands, output, CL_TRUE, 0, sizeof(data), data, 0, nullptr, nullptr);
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &output);

    size_t global = BUFFER_SIZE;
    size_t local = 1024;

    for (int i = 0; i < RUNS; i++) {
        steady_clock::time_point start = steady_clock::now();
        for (int i = 0; i < INCREMENT_PASSES; i++) {
            clEnqueueNDRangeKernel(commands, kernel, 1, nullptr, &global, &local, 0, nullptr, nullptr);
            clEnqueueBarrier(commands);
        }
        clFinish(commands);
        cout << i + 1 << ": " << duration_cast<milliseconds>(steady_clock::now() - start).count() << "ms" << endl;
    }

    clEnqueueReadBuffer(commands, output, CL_TRUE, 0, sizeof(data), data, 0, nullptr, nullptr);

    if (int(data[0] + 0.5) != INCREMENT_PASSES * RUNS) {
        cout << "Mismatching result!" << endl;
        return -3;
    }

    for (int i = 1; i < BUFFER_SIZE; i++) {
        if (data[i] != data[i - 1]) {
            cout << "Corruption at " << i << ": " << data[i] << " != " << data[i - 1] << endl;
            return -1;
        }
    }

    cout << "OK" << endl;

    clReleaseMemObject(output);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(commands);
    clReleaseContext(context);
    return 0;
}
