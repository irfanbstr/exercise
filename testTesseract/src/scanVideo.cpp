#include <iostream>
#include <stdio.h>
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;
Mat Original;
int main( int argc, char** argv )
{
	VideoCapture cap(0);
	if ( !cap.isOpened() ) 
    {
         cout << "webcam tidak terbaca" << endl;
         return -1;
    }
	cap.read(Original);
    namedWindow("Original", CV_WINDOW_AUTOSIZE);
	while (true)
    {
		Mat frame;
		cap >> frame ;
		bool bSuccess = cap.read(Original); 
		if (!bSuccess) 
        {
             cout << "tidak bisa membaca frame dari video" << endl;
             break;
        }
        vector<int> compression_params; //vector that stores the compression parameters of the image
		compression_params.push_back(CV_IMWRITE_JPEG_QUALITY); //specify the compression technique
		compression_params.push_back(98); //specify the compression quality
		cvtColor(Original, Original, CV_BGR2GRAY);
		bool cSuccess = imwrite("test.jpg", Original, compression_params); //write the image to file
		system ("tesseract test.jpg test -l eng");
		imshow("Original", Original);
        if (waitKey(30) == 27) //exit esc
        {
            cout << "esc di tekan" << endl;
            break; 
        }
	}
	return 0;
}
