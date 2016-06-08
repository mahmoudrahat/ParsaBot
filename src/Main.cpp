//#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/photo/photo.hpp"
#include <time.h>
#include <iostream>
#include<string.h>
#include "roomba.h"
#include <unistd.h>
#include "stdio.h"
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;

CvPoint mouse_start, mouse_end; 	// keep track of start and end points of selection area
bool mouseDrawingRect = false;		// is left mouse key down. Does region of interest selection continues?
bool whichMouseKeyPressed = true; 	// which Mouse Key Pressed down. false:right true:left. left key to add ROI. right key to remove ROI. ROI stands for region of interest.
bool doThresholding = false;		// ROI selection is completed. now apply thresholding
bool ColorMap[64][64][64];			// color map array to keep track of target coclors.
bool autoControl = false;			// true: put ParsaBot on auto control. False:turn auto control off
bool drawRectangle = true;			// Show target on the video.
int last_direction = 0;    			//0:right 1:left. to keep track of last direction where the target is seen

void FindBlobs(const cv::Mat &binary, std::vector < std::vector<cv::Point2i> > &blobs);  // Signature of Blob detection function

// callback function to read mouse events
void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
	if (event == EVENT_RBUTTONUP)
	{
		// this event happens when mouse right key is up
		mouseDrawingRect = false;
		mouse_end.x = x;
		mouse_end.y = y;
	}
	else if (event == EVENT_RBUTTONDOWN)
	{
		// this event happens when mouse right key is down
		mouse_start.x = x;
		mouse_start.y = y;
		mouse_end.x = x;
		mouse_end.y = y;
		mouseDrawingRect = true;
		whichMouseKeyPressed = false;
	}
	else if (event == EVENT_LBUTTONUP)
	{
		// event for left mouse key up
		mouseDrawingRect = false;
		mouse_end.x = x;
		mouse_end.y = y;
	}
	else if (event == EVENT_LBUTTONDOWN)
	{
		// event for left mouse key down
		mouse_start.x = x;
		mouse_start.y = y;
		mouse_end.x = x;
		mouse_end.y = y;
		mouseDrawingRect = true;
		whichMouseKeyPressed = true;   
	}
	else if (event == EVENT_MOUSEMOVE)
	{
		// capturing mouse movemnet event
		if (mouseDrawingRect)
		{
			mouse_end.x = x;
			mouse_end.y = y;
		}
	}
}

