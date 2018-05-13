#include <stdio.h>
#include <opencv2/opencv.hpp>

int tolV=100; // value tolerance (huge, due to lighting variations)

int tolH0=10; // hue tolerance for red color
int tolS0=100; // saturation tolerance for red color

int tolH1=21; // hue tolerance for green color
int tolS1=100; // saturation tolerance for green color

int p = 0; // p value; untuk pengaturan deviasi posisi titik bacaan (+)
int q = 0; // q value; untuk pengaturan deviasi posisi titik bacaan (-)

int min_pixels=10; // minimum detected size (box with this many pixels on a side)
int box_pixels=100; // ignore detected pixels farther than this from the initial center of mass

double lastPosition = 320;	// menyimpan posisi akhir titik bacaan; 320px titik tengah layar

/* Return the current wall clock time, in seconds */
double time_in_seconds(void) 
{
	return cv::getTickCount()/cv::getTickFrequency();//hitung per detik
}

int mouse_x,mouse_y;//int nilai koordinat
int mouse_flags=0;//int 0

void myMouseCallback(int event,int x,int y,int flags,void *param)
{
	if (event==CV_EVENT_MOUSEMOVE)//mendeteksi gerak 
	{
		mouse_x=x; mouse_y=y;//koordinat x dan y
	}
	mouse_flags=flags;//int 0
}

/*
 Search an image for a particular color, and return the coordinates
 of that color in the image.
*/

class color_matcher 
{
	public:
		cv::Mat threshed;  // thresholded image: black for no match; white for good match
	
		float posX,posY; // (x,y) pixel coordinates of matching pixels' center of mass
		float area; // pixels hit by center of mass
	
		// Look for this HSV color in this image.
		void match(const cv::Mat &imgHSV,const cv::Vec3b &target);
		public:
		/* Set our position from the center of mass of this image. */
		void calcMoments(const cv::Mat &img) 
		{
			cv::Moments mu=cv::moments(img);
			area = mu.m00; // sum of zero'th moment is area
			posX = mu.m10/area; // center of mass = w*x/weight
			posY = mu.m01/area;
			area /= 255; // scale from bytes to pixels
		}
};
	
void color_matcher::match(const cv::Mat &imgHSV,const cv::Vec3b &target)
{
	// Find matching pixels (color in range)
	// Order here is: Hue, Saturation, Value
	threshed=cv::Mat(imgHSV.size(),1);
	cv::inRange(imgHSV, 
		cv::Scalar(target[0]-tolH0, target[1]-tolS0, target[2]-tolV),
		cv::Scalar(target[0]+tolH1, target[1]+tolS1, target[2]+tolV),
		threshed);
	
	// Calculate the moments to estimate average position
	calcMoments(threshed);
	
	if (area>min_pixels*min_pixels) 
	{
		// Trim the image around the initial average's center of mass
		cv::Rect roi(-box_pixels,-box_pixels,10*box_pixels,10*box_pixels);
		roi+=cv::Point(posX,posY); // shift to initial center
		roi&=cv::Rect(cv::Point(0,0),threshed.size()); // trim to thresh's rectangle
		calcMoments(threshed(roi)); // recompute moments
		posX+=roi.x; posY+=roi.y; // shift back to pixels
	}
}

