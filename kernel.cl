
__constant sampler_t sampler =
  CLK_NORMALIZED_COORDS_FALSE
| CLK_ADDRESS_CLAMP_TO_EDGE
| CLK_FILTER_NEAREST;





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

//Kernel for zncc on left image
__kernel void zncc_left(__read_only image2d_t greyscale1,
								__read_only image2d_t greyscale2,
								__write_only image2d_t dispLeft)
{
int x = get_global_id(0);
int y = get_global_id(1);
int window=9;
int ndisp = 65;
unsigned int i,j,d,best_d=0;
float mean1=0,mean2=0;
float std1=0,std2=0;
float z=0, zncc=0, max_sum=0;

int2 coord2 = {x+4, y+4};

float4 temp;
int temp2;
float temp3;
for(d=0;d<=ndisp;d=d+1){
	//mean loop
	
	for(i=y;i<=y+window;i=i+1){
		for(j=x;j<=x+window;j=j+1){
                            //get mean

                            //img2
											
			
            mean1=mean1+read_imageui(greyscale1,sampler, (int2) { i, j }).x;
            mean2=mean2+read_imageui(greyscale2,sampler,(int2){i-d,j}).x;
                            
                        }
						}
		mean1=mean1/window;
		mean2=mean2/window;
	for(i=y;i<=y+window;i=i+1){
		for(j=x;j<=x+window;j=j+1){
                            std1=std1+(read_imageui(greyscale1,sampler,(int2){i,j}).x-mean1)*(read_imageui(greyscale1,sampler, (int2) {i,j}).x-mean1);
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

				
				//printf(" %f \n", std1);
                
				
                if(zncc>max_sum){
                //img1
					printf("if %f ", max_sum);
                max_sum=zncc;
                best_d=d;
                }

	}
       write_imageui(dispLeft,coord2,best_d);  
}
