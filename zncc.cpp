#include <stdlib.h>
#include <CL/cl.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include "lodepng.h"
#define kernelpath "kernel.cl"
#define image1_path "im0.png"
#define image2_path "im1.png"

void CheckError(cl_int error)
{
	if (error != CL_SUCCESS) {
		std::cerr << "OpenCL call failed with error " << error << std::endl;
		

			std::exit(1);
	}
}







cl_program CreateProgram(const std::string& source,
	cl_context context)
{
	// http://www.khronos.org/registry/cl/sdk/1.1/docs/man/xhtml/clCreateProgramWithSource.html
	size_t lengths[1] = { source.size() };
	const char* sources[1] = { source.data() };

	cl_int error = 0;
	cl_program program = clCreateProgramWithSource(context, 1, sources, lengths, &error);
	CheckError(error);

	return program;
}

std::string LoadKernel(const char* name)
{
	std::ifstream in(name);
	std::string result(
		(std::istreambuf_iterator<char>(in)),
		std::istreambuf_iterator<char>());
	return result;
}



std::string GetDeviceName(cl_device_id id)
{
	size_t size = 0;
	clGetDeviceInfo(id, CL_DEVICE_NAME, 0, nullptr, &size);

	std::string result;
	result.resize(size);
	clGetDeviceInfo(id, CL_DEVICE_NAME, size,
		const_cast<char*> (result.data()), nullptr);

	return result;
}
std::string GetPlatformName(cl_platform_id id)
{
	size_t size = 0;
	clGetPlatformInfo(id, CL_PLATFORM_NAME, 0, nullptr, &size);

	std::string result;
	result.resize(size);
	clGetPlatformInfo(id, CL_PLATFORM_NAME, size,
		const_cast<char*> (result.data()), nullptr);

	return result;
}



