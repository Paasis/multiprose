//
//Ville Kemppainen & Mikko Paasimaa
//Working implementation without optimisation
//Bad commentry
//

#define WINDOWSIZE 9
#define THRESHOLD 8


__constant sampler_t sampler =
  CLK_NORMALIZED_COORDS_FALSE
| CLK_ADDRESS_CLAMP_TO_EDGE
| CLK_FILTER_NEAREST;



//*************************************************************************************************************************
//Kernel for resizing and greyscaling image
__kernel void resizeandgreyscale(__read_only image2d_t input,
								__write_only image2d_t output)
{

int x = get_global_id(0);
int y = get_global_id(1);

int2 coord = {x*4, y*4};
int2 coord2 ={x,y};
uint4 pixel = read_imageui(input, sampler, coord);
uint grey = (uint) (pixel.x * 0.2126 + pixel.y * 0.7152 + pixel.z * 0.0722);

write_imageui(output ,coord2 , grey);
}

//*************************************************************************************************************************
//Kernel for zncc on left image
__kernel void zncc_left(__read_only image2d_t greyscale1,
								__read_only image2d_t greyscale2,
								__write_only image2d_t disp)
{
int x = get_global_id(0);
int y = get_global_id(1);

char window=WINDOWSIZE;
char ndisp = 65;

unsigned short t i,j,d,best_d=0;
float mean1=0,mean2=0;
float std1=0,std2=0;
float z=0, zncc=0, max_sum=0;
int2 coord2 = { x,y };


for(d=0;d<=ndisp;d=d+1){
	//mean value loops
	for(j=y;j<=y+window;j=j+1){
		for(i=x;i<=x+window;i=i+1){

			//get mean value for window	
            mean1=mean1+read_imageui(greyscale1,sampler, (int2) { i, j }).x;
            mean2=mean2+read_imageui(greyscale2,sampler,(int2){i-d,j}).x;
		}
	}
								
		mean1=mean1/(window);
		mean2=mean2/(window);

	//standard deviation values
	for(j=y;j<=y+window;j=j+1){
		for(i=x;i<=x+window;i=i+1){
			std1=std1+(read_imageui(greyscale1,sampler,(int2){i,j}).x-mean1)*(read_imageui(greyscale1,sampler, (int2) {i,i}).x-mean1);
            std2=std2+(read_imageui(greyscale2,sampler, (int2) {i-d,j}).x-mean2)*(read_imageui(greyscale2,sampler, (int2) {i-d,j}).x-mean2);
             
			//z value            
            z=z+((read_imageui(greyscale1,sampler, (int2) {i,j}).x-mean1)*(read_imageui(greyscale2,sampler, (int2) {i-d,j}).x-mean2));
                            
		}
	}
	

     std1=native_sqrt(std1/(window));
     std2=native_sqrt(std2/(window));
     zncc=z/(std1*std2);

				     
	 //compare zncc to previous best value and replace it if better
     if(zncc>max_sum){
		max_sum=zncc;
		best_d=d;
     }
	//Resets the values for the next d cycle
	mean1 = 0;
	mean2 = 0;
	std1 = 0;
	std2 = 0;
	zncc = 0;
	z = 0;
	}
//writes the value to output image
write_imageui(disp,coord2,best_d);  
}

