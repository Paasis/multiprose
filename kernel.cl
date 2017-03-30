
__constant sampler_t sampler =
  CLK_NORMALIZED_COORDS_FALSE
| CLK_ADDRESS_CLAMP_TO_EDGE
| CLK_FILTER_NEAREST;

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
//printf("%d\n", pixel);
write_imageui(output ,coord2 , grey);
}




//Kernel for zncc on left image
__kernel void zncc_left(__read_only image2d_t greyscale1,
								__read_only image2d_t greyscale2,
								__write_only image2d_t dispLeft)
{
int i = get_global_id(0);
int j = get_global_id(1);

int ndisp = 65;
unsigned int i,j,d,best_d;
float mean1,mean2;
float std1,std2;
float z, zncc, max_sum;


int2 coord = {x,y};
int2 coord2 = {x+4, y+4};

for(d=0;d<=ndisp;d=d+1){
	//mean loop
	for(y=i;y<=i+greywidth*window;y=y+greywidth){

}