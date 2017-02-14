#include "lodepng.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
void main(){
	//Image 1 muuttujat
  unsigned error1;
  unsigned char* image1;
  unsigned width1, height1;
  const char* filename1;
  unsigned char* resized_image1=(char*)malloc(1481760);
    filename1="im0.png";
//yhteiset 
 unsigned int index;  
  unsigned int i, j;
  unsigned int threshold=16;
//image 2 muuttujat
  
  unsigned error2;
  unsigned char* image2;
  unsigned char* resized_image2=(char*)malloc(1481760);
  unsigned width2, height2;
  const char* filename2;
  filename2="im1.png";
  
 
 
//greyscale
unsigned int greysize=735*504;
unsigned int greywidth=735;
unsigned int greyhight=504; 
 
 
  printf("ALKU");


  
  error1 = lodepng_decode32_file(&image1, &width1, &height1, filename1);
  if(error1) printf("error %u: %s\n", error1, lodepng_error_text(error1));
  
  error2 = lodepng_decode32_file(&image2, &width2, &height2, filename2);
  if(error2) printf("error %u: %s\n", error2, lodepng_error_text(error2));
  
  printf("DECODE");
  //printf("leveys:%d pituus:%d strlen:%d w*h*4:%d %s",width1,height1, strlen(image1),width1*height1*4, filename1);
   // printf("\nw:%d h:%d strlen:%d %d %s",width2,height2, strlen(image2),width2*height1*4, filename2);
  /*use image here*/
//printf("\nimage0 ekat bitit: %c %c %c %c %c\nimage1 ekat bitit: %c %c %c %c %c",image1[23708161],image1[1],image1[2],image1[3],image1[4],image2[0],image2[1],image2[2],image2[3],image2[4]);
index=0;

for ( i=0;i<height1*width1*4;i=i+width1*4*4 ) {

  for ( j=0;  j<width1*4;j=j+4 ){
     //printf(" i+j=%d",i+j);
    /* if(i+j>height1*width1*4){
         printf(" nyt ");
         break;
     }*/
      resized_image1[index]=image1[i+j];
       
      resized_image2[index]=image2[i+j];
      index=index+1;
     
  }      
}
  free(image1);
 free(image2);
// printf("strlen: %d",strlen(resized_image));


   //const char* filename_resized1;
   //filename_resized1 = "resized1.png";
   //unsigned errorr1 = lodepng_encode32_file(filename_resized1, resized_image1, width1/4, height1/4);  
//mustavalkoisuus
  unsigned char greyscale_image1[1481760/4];
  unsigned char greyscale_image2[1481760/4];
  index=0;
for ( i=0;i<height1/4*width1/4*4;i=i+4){
    greyscale_image1[index]=0.2126*resized_image1[i]+0.7152*resized_image1[i+1]+0.0722*resized_image1[i+2];
        greyscale_image2[index]=0.2126*resized_image2[i]+0.7152*resized_image2[i+1]+0.0722*resized_image2[i+2];
    index=index+1;
}
const char* filename_greyscale1;
filename_greyscale1 = "greyscale1.png";
unsigned errorr1 = lodepng_encode_file(filename_greyscale1, greyscale_image1, width1/4, height1/4,LCT_GREY,8);

   /*
    const char* filename_resized2;
    filename_resized2 = "resized2.png";
    unsigned errorr2 = lodepng_encode32_file(filename_resized2, resized_image2, width1/4, height1/4);  
  */

free(resized_image1);
free(resized_image2);
//ZNCC variables
unsigned int ndisp=260/4;
unsigned int d;
unsigned int x;
unsigned int y;
float window=9;
int window1=9;
//img1
float mean1=0;
float mean2=0;
float std1=0;
float std2=0;
float z=0;
float zncc=0;
unsigned int best_d;
unsigned char disp_image1[greysize];
float max_sum;
//img2
float mean3=0;
float mean4=0;
float std3=0;
float std4=0;
float z2=0;
float zncc2=0;
unsigned int best_d2;
unsigned char disp_image2[greysize];
float max_sum2;


printf("ZNCC loopin alku");
//käy läpi jokaisen rivin
for (j=0;j<=greysize-greywidth;j=j+greywidth){
    //käy läpi jokaisen sarakkeen
   
    for (i=j;i<j+greywidth;i=i+1){
      
        for(d=0;d<=ndisp;d=d+1){
//                printf("\nMean loop");
                for(y=i;y<=i+greywidth*window1;y=y+greywidth){
                        for(x=y;x<=y+window1;x=x+1){
                            //get mean
                            //img1
                            mean1=mean1+greyscale_image1[x];      
                            mean2=mean2+greyscale_image2[x-d];
                            //img2
                            mean3=mean3+greyscale_image2[x];      
                            mean4=mean4+greyscale_image1[x+d];
                            
                        }
                    
                }
                //img1
                mean1=mean1/window;
                mean2=mean2/window;
                //img2
                mean3=mean3/window;
                mean4=mean4/window;
  //              printf("\nzncc loop");
                for(y=i;y<i+greywidth*window1;y=y+greywidth){
                        for(x=y;x<=y+window1;x=x+1){
                            //get values to calculate ZNCC
                            //img1
                            std1=std1+(greyscale_image1[x]-mean1)*(greyscale_image1[x]-mean1);
                            std2=std2+(greyscale_image2[x-d]-mean2)*(greyscale_image2[x-d]-mean2);
                        
                            z=z+((greyscale_image1[x]-mean1)*(greyscale_image2[x-d]-mean2));
                            //img2
                            std3=std3+(greyscale_image2[x]-mean3)*(greyscale_image2[x]-mean3);
                            std4=std4+(greyscale_image1[x+d]-mean4)*(greyscale_image1[x+d]-mean4);
                        
                            z2=z2+((greyscale_image2[x]-mean3)*(greyscale_image1[x+d]-mean4));                            
                            
                        }
                        
                }
                //img1
                std1=sqrt(std1/window);
                std2=sqrt(std2/window);
                zncc=z/(std1*std2);
                //img2
                std3=sqrt(std3/window);
                std4=sqrt(std4/window);
                zncc2=z2/(std3*std4);
                
                
                if(zncc>max_sum){
                //img1
                max_sum=zncc;
                best_d=d;
                }
                
                if(zncc2>max_sum2){
                //img2
                max_sum2=zncc2;
                best_d2=d;
                }
                
//Resets the values for the next d cycle
                //img1
                mean1=0;
                mean2=0;
                std1=0;
                std2=0;
                zncc=0;
                z=0;
                //img2
                mean3=0;
                mean4=0;
                std3=0;
                std4=0;
                zncc2=0;
                z2=0;
        }
        //printf("\nBest_d: %d Max_sum: %f", best_d, max_sum);
       
       //img1
       disp_image1[i+((window1-1)/2)*greywidth+((window1-1)/2)]=best_d;       
       max_sum=0;
        best_d=0;
        
       //img2
       disp_image2[i+((window1-1)/2)*greywidth+((window1-1)/2)]=best_d2;       
       max_sum2=0;
        best_d2=0;
       
}
//printf(" %d",j);

}
  
/*Testing stuff
  printf("\nwohoo!");
//disp image rescale from 0...65 to 0...255

for ( i=0;i<greysize;i=i+1){
    //img1
    disp_image1[i]=3.938*disp_image1[i];
    //img2
    disp_image2[i]=3.938*disp_image2[i]; 
}

    printf("\nwohoo!");
    
    //encode img1
const char* filename_disp1;
filename_disp1 = "disp1.png";
unsigned errorrr1 = lodepng_encode_file(filename_disp1, disp_image1, width1/4, height1/4,LCT_GREY,8);

    //encode img1
const char* filename_disp2;
filename_disp2 = "disp2.png";
unsigned errorrr2 = lodepng_encode_file(filename_disp2, disp_image2, width1/4, height1/4,LCT_GREY,8);
*/

//post processing
//cross correlation
unsigned char final_image[greysize];
for(i=0;i<=greysize;i=i+1){
    if(abs(disp_image1[i]-disp_image2[i])>threshold){
    final_image[i]=0;        
    }
    else{
        final_image[i]=disp_image1[i];
    }

}
//occlusion filling
for(i=0;i<=greysize;i=i+1){
    if(final_image[i]==0){
    final_image[i]=final_image[i-1];        
    }
}

for ( i=0;i<greysize;i=i+1){
    //img1
    final_image[i]=3.938*final_image[i];
}

const char* filename_final;
filename_final = "final.png";
unsigned errorrr2 = lodepng_encode_file(filename_final, final_image, width1/4, height1/4,LCT_GREY,8);
    
}