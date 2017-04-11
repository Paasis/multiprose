#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#include <CL/cl.h>
#include <stdio.h>
//
//int main()
//{
//    // 1. Get a platform.
//    cl_platform_id platform;
//    clGetPlatformIDs(1, &platform, NULL);
//    // 2. Find a gpu device.
//    cl_device_id device;
//    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU,
//        1,
//        &device,
//        NULL);
//
//    // 3. Create a context and command queue on that device.
//    cl_context context = clCreateContext(NULL,
//        1,
//        &device,
//        NULL, NULL, NULL);
//    cl_command_queue queue = clCreateCommandQueue(context,
//        device,
//        0, NULL);
//}

// https://gist.github.com/courtneyfaulkner/7919509 List openCL platforms and devices.

int main1() {

    int i, j;
    char* value;
    size_t valueSize;
    cl_uint platformCount;
    cl_platform_id* platforms;
    cl_uint deviceCount;
    cl_device_id* devices;
    cl_uint maxComputeUnits;
    cl_device_local_mem_type mem_type;
    cl_ulong local_mem_size;
    cl_uint c_units;
    cl_uint c_freq;
    cl_ulong c_buffer;
	cl_uint d_buffer;
    size_t group_size;
    size_t item_sizes;

    // get all platforms
	
    clGetPlatformIDs(0, NULL, &platformCount);
    platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * platformCount);
    clGetPlatformIDs(platformCount, platforms, NULL);

    //print platform info
    clGetPlatformInfo(platforms[0], CL_PLATFORM_VENDOR, 0, NULL, &valueSize);
    value = (char*)malloc(valueSize);
    clGetPlatformInfo(platforms[0], CL_PLATFORM_VENDOR, valueSize, value, NULL);
    printf("Platform vendor: %s\n", value);
    free(value);


    for (i = 0; i < platformCount; i++) {

        // get all devices
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &deviceCount);
        devices = (cl_device_id*)malloc(sizeof(cl_device_id) * deviceCount);
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, deviceCount, devices, NULL);

 

        // for each device print critical attributes
        for (j = 0; j < deviceCount; j++) {

            // print device name
            clGetDeviceInfo(devices[j], CL_DEVICE_NAME, 0, NULL, &valueSize);
            value = (char*)malloc(valueSize);
            clGetDeviceInfo(devices[j], CL_DEVICE_NAME, valueSize, value, NULL);
            printf("%d. Device: %s\n", j + 1, value);
            free(value);

            // print hardware device version
            clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, 0, NULL, &valueSize);
            value = (char*)malloc(valueSize);
            clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, valueSize, value, NULL);
            printf(" %d.%d Hardware version: %s\n", j + 1, 1, value);
            free(value);

            // print software driver version
            clGetDeviceInfo(devices[j], CL_DRIVER_VERSION, 0, NULL, &valueSize);
            value = (char*)malloc(valueSize);
            clGetDeviceInfo(devices[j], CL_DRIVER_VERSION, valueSize, value, NULL);
            printf(" %d.%d Software version: %s\n", j + 1, 2, value);
            free(value);

            // print c version supported by compiler for device
            clGetDeviceInfo(devices[j], CL_DEVICE_OPENCL_C_VERSION, 0, NULL, &valueSize);
            value = (char*)malloc(valueSize);
            clGetDeviceInfo(devices[j], CL_DEVICE_OPENCL_C_VERSION, valueSize, value, NULL);
            printf(" %d.%d OpenCL C version: %s\n", j + 1, 3, value);
            free(value);

           

            // print CL_DEVICE_LOCAL_MEM_TYPE
            clGetDeviceInfo(devices[j], CL_DEVICE_LOCAL_MEM_TYPE,
                sizeof(mem_type), &mem_type, NULL);
            printf(" %d.%d local_mem_type: %d\n", j + 1, 4, mem_type);

            // print local mem size
            clGetDeviceInfo(devices[j], CL_DEVICE_LOCAL_MEM_SIZE,
                sizeof(local_mem_size), &local_mem_size, NULL);
            printf(" %d.%d local_mem_size: %d\n", j + 1, 5, local_mem_size);

            // print parallel compute units
            clGetDeviceInfo(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS,
                sizeof(maxComputeUnits), &maxComputeUnits, NULL);
            printf(" %d.%d Parallel compute units: %d\n", j + 1, 6, maxComputeUnits);

            // print clock freq
            clGetDeviceInfo(devices[j], CL_DEVICE_MAX_CLOCK_FREQUENCY,
                sizeof(c_freq), &c_freq, NULL);
            printf(" %d.%d frequency: %d\n", j + 1, 7, c_freq);

            // print buffer size
            clGetDeviceInfo(devices[j], CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE,
                sizeof(c_buffer), &c_buffer, NULL);
            printf(" %d.%d buffer size: %d\n", j + 1, 8, c_buffer);

			// print max workgroup size
			clGetDeviceInfo(devices[j], CL_DEVICE_MAX_WORK_GROUP_SIZE,
				sizeof(c_buffer), &c_buffer, NULL);
			printf(" %d.%d Workgroup max size: %d\n", j + 1, 9, c_buffer);


			clGetDeviceInfo(devices[j], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,
				sizeof(d_buffer), &d_buffer, NULL);
			size_t e_buffer[sizeof(d_buffer)];
			int i;
			// print max work item size
			clGetDeviceInfo(devices[j], CL_DEVICE_MAX_WORK_ITEM_SIZES,
				sizeof(e_buffer), &e_buffer, NULL);

				for(i = 0; i < d_buffer; i++)
				{
					printf(" %d.%d Work item max size for dimension %d: %d\n", j + 1, 10,i+1, e_buffer[i]);

				}
			




        }

        free(devices);

    }

    free(platforms);
    

	
	return 0;

}


