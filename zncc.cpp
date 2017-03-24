#include <stdlib.h>
#include <CL/cl.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include "lodepng.h"
#define kernelpath "hello.cl"
#define IMAGE1 "im0.png";
void CheckError(cl_int error)
{
	if (error != CL_SUCCESS) {
		std::cerr << "OpenCL call failed with error " << error << std::endl;
		

		std::exit(1);
	}
}

void decode(const char *filename, unsigned &width, unsigned &height, std::vector<unsigned char> &image) {
	lodepng::decode(image, width, height, filename);
}

struct Image {
	int height, width;
	std::vector<unsigned char> pixel;
};

Image load_image(const char *filename) {
	unsigned original_width, original_height;
	std::vector<unsigned char> image = std::vector<unsigned char>();
	decode(filename, original_width, original_height, image);
	Image img;
	img.height = original_height;
	img.width = original_width;
	img.pixel = image;
	return img;
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
	cl_kernel kernel = clCreateKernel(program, "hello", &error);
	CheckError(error);
	std::cout << "kernel created" << std::endl;

	//image
	unsigned char* image;
	unsigned int width, height;
	lodepng_decode32_file(&image, &width, &height, "im0.png");
	//Image image= load_image("im0.png");
	std::cout << "Image loaded" << std::endl;
	static const cl_image_format format = { CL_RGBA, CL_UNSIGNED_INT32 };
	
	cl_image_desc desc;
 
	size_t h = height;
	size_t w = width;
	size_t d = 1;
	size_t r = width * 4;
	
	desc.image_type = CL_MEM_OBJECT_IMAGE2D;
	desc.image_width = w;
	desc.image_height = h;
	desc.image_depth = d;
	desc.image_row_pitch = r;

	
	cl_mem inputImage = clCreateImage(context, CL_MEM_READ_ONLY|CL_MEM_ALLOC_HOST_PTR |CL_MEM_COPY_HOST_PTR, &format,
		&desc,
		image,
		&error);
	CheckError(error);



}
