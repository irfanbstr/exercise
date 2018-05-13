//preprocessor
#include <iostream>
#include <stdio.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

//namespace
using namespace cv;
using namespace std;

//matrix


int main( int argc, char** argv )
{
	Mat Original;
	VideoCapture cap(0);//webcam
	if ( !cap.isOpened() )  // jika tidak terbaca, exit
    {
         cout << "webcam tidak terbaca" << endl;
         return -1;
    }
	cap.read(Original);//capture di imgOriginal
    namedWindow("Original", CV_WINDOW_AUTOSIZE);//nama window
	while (true)//loop sampai exit
    {
		bool bSuccess = cap.read(Original); // membaca frame baru dari video
		if (!bSuccess) //jika tidak sukses, menghentikan loop
        {
             cout << "tidak bisa membaca frame dari video" << endl;
             break;
        }
		imshow("Original", Original); 
        
        if (waitKey(30) == 27) //exit esc
        {
            cout << "esc di tekan" << endl;
            break; 
        }
	}
	return 0;
}
