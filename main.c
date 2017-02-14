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
printf("ENNEN LOOPPIA ON PASKAA");
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

printf("LOOPIN JÄLKEEN ON POUTA SÄÄ");
printf(" \n%f \n%d \n%c \n%s",resized_image1[8786],resized_image1[8786],resized_image1[8786],resized_image1[8786]);


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
float mean1=0;
float mean2=0;
float std1=0;
float std2=0;
float z=0;
float zncc=0;
unsigned int best_d;
unsigned char disp_image1[greysize];
float max_sum;

printf("ZNCC loopin alku");
//käy läpi jokaisen rivin
for (j=0;j<=greysize-greywidth;j=j+greywidth){
    //käy läpi jokaisen sarakkeen
   
    for (i=j;i<j+greywidth;i=i+1){
      
        for(d=0;d<ndisp;d=d+1){
//                printf("\nMean loop");
                for(y=j;y<j+greywidth*window;y=y+greywidth){
                        for(x=i;x<=window;x=x+1){
                            //get mean
                            mean1=mean1+greyscale_image1[x];      
                            mean2=mean2+greyscale_image2[x+d];
                        }
                    
                }
                mean1=mean1/window;
                mean2=mean2/window;
  //              printf("\nzncc loop");
                for(y=j;y<j+greywidth*window;y=y+greywidth){
                        for(x=i;x<=window;x=x+1){
                            //get values to calculate ZNCC
                            std1=std1+(greyscale_image1[x]-mean1)*(greyscale_image1[x]-mean1);
                            std2=std2+(greyscale_image2[x]-mean2)*(greyscale_image2[x]-mean2);
                        
                            z=z+((greyscale_image1[x]-mean1)*(greyscale_image2[x+d]-mean2));
                        }
                }
                std1=sqrt(std1/window*window);
                std2=sqrt(std2/window*window);
                zncc=z/(std1*std2);
                if(zncc>max_sum){
                max_sum=zncc;
                best_d=d;
                    }
        
                mean1=0;
                mean2=0;
        }

        disp_image1[i]=best_d;
        best_d=0;
}
printf(" %d",j);

}
    printf("\nwohoo!");
//disp image rescale from 0...65 to 0...255
for ( i=0;i<greysize;i=i+1){
    disp_image1[i]=3.938*disp_image1[i];
        //greyscale_image2[index]=0.2126*resized_image2[i]+0.7152*resized_image2[i+1]+0.0722*resized_image2[i+2];
    
}    
    printf("\nwohoo!");
    
    //encode
const char* filename_disp1;
filename_disp1 = "disp1.png";
unsigned errorrr1 = lodepng_encode_file(filename_disp1, disp_image1, width1/4, height1/4,LCT_GREY,8);

    
}