int main()
{
	roomba myRoomba; // this object is used to send control commands to ParsaBot
	myRoomba.open_port(); // open connection to roomba.
	printf("Test Roomba control\n");
	myRoomba.test_roomba(); // test if roomba reads our commands?
	usleep(100000);
	VideoCapture cap(0); // open the default camera
	if (!cap.isOpened())  // check if we succeeded
	{
		printf("No camera detected!\n");
		return -1;
	}

	bool live_video = true;   // true: live video  false: still image

	Mat Originalframe, DisplayFrame;

	//Create a window
	namedWindow("Parsa", 1);
	//set the callback function for any mouse event
	setMouseCallback("Parsa", CallBackFunc, NULL);

	for (;;)
	{
		if (live_video){
			Mat temp;
			cap >> temp; // get a new frame from camera
			pyrDown(temp, Originalframe);
		}
		DisplayFrame = Originalframe.clone(); // copy frame

		if (mouseDrawingRect)
		{
			// we are in the middle of drawing a region of interest.
			rectangle(DisplayFrame, mouse_start, mouse_end, Scalar(0, 0, 255));
		}
		else if (mouse_start.x != 0 || mouse_start.y != 0 || mouse_end.x != 0 || mouse_end.y != 0)
		{
			// mouse click released.
			// the region of interest has been determined
			if (mouse_start.x < 0)
				mouse_start.x = 0;
			if (mouse_start.x >= Originalframe.cols)
				mouse_start.x = Originalframe.cols - 1;
			if (mouse_start.y < 0)
				mouse_start.y = 0;
			if (mouse_start.y >= Originalframe.rows)
				mouse_start.y = Originalframe.rows - 1;
			
			if (mouse_end.x < 0)
				mouse_end.x = 0;
			if (mouse_end.x >= Originalframe.cols)
				mouse_end.x = Originalframe.cols - 1;
			if (mouse_end.y < 0)
				mouse_end.y = 0;
			if (mouse_end.y >= Originalframe.rows)
				mouse_end.y = Originalframe.rows - 1;


			Rect myROI(min(mouse_start.x, mouse_end.x), min(mouse_start.y, mouse_end.y), abs(mouse_end.x - mouse_start.x), abs(mouse_end.y - mouse_start.y));
			Mat cropped = Originalframe(myROI);
			uchar r, g, b;
			for (int i = 0; i < cropped.rows; ++i)
			{
				cv::Vec3b* pixel = cropped.ptr<cv::Vec3b>(i); // point to first pixel in row
				for (int j = 0; j < cropped.cols; ++j)
				{
					r = pixel[j][2];
					g = pixel[j][1];
					b = pixel[j][0];
					ColorMap[r/4][g/4][b/4] = whichMouseKeyPressed;
				}
			}
			mouse_start = cvPoint(0, 0);
			mouse_end = cvPoint(0, 0);
			doThresholding = true;
		}

		if (doThresholding)
		{
			// apply thresholding on live video based on selected color map
			
			// ranged image set green value to 255 for all pixels that fall inside color map. otherwise leaves the pixel intact.
			Mat ranged(DisplayFrame.rows, DisplayFrame.cols, CV_8U, Scalar(0));

			//Mat ranged = DisplayFrame.clone();
			uchar r, g, b;
			for (int i = 0; i < DisplayFrame.rows; ++i)
			{
				cv::Vec3b* pixel = DisplayFrame.ptr<cv::Vec3b>(i); // point to first pixel in row
				uchar* pixel2 = ranged.ptr<uchar>(i); // point to first pixel in row
				for (int j = 0; j < DisplayFrame.cols; ++j)
				{
					r = pixel[j][2]; // red value
					g = pixel[j][1]; // green value
					b = pixel[j][0]; // blue value
					if (ColorMap[r/4][g/4][b/4])
					{
						pixel[j][1] = 255;	// set green value to 255 to Distinguish target areas					
						pixel2[j] = 255;
					}
				}

			}

			std::vector < std::vector<cv::Point2i > > blobs;
			// this function finds all blobs inside image
			FindBlobs(ranged, blobs);			

			// find maximum blob.
			int max = 0;
			int index = -1;
			for (size_t i = 0; i < blobs.size(); i++) {
				if (blobs[i].size() > max)
				{
					max = blobs[i].size();
					index = i;
				}
			}

			if(autoControl)
			{
				// here is the control logic of roomba
				
				// check if we have a maximum blob and its size if big enough. 
				// for small targets reduce threshold value. now is 1200 pixels
				if (index != -1 && blobs[index].size() > 1200)
				{
					// find bounding box of target blob
					int minx = 1000, miny = 1000, maxx = -1, maxy = -1;
					for (size_t j = 0; j < blobs[index].size(); j++)
					{
						int x = blobs[index][j].x;
						int y = blobs[index][j].y;
						if (x < minx)
							minx = x;
						if (x > maxx)
							maxx = x;
						if (y < miny)
							miny = y;
						if (y > maxy)
							maxy = y;
					}

					// draw 4 lines to display target blob
					if (drawRectangle){
						rectangle(DisplayFrame, cvPoint(-10, miny), cvPoint(700, maxy), cvScalar(30, 80, 180), 5);
						rectangle(DisplayFrame, cvPoint(minx, -10), cvPoint(maxx, 500), cvScalar(30, 80, 180), 5);
					}

					// find center of the target
					int centerx = (minx + maxx) / 2;
					int imageSize = Originalframe.cols;
					double ballsize = (maxx - minx) * (maxy - miny); 
					printf("ball size:%f ", ballsize); // print target blob size
					if (/*blobs[index].size() > 28000*/ballsize > 40000){
						// if ball is to big. go back
						printf("Back %d\n", blobs[index].size());
						myRoomba.send_roomba(myRoomba.backward);						
					}
					else if (/*blobs[index].size() > 28000*/ballsize > 28000)
					{
						// if ball size is between 40000 and 28000 stop the robot.
						printf("Stop %f\n", ballsize);
						myRoomba.send_roomba(myRoomba.stop);
					}else if (centerx < imageSize / 3)
					{
						// if ball center is on The first third of the picture. send left command.
						printf("Left\n");
						myRoomba.send_roomba(myRoomba.left);
						last_direction = 1;
						//usleep(10000);	
					}else if (centerx > (2 * imageSize) / 3)
					{
						// if ball center is on The last third of the picture. send left command.
						printf("Right\n");
						myRoomba.send_roomba(myRoomba.right);
						last_direction = 0;
						//usleep(10000);	
					}else
					{
						// other wise go forward.
						printf("forward\n");
						myRoomba.send_roomba(myRoomba.forward);
						//usleep(10000);	
					}
					//printf("size:%d x:%d y:%d \n", blobs[index].size(), minx, miny);
				}
				else
				{
					// if no ball detected. turn around to find the ball
					printf("search for ball \n");
					if(last_direction == 0)
						myRoomba.send_roomba(myRoomba.right);
					else
						myRoomba.send_roomba(myRoomba.left);
					//usleep(10000);	
				}

			}
			//cv::imshow("ranged", ranged);
		}

		imshow("Parsa", DisplayFrame);

		
		// reading keyboard keys.
		int key = waitKey(30);
		if (key == 32)
		{
			// space key will switch live video to still image and vice versa, it also sends a stop command to roomba
			live_video = !live_video;
			myRoomba.send_roomba(myRoomba.stop);
		}
		else if (key == 'a')  
		{
			// toggle auto control 
			autoControl = !autoControl;
			if(!autoControl)
				myRoomba.send_roomba(myRoomba.stop);
			live_video = true;
		}
		else if (key == 'd')
		{
			// toggle drawing rectangle
			drawRectangle = !drawRectangle;
		}
		else if (key == 'b')
		{
			// send beep command
			printf("Roomba beep \n");
			myRoomba.send_roomba(myRoomba.beep);
		}
		else if (key == 's')
		{
			// save color map to file
			printf("Save color map \n");
			ofstream myfile ("ColorMap.txt");
			if(myfile.is_open())
			{
				for (int f = 0; f < 64;f++)
					for (int g = 0; g < 64;g++)
						for (int h = 0; h < 64; h++)
							myfile << ColorMap[f][g][h] << "\n";
				myfile.close();
			}
				else cout << "Unable to open file\n";
		}
		else if (key == 'l')
		{
			// load color map from file
			printf("Load color map \n");
			string line;
			ifstream myfile("ColorMap.txt");
			if (myfile.is_open())
			{
				for (int f = 0; f < 64; f++)
				for (int g = 0; g < 64; g++)
				for (int h = 0; h < 64; h++)				
				{
					getline(myfile, line);
					ColorMap[f][g][h] = line == "1" ? true : false;
				}
				myfile.close();
				doThresholding = true;
			}
			else cout << "Unable to open file\n";
		}
		else if (key == 'm')
		{
			// send safe mode command to robot
			printf("Roomba safe mode \n");
			myRoomba.send_roomba(myRoomba.pause);
			usleep(50000);	
			myRoomba.send_roomba(myRoomba.safe);
			usleep(50000);	
		}
		else if (key == 'r')
		{  
			//reset colorMap		
			for (int i = 0; i < 64;i++)
			for (int j = 0; j < 64;j++)
			for (int k = 0; k < 64; k++)
			{
				ColorMap[i][j][k] = false;
			}
		}
		else if (key == 27)
		{
			//ESCAPE exit
			break;
		}
		else if (key == 65362)
		{
			//up arrow key
			myRoomba.send_roomba(myRoomba.forward);
			//usleep(100000);	
		}
		else if (key == 65364)
		{
			//down arrow key
			myRoomba.send_roomba(myRoomba.backward);
			//usleep(100000);	
		}
		else if (key == 65361)
		{
			//left arrow key
			myRoomba.send_roomba(myRoomba.left);
			//usleep(100000);	
		}
		else if (key == 65363)
		{
			//right arrow key
			myRoomba.send_roomba(myRoomba.right);
			//usleep(100000);	
		}
	}
	return 0;
}

