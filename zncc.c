#include "lodepng.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
__int64 freq, start, end, diff;
//greyscale
unsigned int greysize=735*504;
unsigned int greywidth=735;
unsigned int greyhight=504; 
unsigned int ndisp=260/4;

int window=9;
int threshold=16;
const char* filename_final = "final2.png";

char resize_greyscale(unsigned char* image,unsigned char* greyscale_image,int width,int height){
unsigned char* resized_image=(char*)malloc(1481760);
int i,j,index;
 index=0;
for ( i=0;i<height*width*4;i=i+width*4*4 ) {

  for ( j=0;  j<width*4;j=j+4 ){
      resized_image[index]=image[i+j];
      
      index=index+1;
     
  }      
}

free(image);
index=0; 
for ( i=0;i<height/4*width/4*4;i=i+4){
    greyscale_image[index]=0.2126*resized_image[i]+0.7152*resized_image[i+1]+0.0722*resized_image[i+2];
        index=index+1;
}

free(resized_image);
return 0;
}

char zncc_left(unsigned char* greyscale_image1,unsigned char* greyscale_image2, unsigned char* disp_image){
unsigned int i,j,d,y,x,best_d;
float mean1,mean2;
float std1,std2;
float z, zncc, max_sum;

//käy läpi jokaisen rivin
for (j=0;j<=greysize-greywidth*((window+1)/2);j=j+greywidth){
    //käy läpi jokaisen sarakkeen
   
    for (i=j;i<j+greywidth;i=i+1){
      
        for(d=0;d<=ndisp;d=d+1){
//                printf("\nMean loop");
                for(y=i;y<=i+greywidth*window;y=y+greywidth){
                        for(x=y;x<=y+window;x=x+1){
                            //get mean
                            //img1
                            mean1=mean1+greyscale_image1[x];      
                            mean2=mean2+greyscale_image2[x-d];
                            
                        }
                    
                }
                //img1
                mean1=mean1/window;
                mean2=mean2/window;

  //              printf("\nzncc loop");
                for(y=i;y<i+greywidth*window;y=y+greywidth){
                        for(x=y;x<=y+window;x=x+1){
                            //get values to calculate ZNCC
                            //img1
                            std1=std1+(greyscale_image1[x]-mean1)*(greyscale_image1[x]-mean1);
                            std2=std2+(greyscale_image2[x-d]-mean2)*(greyscale_image2[x-d]-mean2);
                        
                            z=z+((greyscale_image1[x]-mean1)*(greyscale_image2[x-d]-mean2));

                        }
                        
                }
                //img1
                std1=sqrt(std1/window);
                std2=sqrt(std2/window);
                zncc=z/(std1*std2);

                
                
                if(zncc>max_sum){
                //img1
                max_sum=zncc;
                best_d=d;
                }
                
                
//Resets the values for the next d cycle
                //img1
                mean1=0;
                mean2=0;
                std1=0;
                std2=0;
                zncc=0;
                z=0;
  
        }
       // printf("\ni: %d",i);
       
       
       disp_image[i+((window-1)/2)*greywidth+((window-1)/2)]=best_d;       
       max_sum=0;
        best_d=0;

    }        
    
}


return 0;
}
char zncc_right(unsigned char* greyscale_image1,unsigned char* greyscale_image2,unsigned char* disp_image){
int i,j,d,y,x;
float mean1,mean2;
float std1,std2;
float z, zncc, max_sum;
int best_d;
//käy läpi jokaisen rivin
for (j=0;j<=greysize-greywidth*((window+1)/2);j=j+greywidth){
    //käy läpi jokaisen sarakkeen
   
    for (i=j;i<j+greywidth;i=i+1){
      
        for(d=0;d<=ndisp;d=d+1){
//                printf("\nMean loop");
                for(y=i;y<=i+greywidth*window;y=y+greywidth){
                        for(x=y;x<=y+window;x=x+1){
                            //get mean

                            //img2
                            mean1=mean1+greyscale_image2[x];      
                            mean2=mean2+greyscale_image1[x+d];
                            
                        }
                    
                }

                //img2
                mean1=mean1/window;
                mean2=mean2/window;
  //              printf("\nzncc loop");
                for(y=i;y<i+greywidth*window;y=y+greywidth){
                        for(x=y;x<=y+window;x=x+1){
                            //get values to calculate ZNCC
                      //img2
                            std1=std1+(greyscale_image2[x]-mean1)*(greyscale_image2[x]-mean1);
                            std2=std2+(greyscale_image1[x+d]-mean2)*(greyscale_image1[x+d]-mean2);
                        
                            z=z+((greyscale_image2[x]-mean1)*(greyscale_image1[x+d]-mean2));                            
                            
                        }
                        
                }

                //img2
                std1=sqrt(std1/window);
                std2=sqrt(std2/window);
                zncc=z/(std1*std2);
                
                

                
                if(zncc>max_sum){
                //img2
                max_sum=zncc;
                best_d=d;
                }
                
//Resets the values for the next d cycle
                
                mean1=0;
                mean2=0;
                std1=0;
                std2=0;
                zncc=0;
                z=0;
 
        }
        //printf("\nBest_d: %d Max_sum: %f", best_d, max_sum);
       

        
       //img2
       disp_image[i+((window-1)/2)*greywidth+((window-1)/2)]=best_d;       
       max_sum=0;
        best_d=0;
       
}    
    
}
return 0;
}


