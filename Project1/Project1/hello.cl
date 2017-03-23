__kernel void hello(__global uchar4 *inputImageBuffer,
					__global uchar4 *outputImageBuffer)
{

int i,j,index,width,height;
 index=0;
 width=0;
 height=0;
for ( i=0;i<height*width*4;i=i+width*4*4 ) {

  for ( j=0;  j<width*4;j=j+4 ){
      outputImageBuffer[index]=inputImageBuffer[i+j];
      
      index=index+1;
     
  }      
}


//index=0; 
//for ( i=0;i<height/4*width/4*4;i=i+4){
//    greyscale_image[index]=0.2126*resized_image[i]+0.7152*resized_image[i+1]+0.0722*resized_image[i+2];
//        index=index+1;
//}


}