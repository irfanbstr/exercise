//preprocessor
#include <iostream>
#include <stdio.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <time.h>

//namespace
using namespace cv;
using namespace std;

int main( int argc, char** argv )
{
	VideoCapture cap(0) ;
	int t=getTickCount()/getTickFrequency();//at the starting of loop
	while(1)
	{
		Mat frame;
		cap >> frame ;
		//sleep(1);
		int s = getTickCount()/getTickFrequency()-t;
		for(s=0;s<10;s+=s)
		{//in milliseconds
		//here getTickCount() is constantly updated, but t is fixed till you reset it
			cout<<s<<endl;
			t=getTickCount()/getTickFrequency();//reset the difference back to 0
		}
		if(waitKey(30)>=0) 
		break ;
	}
}