// this function finds all blob inside an binary image by using floodfill.
void FindBlobs(const cv::Mat &binary, std::vector < std::vector<cv::Point2i> > &blobs)
{
	// this fuction uses floodfill command to compute all blobs.
	blobs.clear();

	// Fill the label_image with the blobs
	// 0  - background
	// 1  - unlabelled foreground
	// 2+ - labelled foreground

	cv::Mat label_image;
	binary.convertTo(label_image, CV_32SC1);

	int label_count = 2; // starts at 2 because 0,1 are used already

	for (int y = 0; y < label_image.rows; y++) {
		int *row = (int*)label_image.ptr(y);
		for (int x = 0; x < label_image.cols; x++) {
			if (row[x] != 255) {
				continue;
			}

			cv::Rect rect;
			// floodfill the entire blob
			cv::floodFill(label_image, cv::Point(x, y), label_count, &rect, 0, 0, 4);

			// push back blob to return afterwards
			std::vector <cv::Point2i> blob;
			for (int i = rect.y; i < (rect.y + rect.height); i++) {
				int *row2 = (int*)label_image.ptr(i);
				for (int j = rect.x; j < (rect.x + rect.width); j++) {
					if (row2[j] != label_count) {
						continue;
					}

					blob.push_back(cv::Point2i(j, i));
				}
			}

			blobs.push_back(blob);

			label_count++;
		}
	}
}