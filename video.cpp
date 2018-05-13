#include <iostream>
#include <stdio.h>
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <std_msgs/String.h>
#include <std_msgs/Int16.h>
#include "mavros_msgs/State.h"
#include <ros/ros.h>


void droneStatusCB						(const std_msgs::String& msg);


using namespace cv;
using namespace std;
Mat Original;

std_msgs::String drone_status;
std_msgs::String drone_feedback;

int main( int argc, char** argv )
{
	ros::init(argc, argv, "video");
	ros::NodeHandle nh;

	ros::Publisher pub_drone_status	= nh.advertise<std_msgs::String>("/auvsi17/drone/status", 16,true);
	ros::Subscriber sub_drone_status = nh.subscribe("/auvsi17/drone/status", 10, droneStatusCB);

	ROS_WARN_STREAM("Wait until loiter");
	while (ros::ok() && drone_status.data != "flying")
	{
		ros::spinOnce();
	} 
	
	ROS_INFO("Taking Picture");
	
	VideoCapture cap(0);
	if ( !cap.isOpened() ) 
    {
         cout << "webcam tidak terbaca" << endl;
         return -1;
    }
    
	cap.read(Original);
    namedWindow("Original", CV_WINDOW_AUTOSIZE);
    
    vector<int> compression_params; //vector that stores the compression parameters of the image
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY); //specify the compression technique
    compression_params.push_back(98); //specify the compression quality
    bool cSuccess = imwrite("../picture.jpg", Original, compression_params); //write the image to file
    
    Mat im_gray = imread("picture.jpg",CV_LOAD_IMAGE_GRAYSCALE);
	
	Mat img_bw = im_gray > 128;
	
	imwrite("../image_bw.jpg", img_bw);
	
	sleep(8);
    
	system ("tesseract image_bw.jpg out -l letsgodigital");
	imshow("Original", Original);

	ROS_INFO("Picture taken");
	
	drone_feedback.data = "to_the_land";
	pub_drone_status.publish(drone_feedback);
	ros::shutdown();
	
}

void droneStatusCB(const std_msgs::String& msg)
{
	drone_status.data = msg.data;
}
