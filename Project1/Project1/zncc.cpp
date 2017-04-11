//Defines


#include <stdlib.h>
#include <CL/cl.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include "lodepng.h"
#include <time.h>
#define kernelpath "kernel.cl"
#define image1_path "im0.png"
#define image2_path "im1.png"

//used to change device type 
#define DEVICE  CL_DEVICE_TYPE_CPU


//Initialization of the memoryobjects
cl_mem inputImage=0;
cl_mem inputImage2=0;
cl_mem greyscale1=0;
cl_mem greyscale2=0;
cl_mem disp_left=0;
cl_mem disp_right=0;
cl_mem processed=0;
cl_mem final_image=0;



//
//Ville Kemppainen & Mikko Paasimaa
//Working implementation without optimisation
//

//function which releases all memoryobjects in case of a crash
void release_mem_object()
{
	clReleaseMemObject(inputImage);
	clReleaseMemObject(inputImage2);
	clReleaseMemObject(greyscale1);
	clReleaseMemObject(greyscale2);
	clReleaseMemObject(disp_left);
	clReleaseMemObject(disp_right);
	clReleaseMemObject(processed);
	clReleaseMemObject(final_image);


}
//Error checking function
//function from Anterus blog
void CheckError(cl_int error)
{
	if (error != CL_SUCCESS) {
		std::cerr << "OpenCL call failed with error " << error << std::endl;
		
		release_mem_object();

			std::exit(1);
	}
}






//function from Anterus blog
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
//function from Anterus blog
std::string LoadKernel(const char* name)
{
	std::ifstream in(name);
	std::string result(
		(std::istreambuf_iterator<char>(in)),
		std::istreambuf_iterator<char>());
	return result;
}


//function from Anterus blog
std::string GetDeviceName(cl_device_id id)
{
	size_t size = 0;
	clGetDeviceInfo(id, CL_DEVICE_NAME, 0, NULL, &size);

	std::string result;
	result.resize(size);
	clGetDeviceInfo(id, CL_DEVICE_NAME, size,
		const_cast<char*> (result.data()), NULL);

	return result;
}
//function from Anterus blog
std::string GetPlatformName(cl_platform_id id)
{
	size_t size = 0;
	clGetPlatformInfo(id, CL_PLATFORM_NAME, 0, NULL, &size);

	std::string result;
	result.resize(size);
	clGetPlatformInfo(id, CL_PLATFORM_NAME, size,
		const_cast<char*> (result.data()), NULL);

	return result;
}
//function used for timing specific events with clGetEventProfilingInfo
void Time(cl_event event)
{
	//function initialization
	cl_ulong time_start, time_end;
	double total_time;
	//check that the event has ended before timing it	
	clWaitForEvents(1, &event);
	//get start and end times for the event
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
	
	total_time = time_end - time_start;
	printf("Execution time in milliseconds = %0.3f ms\n", (total_time / 1000000.0));

}