int main(){
//get platform
	cl_uint platformIdCount = 0;
	clGetPlatformIDs(0, nullptr, &platformIdCount);

	if (platformIdCount == 0) {
		std::cerr << "No OpenCL platform found" << std::endl;
		return 1;
	}
	else {
		std::cout << "Found " << platformIdCount << " platform(s)" << std::endl;
	}

	std::vector<cl_platform_id> platformIds(platformIdCount);
	clGetPlatformIDs(platformIdCount, platformIds.data(), nullptr);

	for (cl_uint i = 0; i < platformIdCount; ++i) {
		std::cout << "\t (" << (i + 1) << ") : " << GetPlatformName(platformIds[i]) << std::endl;
	}
//get devices
	cl_uint deviceIdCount = 0;
	clGetDeviceIDs(platformIds[0], CL_DEVICE_TYPE_ALL, 0, nullptr,
		&deviceIdCount);

	if (deviceIdCount == 0) {
		std::cerr << "No OpenCL devices found" << std::endl;
		return 1;
	}
	else {
		std::cout << "Found " << deviceIdCount << " device(s)" << std::endl;
	}

	std::vector<cl_device_id> deviceIds(deviceIdCount);
	clGetDeviceIDs(platformIds[0], CL_DEVICE_TYPE_ALL, deviceIdCount,
		deviceIds.data(), nullptr);

	for (cl_uint i = 0; i < deviceIdCount; ++i) {
		std::cout << "\t (" << (i + 1) << ") : " << GetDeviceName(deviceIds[i]) << std::endl;
	}
//context
	const cl_context_properties contextProperties[] =
	{
		CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties> (platformIds[0]),
		0, 0
	};

	cl_int error = CL_SUCCESS;
	cl_context context = clCreateContext(contextProperties, deviceIdCount,
		deviceIds.data(), nullptr, nullptr, &error);
	CheckError(error);

	std::cout << "Context created" << std::endl;

// Create a program from source
	cl_program program = CreateProgram(LoadKernel(kernelpath),
		context);
	
	error = clBuildProgram(program, deviceIdCount, deviceIds.data(),
		nullptr, nullptr, nullptr);
	// Shows the log
	char* build_log;
	size_t log_size;
	// First call to know the proper size
	clGetProgramBuildInfo(program, deviceIds[1], CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
	build_log = new char[log_size + 1];
	// Second call to get the log
	clGetProgramBuildInfo(program, deviceIds[1], CL_PROGRAM_BUILD_LOG, log_size, build_log, NULL);
	build_log[log_size] = '\0';
	std::cout << build_log << std::endl;
	delete[] build_log;

	// http://www.khronos.org/registry/cl/sdk/1.1/docs/man/xhtml/clCreateKernel.html
	cl_kernel gskernel = clCreateKernel(program, "resizeandgreyscale", &error);
	CheckError(error);
	std::cout << "kernel created" << std::endl;

//image
	unsigned char* image;
	unsigned int width, height;
	lodepng_decode32_file(&image, &width, &height, image1_path);
	//Image image= load_image("im0.png");
	std::cout << "Image loaded" << std::endl;
//input format
	static const cl_image_format format = { CL_RGBA, CL_UNSIGNED_INT8 };
//output format
	static const cl_image_format oformat = { CL_R, CL_UNSIGNED_INT8 };
	
	
//input image
	cl_mem inputImage = clCreateImage2D(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR,
		&format, width, height, 0, image, &error);
	CheckError(error);
	
	free(image);
//create greyscale imageobjects
	cl_mem greyscale1 = clCreateImage2D(context, CL_MEM_WRITE_ONLY| CL_MEM_ALLOC_HOST_PTR,
		&oformat, width/4, height/4, 0, nullptr, &error);


// Setup the kernel arguments
	clSetKernelArg(gskernel, 0, sizeof(cl_mem), &inputImage);
	clSetKernelArg(gskernel, 1, sizeof(cl_mem), &greyscale1);
//Command queue
	cl_command_queue queue = clCreateCommandQueueWithProperties(context, deviceIds[0],
		0, &error);
	CheckError(error);

//Kernel
	cl_uint work_dimension = 2;
	const size_t work_offset = 0;
	const size_t global_worksize[2] = { width/4,height/4 };
	const size_t local_worksize = NULL;
	const cl_event event_wait_list = NULL;
	cl_uint num_events_in_wait_list = 0;
	cl_event event = NULL;

	
	CheckError(clEnqueueNDRangeKernel(queue, gskernel, work_dimension, work_offset, global_worksize, local_worksize,
		 num_events_in_wait_list, nullptr, &event));
	//release original image
	clReleaseMemObject(inputImage);
	//read image 2 
	unsigned char* image2;
	
	lodepng_decode32_file(&image2, &width, &height, image2_path);

	std::cout << "Image2 loaded" << std::endl;
	//load image 2 to imageobject
	cl_mem inputImage2 = clCreateImage2D(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR,
		&format, width, height, 0, image2, &error);
	CheckError(error);

	free(image2);
	cl_mem greyscale2 = clCreateImage2D(context, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR,
		&oformat, width / 4, height / 4, 0, nullptr, &error);

	// Setup the kernel arguments again
	clSetKernelArg(gskernel, 0, sizeof(cl_mem), &inputImage2);
	clSetKernelArg(gskernel, 1, sizeof(cl_mem), &greyscale2);
	//run kernel again
	CheckError(clEnqueueNDRangeKernel(queue, gskernel, work_dimension, work_offset, global_worksize, local_worksize,
		num_events_in_wait_list, nullptr, &event));
	//release original image2
	clReleaseMemObject(inputImage2);








//readimage
	unsigned char* output = (unsigned char*)malloc(735 * 504 * 4 + 1);
	std::size_t origin[3] = { 0 };
	std::size_t region[3] = { width / 4, height / 4, 1 };
	clEnqueueReadImage(queue, greyscale2, CL_TRUE,
		origin, region, 0, 0,
		output, 0, NULL, NULL);
//encode image for test purposes
	unsigned error3 = lodepng_encode_file("test.png", output, width / 4, height / 4, LCT_GREY, 8);
	//unsigned error1=lodepng::encode("test.png", output, width/4, height/4);
	const char* asd = lodepng_error_text(error3);
	std::cout << asd << std::endl;
//RELEASE
	
	free(output);
	clReleaseMemObject(greyscale1);


	clReleaseCommandQueue(queue);

	clReleaseKernel(gskernel);
	clReleaseProgram(program);

	clReleaseContext(context);
}