int main(int argc,char *argv[])
{
	double pointRead;
	double pointOutput;
	
	cv::VideoCapture *cap=0;
	long framecount=0;
	double time_start=-1.0;
	
	int argi=1; // command line argument index
	while (argc>=argi+2) 
	{ /* keyword-value pairs */
		char *key=argv[argi++];
		char *value=argv[argi++];
		if (0==strcmp(key,"--cam")) cap=new cv::VideoCapture(atoi(value));
		else if (0==strcmp(key,"--file")) cap=new cv::VideoCapture(value);
		else printf("Unrecognized command line argument '%s'!\n",key);
	}
	
	// Initialize capturing live feed from the camera
	if (!cap) cap=new cv::VideoCapture(0);

	// Couldn't get a device? Throw an error and quit
	if(!cap->isOpened())
	{
		printf("Could not initialize capturing...\n");
		return -1;
	}

	// Make the windows we'll be using
	cv::namedWindow("control panel");
	cv::setMouseCallback("control panel",myMouseCallback,NULL);
	cv::createTrackbar("Hue tol","control panel",&tolH0,180,NULL);
	cv::createTrackbar("Sat tol","control panel",&tolS0,255,NULL);
	cv::createTrackbar("Val tol","control panel",&tolV,255,NULL);
	cv::createTrackbar("Dev(+)","control panel",&p,512,NULL);
	cv::createTrackbar("Dev(-)","control panel",&q,512,NULL);
	cv::createTrackbar("Area min","control panel",&min_pixels,200,NULL);

	// This image holds the "scribble" data...
	// the tracked positions of the target
	cv::Mat scribble;
	
	// Will hold a frame captured from the camera
	cv::Mat frame, frameHSV;
		
	// Image color matching:
	enum {nMatchers=2};
	color_matcher matchers[nMatchers];
	
	cv::Vec3b targets[nMatchers];
	targets[0]=cv::Vec3b(170,200,200); // warna merah; -> ganti baris ini jika ingin mendeteksi warna lain
	targets[1]=cv::Vec3b(170,200,200); // warna merah; -> ganti baris ini jika ingin mendeteksi warna lain

	// An infinite loop
	while(true)
	{
		(*cap)>>frame; // grab next frame from camera

		// If we couldn't grab a frame... quit
		if(frame.empty())
			break;
		
		framecount++;
		if (time_start<0) time_start=time_in_seconds();
		
		if (scribble.empty())
			scribble = cv::Mat::zeros(frame.size(),CV_8UC3);

		// Convert to HSV (for more reliable color matching)
		cv::cvtColor(frame, frameHSV, CV_BGR2HSV);
		
		// Dump the value at the mouse location
		if (mouse_x>=0 && mouse_x<frameHSV.cols &&
		    mouse_y>=0 && mouse_y<frameHSV.rows &&
		    0!=(mouse_flags&CV_EVENT_FLAG_LBUTTON))
			{
				printf("HSV: ");
				cv::Vec3b pix=frameHSV.at<cv::Vec3b>(mouse_y,mouse_x);
				for (int k=0;k<3;k++) printf("%d ",(int)pix[k]);
				printf("\n");
				fflush(stdout); // <- for "tee" or "tail"
				targets[0]=pix;
			}
		
		// Do color matching
		for (int m=0;m<nMatchers;m++)
			matchers[m].match(frameHSV,targets[m]);
		
		pointRead = matchers[0].posX; // membaca posisi (koordinat X) objek berwarna merah
		pointOutput = pointRead + p - q; // Koordinat X objek merah setelah diolah dengan "pengaturan deviasi"
		
		// NaN Handler
		// Last position handler
		if (pointRead != pointRead)
		{
			pointOutput = lastPosition;
			printf("Error color detection. Set x point to last position: %.2f \n", lastPosition);
			
			// Define your last state handler here
			
		} 
		else 
		{
			printf("Current x point: %.2f \n", lastPosition);
			lastPosition = pointOutput;
			fflush(stdout);
		}
		

		if ( ((framecount)%2)==0 ) 
		{
			// Pack thresholded images into color channels
			std::vector<cv::Mat> channels;
			channels.push_back(matchers[1].threshed); // blue channel (same as green)
			channels.push_back(matchers[1].threshed); // green channel
			channels.push_back(matchers[0].threshed); // red channel 
			cv::Mat colorThresh;
			cv::merge(channels,colorThresh); // threshold images packed into colors
		
			// Mix with video
			frame=frame*0.5+scribble+colorThresh;
			//cv::imshow("video", frame);
			
			// Circle
			circle( frame, cv::Point( matchers[0].posX, matchers[0].posY ), 8, cv::Scalar( 0, 0, 255 ), 2);
			circle( frame, cv::Point( pointOutput, 240 ), 8, cv::Scalar( 0, 255, 255 ), 2);
			
			cv::imshow("Video Window", frame);
		
			// Slowly scale back old scribbles to zero
			scribble=scribble*0.94-1.1;
		
			// Show thresholded images separately
			//cv::imshow("thresh", matchers[1].threshed);
			cv::imshow("threshRed", matchers[0].threshed);

			// Wait for a keypress (for up to 1ms)
			int c = cv::waitKey(1);
			if(c!=-1)
			{
				// If any key is pressed, break out of the loop
				break;
			}
		}
	}

	return 0;
}
