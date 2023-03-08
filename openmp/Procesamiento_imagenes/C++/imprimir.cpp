#include <stdio.h>
#include <opencv.h>

using namespace cv;

int main(int argc, char** argv )
{
    Mat imagen;
    
    imagen = imread( "./Unison.jpg", IMREAD_COLOR );
    if ( !image.data )
    {
        printf("No image data \n");
        return -1;
    }
    
    printf(imagen)
    namedWindow("Display Image", WINDOW_AUTOSIZE );
    imshow("Display Image", image);
    waitKey(0);
    return 0;
}