char post_process(unsigned char* final_image,unsigned char* disp_image1,unsigned char* disp_image2){
int i,j;
unsigned char* reference=final_image;
for(i=0;i<=greysize;i=i+1){
    if(abs(disp_image1[i]-disp_image2[i-disp_image1[i]])>threshold){
    final_image[i]=0;        
    //final_image[i]=final_image[i-1];
    }
    else{
        final_image[i]=disp_image1[i];
    }

}

//occlusion filling

for(i=0;i<=greysize;i=i+1){
    if(final_image[i]==0){
    //final_image[i]=final_image[i-1];
    for(j=1;j<window;j=j+1){
        
        if(final_image[i]!=0){
            printf("\n%d",j);
            break;
        }
        else if(reference[i-j]!=0){
            final_image[i]=reference[i-j];
                        printf("\na");
        }
        else if(reference[i+j]!=0){
        final_image[i]=reference[i+j];
          printf("\nb");
        }
        else if(reference[i+j*greywidth]!=0){
        final_image[i]=reference[i+j*greywidth];
          printf("\nc");
        }
        else if(reference[i-j*greywidth]!=0){
        final_image[i]=reference[i-j*greywidth];
          printf("\nd");
        }

        else {
            final_image[i]=0;
        }
        
    }
    }
}

for ( i=0;i<greysize;i=i+1){
    //img1
    final_image[i]=3.938*final_image[i];
}

return 0;    
}
void main(){
// start
QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
QueryPerformanceCounter((LARGE_INTEGER*)&start);  
printf("\nZNCC:");
//avataan kuva 1
unsigned char* image1;
unsigned error;
unsigned width, height;

printf("\nAvataan kuva 1:");
const char* filename1="im0.png";
error = lodepng_decode32_file(&image1, &width, &height, filename1);
if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

printf("\nGreyscale ja skaalaus kuva 1:");
unsigned char* greyscale_image1=(char*)malloc(1481760/4);  
resize_greyscale(image1,greyscale_image1,width,height);


  //avataan kuva 2
  printf("\nAvataan kuva 2:");
   unsigned char* image2;
 const char* filename2="im1.png";
 error = lodepng_decode32_file(&image2, &width, &height, filename2);
 if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

 printf("\nSkaalaus ja greyscale kuva 2: ");
unsigned char* greyscale_image2=(char*)malloc(1481760/4); 
resize_greyscale(image2,greyscale_image2,width,height);  

printf("\nZNCC left: ");
//unsigned char* disp_image1=(char*)malloc(greysize);
unsigned char disp_image1[greysize];
zncc_left(greyscale_image1,greyscale_image2,disp_image1);

printf("\nZNCC right: ");
//unsigned char* disp_image2=(char*)malloc(greysize);
unsigned char disp_image2[greysize];
zncc_right(greyscale_image1,greyscale_image2,disp_image2);
free(greyscale_image1);
free(greyscale_image2);

printf("\nPost process:");
unsigned char* final_image=(char*)malloc(greysize);
post_process(final_image,disp_image1,disp_image2);
free(disp_image1);
free(disp_image2);
//const char* filename_final;
//filename_final = "finalw9t16.png";
unsigned error1 = lodepng_encode_file(filename_final, final_image, width/4, height/4,LCT_GREY,8);
 

 // end
QueryPerformanceCounter((LARGE_INTEGER*)&end);
diff = ((end - start) * 1000) / freq;

unsigned int milliseconds = (unsigned int)(diff & 0xffffffff);
printf("\nExecution time: %u ms\n", milliseconds);
}

