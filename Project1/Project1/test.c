#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <stdlib.h>
#include "lodepng.h"
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define MEM_SIZE (128)
#define MAX_SOURCE_SIZE (0x100000)

int main3()
{
	cl_device_id device_id = NULL;
	cl_context context = NULL;
	cl_command_queue command_queue = NULL;
	cl_mem memobj = NULL;
	cl_program program = NULL;
	cl_kernel kernel = NULL;
	cl_platform_id platform_id = NULL;
	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;
	cl_int ret;

	char string[MEM_SIZE];

	FILE *fp;
	char fileName[] = "./hello.cl";
	char *source_str;
	size_t source_size;

	/* Load the source code containing the kernel*/
	fp = fopen(fileName, "r");
	if (!fp) {
		fprintf(stderr, "Failed to load kernel.\n");
		exit(1);
	}
	fseek(fp, 0, SEEK_END);
	source_size = ftell(fp);
	rewind(fp);

	source_str = (char*)malloc(source_size+1);
	source_str[source_size] = "\0";
	fread(source_str, sizeof(char), source_size, fp);

	fclose(fp);

	/* Get Platform and Device Info */
	ret = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
	if (ret != CL_SUCCESS)
	{
		printf("error %d", ret);

	}
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);
	if (ret != CL_SUCCESS)
	{
		printf("error %d", ret);

	}
	/* Create OpenCL context */
	context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
	if (ret != CL_SUCCESS)
	{
		printf("error %d", ret);

	}
	/* Create Command Queue */
	command_queue = clCreateCommandQueue(context, device_id, NULL, &ret);
	if (ret != CL_SUCCESS)
	{
		printf("error %d", ret);

	}
	/* Create Memory Buffer */
	//memobj = clCreateBuffer(context, CL_MEM_READ_WRITE, MEM_SIZE * sizeof(char), NULL, &ret);

	//Create image object
	//read image
	unsigned char* image1;
	unsigned error;
	unsigned width, height;

	printf("\nAvataan kuva 1:");
	const char* filename1 = "im0.png";
	error = lodepng_decode32_file(&image1, &width, &height, filename1);
	if (error) printf("error %u: %s\n", error, lodepng_error_text(error));





	//int width = 2940;
	//int height = 2016;
	int pixelSize = 4;
//Image buffers



	//Input image objectin luonti
	cl_image_desc imageDesc1;
	//Bufferi
	cl_mem inputImageBuffer = clCreateBuffer(context,
		CL_MEM_READ_ONLY,
		width * height * pixelSize, &image1, NULL);
	//Imageformatin luonti
	cl_image_format imageFormat;
	imageFormat.image_channel_order = CL_RGBA;
	imageFormat.image_channel_data_type = CL_UNSIGNED_INT32;
	//image descin luonti
	imageDesc1.image_type = CL_MEM_OBJECT_IMAGE1D_ARRAY;
	imageDesc1.image_width = width;
	imageDesc1.image_height = height;
	
	imageDesc1.buffer = inputImageBuffer;
	imageDesc1.image_depth = 1;
	imageDesc1.image_array_size = 0;
	imageDesc1.image_row_pitch = 0;
	imageDesc1.image_slice_pitch = 0;
	imageDesc1.num_mip_levels = 0;
	imageDesc1.num_samples = 0;

	cl_mem inputImage = clCreateImage(context,
		CL_MEM_READ_ONLY,
		&imageFormat,
		&imageDesc1,/*Buffer passed as argument */
		NULL,
		NULL);

	//Output imagen luonti
	cl_image_desc imageDesc2;
	cl_mem outputImageBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE,
		width / 4 * height / 4 * pixelSize / 4, 0, ret);
	if (ret != CL_SUCCESS)
	{
		printf("error %d", ret);

	}

	cl_image_format imageFormat2;
	imageFormat2.image_channel_order = CL_RGBA;
	imageFormat2.image_channel_data_type = CL_UNSIGNED_INT32;

	imageDesc2.image_type = CL_MEM_OBJECT_IMAGE1D_ARRAY;
	imageDesc2.image_width = width;
	imageDesc2.image_height = height;

	imageDesc2.buffer = inputImageBuffer;
	imageDesc2.image_depth = 1;
	imageDesc2.image_array_size = 0;
	imageDesc2.image_row_pitch = 0;
	imageDesc2.image_slice_pitch = 0;
	imageDesc2.num_mip_levels = 0;
	imageDesc2.num_samples = 0;

	cl_mem OutputImage = clCreateImage(context,
		CL_MEM_READ_WRITE,
		&imageFormat2,
		&imageDesc2,/*Buffer passed as argument */
		NULL,
		ret);
	if (ret != CL_SUCCESS)
	{
		printf("error %d", ret);

	}

	//
	/* Create Kernel Program from the source */
	program = clCreateProgramWithSource(context, 1, (const char **)&source_str,
		&source_size, &ret);
	if (ret != CL_SUCCESS)
	{
		printf("error %d", ret);

	}
	/* Build Kernel Program */
	ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
	if (ret != CL_SUCCESS)
	{
		printf("error %d", ret);

	}
	/* Create OpenCL Kernel */
	kernel = clCreateKernel(program, "hello", &ret);
	if (ret != CL_SUCCESS)
	{
		printf("error %d", ret);

	}
	/* Set OpenCL Kernel Parameters */
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), &inputImageBuffer);
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), &outputImageBuffer);
	/* Execute OpenCL Kernel */
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, 1,1,0,NULL, NULL);
	
	if (ret != CL_SUCCESS)
	{
		printf("error %d",ret);

	}


	unsigned char* test_image = (char*)malloc(1481760);
	/* Copy results from the memory buffer */
	ret = clEnqueueReadBuffer(command_queue, outputImageBuffer, CL_TRUE, 0,
		height/4*width/4, test_image, 0, NULL, NULL);

	const char* filename_final;
	filename_final = "test1.png";

	unsigned error1 = lodepng_encode_file(filename_final, test_image, width / 4, height / 4, LCT_GREY, 8);

	/* Display Result */
	//puts(string);

	/* Finalization */
	ret = clReleaseMemObject(outputImageBuffer);
	ret = clReleaseMemObject(inputImageBuffer);
	ret = clReleaseMemObject(inputImage);
	ret = clReleaseMemObject(OutputImage);

	ret = clFlush(command_queue);
	ret = clFinish(command_queue);
	ret = clReleaseKernel(kernel);
	ret = clReleaseProgram(program);
	ret = clReleaseMemObject(memobj);
	ret = clReleaseCommandQueue(command_queue);
	ret = clReleaseContext(context);

	free(source_str);

	return 0;
}