//*************************************************************************************************************************
//Kernel for zncc on right image
__kernel void zncc_right(__read_only image2d_t greyscale1,
								__read_only image2d_t greyscale2,
								__write_only image2d_t disp)
{
int x = get_global_id(0);
int y = get_global_id(1);
char window=WINDOWSIZE;
char ndisp = 65;
unsigned short i,j,d,best_d=0;
float mean1=0,mean2=0;
float std1=0,std2=0;
float z=0, zncc=0, max_sum=0;
int2 coord2 = { x,y };

for(d=0;d<=ndisp;d=d+1){
	//mean value loops	
	for(j=y;j<=y+window;j=j+1){
		for(i=x;i<=x+window;i=i+1){
            //get mean value for window		
            mean1=mean1+read_imageui(greyscale1,sampler, (int2) { i, j }).x;
            mean2=mean2+read_imageui(greyscale2,sampler,(int2){i+d,j}).x;
        }
	}
	
		mean1=mean1/window;
		mean2=mean2/window;

	//standard deviation values
	for(j=y;j<=y+window;j=j+1){
		for(i=x;i<=x+window;i=i+1){
            std1=std1+(read_imageui(greyscale1,sampler,(int2){i,j}).x-mean1)*(read_imageui(greyscale1,sampler, (int2) {i,i}).x-mean1);
            std2=std2+(read_imageui(greyscale2,sampler, (int2) {i+d,j}).x-mean2)*(read_imageui(greyscale2,sampler, (int2) {i+d,j}).x-mean2);
             //z value           
            z=z+((read_imageui(greyscale1,sampler, (int2) {i,j}).x-mean1)*(read_imageui(greyscale2,sampler, (int2) {i+d,j}).x-mean2));
        }
	}
	
	std1=native_sqrt(std1/window);
	std2=native_sqrt(std2/window);
	zncc=z/(std1*std2);

	//compare zncc to previous best value and replace it if better	
	if(zncc>max_sum){
		max_sum=zncc;
		best_d=d;
	}
	//Resets the values for the next d cycle
	mean1 = 0;
	mean2 = 0;
	std1 = 0;
	std2 = 0;
	zncc = 0;
	z = 0;
	}
//writes the value to output image
write_imageui(disp,coord2,best_d);  
}


//*************************************************************************************************************************
//post process
__kernel void post_process(__read_only image2d_t disp_left,
								__read_only image2d_t disp_right,
								__write_only image2d_t processed)
{
	int x = get_global_id(0);
	int y = get_global_id(1);


	char threshold=THRESHOLD;

	int2 coord = {x, y};
	//compares the disparity values of the images to threshold value. If higher than threshold, value is set to 0. If within the threshold, value is set to value of left image.
	if(abs(read_imageui(disp_left, sampler, coord).x - read_imageui(disp_right, sampler, (int2){x - read_imageui(disp_left, sampler, coord).x, y}).x) > (uint)threshold)
	{
		write_imageui(processed, coord, 0);
	}
	else
	{
		write_imageui(processed, coord, read_imageui(disp_left, sampler, coord).x);
	}
}


//*************************************************************************************************************************
//occlusion filling
__kernel void occlusion(__read_only image2d_t processed,
						__write_only image2d_t final_image)
{
	int x = get_global_id(0);
	int y = get_global_id(1);

	int2 coord = {x, y};
	char i, searchrange=50;
	//finds the nearest non-zero value for the zeroes in the image in x and y directions. Multiplies the value to make it look brighter (from 0-65 scale to 0-255)
	if(read_imageui(processed, sampler, coord).x != 0){
		write_imageui(final_image, coord, 3.938*read_imageui(processed, sampler, coord).x);
	}
	else
	{
		for(i = 1; i < searchrange; i=i+1){
			if (read_imageui(processed, sampler, (int2){x, y-1}).x != 0){
				write_imageui(final_image, coord, 3.938*read_imageui(processed, sampler, (int2){x, y-1}).x);
				break;
			}	
			else if (read_imageui(processed, sampler, (int2){x-i, y}).x != 0){
				write_imageui(final_image, coord, 3.938*read_imageui(processed, sampler, (int2){x-i, y}).x);
				break;
			}	
			else if (read_imageui(processed, sampler, (int2){x+i, y}).x != 0){
				write_imageui(final_image, coord, 3.938*read_imageui(processed, sampler, (int2){x+i, y}).x);
				break;
			}	
			else if (read_imageui(processed, sampler, (int2){x-i, y+i}).x != 0){
				write_imageui(final_image, coord, 3.938*read_imageui(processed, sampler, (int2){x, y+i}).x);
				break;
			}	
		}
	}
}