int main(){

// PLATFORM
	
	cl_uint platformIdCount = 0;
	clGetPlatformIDs(0, NULL, &platformIdCount);

	if (platformIdCount == 0) {
		std::cerr << "No OpenCL platform found" << std::endl;
		return 1;
	}
	else {
		std::cout << "Found " << platformIdCount << " platform(s)" << std::endl;
	}

	std::vector<cl_platform_id> platformIds(platformIdCount);
	clGetPlatformIDs(platformIdCount, platformIds.data(), NULL);

	for (cl_uint i = 0; i < platformIdCount; ++i) {
		std::cout << "\t (" << (i + 1) << ") : " << GetPlatformName(platformIds[i]) << std::endl;
	}

// DEVICE

	cl_uint deviceIdCount = 0;
	clGetDeviceIDs(platformIds[0], DEVICE, 0, NULL,
		&deviceIdCount);

	if (deviceIdCount == 0) {
		std::cerr << "No OpenCL devices found" << std::endl;
		return 1;
	}
	else {
		std::cout << "Found " << deviceIdCount << " device(s)" << std::endl;
	}

	std::vector<cl_device_id> deviceIds(deviceIdCount);
	clGetDeviceIDs(platformIds[0], DEVICE, deviceIdCount,
		deviceIds.data(), NULL);

	for (cl_uint i = 0; i < deviceIdCount; ++i) {
		std::cout << "\t (" << (i + 1) << ") : " << GetDeviceName(deviceIds[i]) << std::endl;
	}
	std::cout << "\n\nRun time log:" << std::endl;

// CONTEXT
	std::cout << "Context: ";
	const cl_context_properties contextProperties[] =
	{
		CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties> (platformIds[0]),
		0, 0
	};

	cl_int error = CL_SUCCESS;
	cl_context context = clCreateContext(contextProperties, deviceIdCount,
		deviceIds.data(), NULL, NULL, &error);
	CheckError(error);

	std::cout << "Done" << std::endl;

// PROGRAM
	std::cout << "Building program: ";
	cl_program program = CreateProgram(LoadKernel(kernelpath),
		context);
	
	error = clBuildProgram(program, deviceIdCount, deviceIds.data(),
		"-cl-fast-relaxed-math", NULL, NULL);
	
	std::cout << "Done" << std::endl;
	
	
	// Shows program building log. Used for debugging build errors
	char* build_log;
	size_t log_size;
	// First call to know the proper size
	clGetProgramBuildInfo(program, deviceIds[0], CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
	build_log = new char[log_size + 1];
	// Second call to get the log
	clGetProgramBuildInfo(program, deviceIds[0], CL_PROGRAM_BUILD_LOG, log_size, build_log, NULL);
	build_log[log_size] = '\0';
	std::cout << build_log << std::endl;
	delete[] build_log;
	

//Execution timer start
	clock_t t1, t2;
	t1 = clock();


// Kernel creation
	std::cout << "Kernel creation: ";

	cl_kernel gskernel = clCreateKernel(program, "resizeandgreyscale", &error);
	CheckError(error);
	cl_kernel zncc_left = clCreateKernel(program, "zncc_left", &error);
	CheckError(error);
	cl_kernel zncc_right = clCreateKernel(program, "zncc_right", &error);
	CheckError(error);
	cl_kernel post_process = clCreateKernel(program, "post_process", &error);
	CheckError(error);
	cl_kernel occlusion = clCreateKernel(program, "occlusion", &error);
	CheckError(error);
	std::cout << "Done" << std::endl;

// Load input image 1 with lodepng
	unsigned char* image;
	unsigned int width, height;
	lodepng_decode32_file(&image, &width, &height, image1_path);
	
	std::cout << "Image 1 loaded" << std::endl;

// Input image objects format
	static const cl_image_format format = { CL_RGBA, CL_UNSIGNED_INT8 };
// Output image objects format
	static const cl_image_format oformat = { CL_R, CL_UNSIGNED_INT8 };
	
	
// Create image object for input image
	cl_mem inputImage = clCreateImage2D(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR,
		&format, width, height, 0, image, &error);
	CheckError(error);
	//free the original image loaded with lodepng becouse it is  no longer used anywhere
	free(image);

// Create image object for greyscale image 1
	cl_mem greyscale1 = clCreateImage2D(context, CL_MEM_READ_WRITE| CL_MEM_ALLOC_HOST_PTR,
		&oformat, width/4, height/4, 0, NULL, &error);

// Command queue
	std::cout << "Command queue creation: ";
	cl_command_queue queue = clCreateCommandQueue(context, deviceIds[0], CL_QUEUE_PROFILING_ENABLE, &error);
	CheckError(error);
	std::cout << "Done" << std::endl;
// Kernel
	//setting up variables used on running the kernel
	cl_uint work_dimension = 2;
	const size_t work_offset = 0;
	const size_t global_worksize[2] = { width/4,height/4 };
	const size_t local_worksize = NULL;
	const cl_event event_wait_list=NULL;
	cl_uint num_events_in_wait_list = 0;
	cl_event gskernel_event;

	// Setup the kernel arguments for gskernel
	clSetKernelArg(gskernel, 0, sizeof(cl_mem), &inputImage);
	clSetKernelArg(gskernel, 1, sizeof(cl_mem), &greyscale1);

	std::cout << "Greyscale and resize kernel for image 1: ";
	CheckError(clEnqueueNDRangeKernel(queue, gskernel, work_dimension, work_offset, global_worksize, local_worksize,
		 num_events_in_wait_list, NULL, &gskernel_event));
	std::cout << "Done" << std::endl;

	// wait for gskernel to stop executing before continuing
	clFinish(queue);
	// release original images imageobject
	clReleaseMemObject(inputImage);

// Load input image 2 with lodepng
	unsigned char* image2;
	
	lodepng_decode32_file(&image2, &width, &height, image2_path);

	std::cout << "Image2 loaded" << std::endl;
	// Create imageobject for image 2
	cl_mem inputImage2 = clCreateImage2D(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR,
		&format, width, height, 0, image2, &error);
	CheckError(error);
	// Free image 2 as it is nolonger used
	free(image2);
	// Greyscale and resize image 2
	cl_mem greyscale2 = clCreateImage2D(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
		&oformat, width / 4, height / 4, 0, NULL, &error);
	
	// Setup the kernel arguments for gskernel to use image 2 instead of image 1
	clSetKernelArg(gskernel, 0, sizeof(cl_mem), &inputImage2);
	clSetKernelArg(gskernel, 1, sizeof(cl_mem), &greyscale2);
	std::cout << "Greyscale kernel for image 2: ";
	// run kernel again
	CheckError(clEnqueueNDRangeKernel(queue, gskernel, work_dimension, work_offset, global_worksize, local_worksize,
		0, NULL, &gskernel_event));
	std::cout << "Done" << std::endl;
	
	// Wait for kernel to stop executing before continuing
	clFinish(queue);
	//release original image2 image object 
	clReleaseMemObject(inputImage2);

//ZNCC_LEFT
	std::cout << "Zncc_left kernel: " ;
	// create image object for the  output image produced by the zncc_left kernel
	cl_mem disp_left = clCreateImage2D(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
		&oformat, width/4, height/4, 0, NULL, &error);
	CheckError(error);
	// set kernel arguments
	clSetKernelArg(zncc_left, 0, sizeof(cl_mem), &greyscale1);
	clSetKernelArg(zncc_left, 1, sizeof(cl_mem), &greyscale2);
	clSetKernelArg(zncc_left, 2, sizeof(cl_mem), &disp_left);
	
	cl_event zncc_left_event;
	// run kernel
	CheckError(clEnqueueNDRangeKernel(queue, zncc_left, work_dimension, work_offset, global_worksize, local_worksize,
		0, NULL, &zncc_left_event));
	
	std::cout << "Done" << std::endl;
	

//ZNCC_RIGHT
	std::cout << "Zncc_right kernel: " ;
	cl_mem disp_right = clCreateImage2D(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
		&oformat, width / 4, height / 4, 0, NULL, &error);
	CheckError(error);
	
	// Set kernel arguments
	clSetKernelArg(zncc_right, 0, sizeof(cl_mem), &greyscale2);
	clSetKernelArg(zncc_right, 1, sizeof(cl_mem), &greyscale1);
	clSetKernelArg(zncc_right, 2, sizeof(cl_mem), &disp_right);
	cl_event zncc_right_event;
	// run kernel
	CheckError(clEnqueueNDRangeKernel(queue, zncc_right, work_dimension, work_offset, global_worksize, local_worksize,
		0, NULL, &zncc_right_event));
	
	//wait for both zncc kernels to stop before continuing to the post processing
	clFinish(queue);

	//release greyscale images
	clReleaseMemObject(greyscale1);
	clReleaseMemObject(greyscale2);
	std::cout << "Done" << std::endl;

// POST_PROCESS
	std::cout << "Post process: ";
	// create image object for the processed image
	cl_mem processed = clCreateImage2D(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
		&oformat, width / 4, height / 4, 0, NULL, &error);
	CheckError(error);
	// set up kernel arguments
	clSetKernelArg(post_process, 0, sizeof(cl_mem), &disp_left);
	clSetKernelArg(post_process, 1, sizeof(cl_mem), &disp_right);
	clSetKernelArg(post_process, 2, sizeof(cl_mem), &processed);

	cl_event post_process_event;
	//run kernel
	CheckError(clEnqueueNDRangeKernel(queue, post_process, work_dimension, work_offset, global_worksize, local_worksize,
		0, NULL, &post_process_event));

	clFinish(queue);
	// Release disparity images
	clReleaseMemObject(disp_left); 
	clReleaseMemObject(disp_right);

	std::cout << "Done" << std::endl;

// OCCLUSION
	std::cout << "Occlusion filling: " ;
	//create image object for the final image
	cl_mem final_image = clCreateImage2D(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
		&oformat, width / 4, height / 4, 0, NULL, &error);
	CheckError(error);
	//kernel arguments for the occlusion filling kernel
	clSetKernelArg(occlusion, 0, sizeof(cl_mem), &processed);
	clSetKernelArg(occlusion, 1, sizeof(cl_mem), &final_image);

	cl_event occlusion_event;
	// run kernel
	CheckError(clEnqueueNDRangeKernel(queue, occlusion, work_dimension, work_offset, global_worksize, local_worksize,
		0, NULL, &occlusion_event));
	
	clFinish(queue);
	//release processed image
	clReleaseMemObject(processed);
	std::cout << "Done" << std::endl;

// Read image from the host
	std::cout << "Reading image" << std::endl;
	unsigned char* output = (unsigned char*)malloc(735 * 504);
	std::size_t origin[3] = { 0 };
	std::size_t region[3] = { width / 4, height / 4, 1 };
	clEnqueueReadImage(queue, final_image, CL_TRUE,
		origin, region, 0, 0,
		output, 0, NULL, NULL);
	//waits until clEnqueueReadImage is done
	clFinish(queue);

//Creating the final image
	std::cout << "Creating final image" << std::endl;
	unsigned error3 = lodepng_encode_file("test1.png", output, width / 4, height / 4, LCT_GREY, 8);
	//show lodepng error messages
	const char* asd = lodepng_error_text(error3);
	std::cout <<"Lodepng:" << asd << std::endl;

// Time kernels using Time() function
	std::cout << "\n\nPost execution log:" << std::endl;
	std::cout << "Greyscale and resize kernel:" << std::endl;
	Time(gskernel_event);
	std::cout << "Zncc_left kernel:" << std::endl;
	Time(zncc_left_event);
	std::cout << "Zncc_right kernel:" << std::endl;
	Time(zncc_right_event);
	std::cout << "Postprocess kernel:" << std::endl;
	Time(post_process_event);
	std::cout << "Occlusion kernel:" << std::endl;
	Time(occlusion_event);

// Release all the remaining objects
	// images
	free(output);
	clReleaseMemObject(final_image);
	// command queue
	clReleaseCommandQueue(queue);
	// kernels
	clReleaseKernel(gskernel);
	clReleaseKernel(zncc_left);
	clReleaseKernel(zncc_right);
	clReleaseKernel(post_process);
	clReleaseKernel(occlusion);
	// program
	clReleaseProgram(program);
	// context
	clReleaseContext(context);

	// end timer and show result
	t2 = clock();
	float diff(((float)t2 - (float)t1) / CLOCKS_PER_SEC);
	std::cout << "Total execution time:"<< diff <<"s" << std::endl;



}


