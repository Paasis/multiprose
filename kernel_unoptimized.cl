//
//Ville Kemppainen & Mikko Paasimaa
//Working implementation without optimisation
//Bad commentry
//




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

 //printf("%s\n", "this is a test string\n");
//printf("coord:%d coord:%d  x:%d y:%d z:%d grey:%d\n",x,y, pixel.x, pixel.y, pixel.z , grey);
//printf("%d ",y);
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
int window=9;
int ndisp = 65;
unsigned int i,j,d,best_d=0;
float mean1=0,mean2=0;
float std1=0,std2=0;
float z=0, zncc=0, max_sum=0;
int2 coord2 = { x,y };
//int2 coord2 = {x+4, y+4};


for(d=0;d<=ndisp;d=d+1){
	//mean loop
	
	for(j=y;j<=y+window;j=j+1){
		for(i=x;i<=x+window;i=i+1){
                            //get mean

                            //img2
											
			
            mean1=mean1+(float)read_imageui(greyscale1,sampler, (int2) { i, j }).x;
            mean2=mean2+(float)read_imageui(greyscale2,sampler,(int2){i-d,j}).x;
			
		//	printf(" %f \n", mean1);

                        }
						}
	
		mean1=mean1/(window);
		mean2=mean2/(window);
	for(j=y;j<=y+window;j=j+1){
		for(i=x;i<=x+window;i=i+1){
                            std1=std1+(read_imageui(greyscale1,sampler,(int2){i,j}).x-mean1)*(read_imageui(greyscale1,sampler, (int2) {i,i}).x-mean1);
                            std2=std2+(read_imageui(greyscale2,sampler, (int2) {i-d,j}).x-mean2)*(read_imageui(greyscale2,sampler, (int2) {i-d,j}).x-mean2);
                        
                            z=z+((read_imageui(greyscale1,sampler, (int2) {i,j}).x-mean1)*(read_imageui(greyscale2,sampler, (int2) {i-d,j}).x-mean2));
                            
                        }
						}
	
	//img1
                std1=native_sqrt(std1/window);
                std2=native_sqrt(std2/window);
                zncc=z/(std1*std2);

				//temp3 = (read_imageui(greyscale1, sampler, (int2){ i, j }).x);
				//temp3 = temp.x;

				
				
                
				
                if(zncc>max_sum){
                //img1
					//printf("if %d\n ", best_d);
                max_sum=zncc;
                best_d=d;
                }
				//Resets the values for the next d cycle
				//img1
				mean1 = 0;
				mean2 = 0;
				std1 = 0;
				std2 = 0;
				zncc = 0;
				z = 0;
	}
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
int window=9;
int ndisp = 65;
unsigned int i,j,d,best_d=0;
float mean1=0,mean2=0;
float std1=0,std2=0;
float z=0, zncc=0, max_sum=0;
int2 coord2 = { x,y };
//int2 coord2 = {x+4, y+4};


for(d=0;d<=ndisp;d=d+1){
	//mean loop
	
	for(j=y;j<=y+window;j=j+1){
		for(i=x;i<=x+window;i=i+1){
                            //get mean

                            //img2
											
			
            mean1=mean1+(float)read_imageui(greyscale1,sampler, (int2) { i, j }).x;
            mean2=mean2+(float)read_imageui(greyscale2,sampler,(int2){i+d,j}).x;
			
		//	printf(" %f \n", mean1);

                        }
						}
	
		mean1=mean1/window;
		mean2=mean2/window;
	for(j=y;j<=y+window;j=j+1){
		for(i=x;i<=x+window;i=i+1){
                            std1=std1+(read_imageui(greyscale1,sampler,(int2){i,j}).x-mean1)*(read_imageui(greyscale1,sampler, (int2) {i,i}).x-mean1);
                            std2=std2+(read_imageui(greyscale2,sampler, (int2) {i+d,j}).x-mean2)*(read_imageui(greyscale2,sampler, (int2) {i+d,j}).x-mean2);
                        
                            z=z+((read_imageui(greyscale1,sampler, (int2) {i,j}).x-mean1)*(read_imageui(greyscale2,sampler, (int2) {i+d,j}).x-mean2));
                            
                        }
						}
	
                std1=native_sqrt(std1/window);
                std2=native_sqrt(std2/window);
                zncc=z/(std1*std2);

				
				
                
				
                if(zncc>max_sum){
					//printf("if %d\n ", best_d);
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


	uint threshold=16;
	int2 coord = {x, y};

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
	int i, searchrange=100;

	if(read_imageui(processed, sampler, coord).x != 0){
		write_imageui(final_image, coord, 3.938*read_imageui(processed, sampler, coord).x);
	}
	else
	{
		for(i = 1; i < searchrange; i=i+1){
			if (read_imageui(processed, sampler, (int2){x-i, y}).x != 0){
				write_imageui(final_image, coord, 3.938*read_imageui(processed, sampler, (int2){x-i, y}).x);
				break;
			}	
			else if (read_imageui(processed, sampler, (int2){x+i, y}).x != 0){
				write_imageui(final_image, coord, 3.938*read_imageui(processed, sampler, (int2){x+i, y}).x);
				break;
			}	
			else if (read_imageui(processed, sampler, (int2){x, y-1}).x != 0){
				write_imageui(final_image, coord, 3.938*read_imageui(processed, sampler, (int2){x, y-1}).x);
				break;
			}	
			else if (read_imageui(processed, sampler, (int2){x-i, y+i}).x != 0){
				write_imageui(final_image, coord, 3.938*read_imageui(processed, sampler, (int2){x, y+i}).x);
				break;
			}	
		}

	}
}