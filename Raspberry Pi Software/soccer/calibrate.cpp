#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <time.h>
#include <queue>
#include <string>
#include <ctime>
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include "raspicam/raspicam_cv.h"
#include "area.hpp"

#define WIDTH 320
#define HEIGHT 240


int H_MIN = 0, H_MAX = 180;
int S_MIN = 0, S_MAX = 255;
int V_MIN = 0, V_MAX = 255;
int THOLD = 40;
int TCENTER = 40;


cv::Mat frame; // Original image
cv::Mat hsv; // HSV Image
cv::Mat filtered; // Color filtered image and mask
cv::Mat gray;
cv::Mat canny;


cv::VideoWriter vidOut;


bool loadCalibration();

int main(int argc, char** argv) {
	
	if( !loadCalibration() ) return 0;
	
	// Create sliders
	cv::namedWindow( "Slider" );
	cv::createTrackbar( "H_MIN", "Slider", &H_MIN, 255 );
	cv::createTrackbar( "H_MAX", "Slider", &H_MAX, 180 );
	cv::createTrackbar( "S_MIN", "Slider", &S_MIN, 255 );
	cv::createTrackbar( "S_MAX", "Slider", &S_MAX, 255 );
	cv::createTrackbar( "V_MIN", "Slider", &V_MIN, 255 );
	cv::createTrackbar( "V_MAX", "Slider", &V_MAX, 255 );
	cv::createTrackbar( "THOLD", "Slider", &THOLD, 300 );
	cv::createTrackbar( "TCENTER", "Slider", &TCENTER, 300 );
	
	// Camera object
	raspicam::RaspiCam_Cv cam;
	cam.set( CV_CAP_PROP_FRAME_HEIGHT, HEIGHT );
	cam.set( CV_CAP_PROP_FRAME_WIDTH, WIDTH );
	cam.set( CV_CAP_PROP_FORMAT, CV_8UC3 );
	cam.set( CV_CAP_PROP_FPS, 60 );
	cam.set( CV_CAP_PROP_WHITE_BALANCE_RED_V, -1 );
	cam.set( CV_CAP_PROP_WHITE_BALANCE_BLUE_U, -1 );
	
	printf( "[*] Open Camera\n" );
	if( !cam.open() ) {
		printf( "[*] Failed to open camera!\n" );
		return -1;
	}
	
	printf( "[*] Waiting for camera to stabilize\n" );
	usleep( 3 * 1000000);
	printf( "[*] Ready for capturing\n" );
	
	char vidName[50];
	time_t t = time(0);
	struct tm* now = localtime( &t );
	sprintf( vidName, "%d.%d.%d-%d_%d.avi", now->tm_mday, now->tm_mon+1, now->tm_year+1900, now->tm_hour, now->tm_min );
	
	//vidOut.open( vidName, CV_FOURCC('H','2','6','4') , 30, cv::Size(WIDTH, HEIGHT) );

	
	
	bool first = true;
	
	cv::Mat container( HEIGHT * 2, WIDTH * 2, CV_8UC3, cv::Scalar(0, 0, 0) ); 
	
	struct timespec t_start, t_end;
	
	// Grab and retrieve image from camera
	for( ;; ) {
		
		clock_gettime( CLOCK_REALTIME, &t_start );
		
		cam.set( CV_CAP_PROP_WHITE_BALANCE_RED_V, -1 );
		cam.set( CV_CAP_PROP_WHITE_BALANCE_BLUE_U, -1 );

		// Get current image from camera
		cam.grab();
		cam.retrieve( frame );

		// Mask image
		cv::Mat mask = frame( cv::Rect(0, 0, 320, 80) );
		mask.setTo( cv::Scalar(0, 0, 0) );
		
		cv::Point mid( frame.cols / 2, frame.rows );

		if( first ) {
			printf( "[*] Image size: %d %d\n", frame.cols, frame.rows );
			first = false;
			continue;
		}	
		
		//cv::GaussianBlur( frame, frame, cv::Size(13, 13), 2, 2 );

		// Convert image from BGR to HSV
		cv::cvtColor( frame, hsv, cv::COLOR_RGB2HSV );
		/* cv::cvtColor( frame, gray, cv::COLOR_BGR2GRAY ); */
		
		
		// Filter pixel for specific values in given range
		cv::inRange( hsv, cv::Scalar( H_MIN, S_MIN, V_MIN ), cv::Scalar( H_MAX, S_MAX, V_MAX ), filtered );
		/* cv::Mat maskFilter = filtered.clone(); 
		
		gray.copyTo( gray, maskFilter ); 
		
		cv::Canny( gray, canny, THOLD, THOLD*3, 3 ); */
		
		// Found areas
		std::vector<Area> areas;

		// Search every 4th pixel 
		for( int y = 0; y < filtered.rows; y+=4 ) {
		   	for( int x = 0; x < filtered.cols; x+=4 ) {

				// Read value
				uchar value = filtered.at<uchar>( y, x );

                // Pixel is not white
                if( value != 255 ) continue;

				// New area
                Area a( cv::Point( x, y ) );

				// Queue for searching pixel
				std::queue<cv::Point> queue;
				queue.push( cv::Point( x, y ) );

				// Flood fill using a queue
				bool bEmpty = false;
				while( !bEmpty ) {
					cv::Point p = queue.front();
					if( p.x > 0 && filtered.at<uchar>( p.y, p.x - 1 ) == 255 ) {
						a.addPixel( cv::Point( p.x - 1, p.y ) );
						filtered.at<uchar>( p.y, p.x - 1 ) = 254;
						queue.push( cv::Point( p.x - 1, p.y ) );
					}
					if( p.y > 0 && filtered.at<uchar>( p.y - 1, p.x ) == 255 ) {
						a.addPixel( cv::Point( p.x, p.y - 1 ) );
						filtered.at<uchar>( p.y - 1, p.x ) = 254;
						queue.push( cv::Point( p.x, p.y - 1 ) );
					}
					if( p.x < filtered.cols - 1 && filtered.at<uchar>( p.y, p.x + 1 ) == 255 ) {
						a.addPixel( cv::Point( p.x + 1, p.y ) );
						filtered.at<uchar>( p.y, p.x + 1) = 254;
						queue.push( cv::Point( p.x + 1, p.y ) );
					}
					if( p.y < filtered.rows - 1 && filtered.at<uchar>( p.y + 1, p.x ) == 255 ) {
						a.addPixel( cv::Point( p.x, p.y + 1 ) );
						filtered.at<uchar>( p.y + 1, p.x ) = 254;
						queue.push( cv::Point( p.x, p.y + 1) );
					}

					queue.pop();
					bEmpty = queue.empty();
				}

				// Area found
				areas.push_back( a );
            }
		}
		
		
		/* std::vector<cv::Vec3f> circles;
		cv::HoughCircles( filtered, circles, CV_HOUGH_GRADIENT, 1, filtered.rows / 5, THOLD, TCENTER, 0, 0 );
		for( size_t i = 0; i < circles.size(); i++ ) {
			cv::Point center( cvRound(circles[i][0]), cvRound(circles[i][1]) );
			int radius = cvRound(circles[i][2]);
			
			cv::circle( frame, center, 3, cv::Scalar(0, 255, 0), -1, 8, 0 );
			cv::circle( frame, center, radius, cv::Scalar(0, 0, 255), 3, 8, 0 );
		} */


		Area bigArea( cv::Point(0,0) );

		// Search biggest area
		for( std::vector<Area>::iterator it = areas.begin(); it != areas.end(); it++ ) {
			Area a = (Area) *it;

			if( a.getPixelCount() > bigArea.getPixelCount() ) bigArea = a;
		}

		
		
		float prop1 = (float) bigArea.getWidth() / (float) bigArea.getHeight();
		float prop2 = (float) bigArea.getHeight() / (float) bigArea.getWidth();
		
		// Draw biggest area
		if( bigArea.getPixelCount() > 20 && prop1 < 2 && prop2 < 2 ) {
			cv::Point obj = bigArea.getStart();
			obj.x += bigArea.getWidth() / 2;
			obj.y += bigArea.getHeight() / 2;
				
			float diffX = obj.x - mid.x;
			float diffY = mid.y - obj.y;
			
			bigArea.draw( &frame, cv::Scalar( 0, 0, 255 ), cv::Scalar( 0, 255, 0 ) );			
			cv::line( frame, obj, mid, cv::Scalar( 255, 100, 100 ) );

			float horizontal = 0, vertical = 0;
			horizontal = ((float) (obj.x - mid.x) / ((float) WIDTH / 2.0f)) * 31.0f + 31.0f;
			vertical = ((float) (mid.y - obj.y) / (float) HEIGHT) * 50.0f;



			std::string text;
			std::stringstream out;
			out << horizontal << " " << vertical;
			text = out.str();
			
			
			cv::putText( frame, text, cv::Point(mid.x, 50), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 255) );

		} else {
			cv::putText( frame, "Ball not found", cv::Point(mid.x, 50), CV_FONT_HERSHEY_PLAIN, 1, cv::Scalar(0,0,255) );
		}
		
		cv::cvtColor( filtered, filtered, cv::COLOR_GRAY2BGR );
		//cv::cvtColor( gray, gray, cv::COLOR_GRAY2BGR );
		//cv::cvtColor( canny, canny, cv::COLOR_GRAY2BGR );
		
		
		frame.copyTo( container(cv::Rect(0, 0, WIDTH, HEIGHT)) );
		filtered.copyTo( container(cv::Rect(WIDTH, 0, WIDTH, HEIGHT)) );
		/* gray.copyTo( container(cv::Rect(0, HEIGHT, WIDTH, HEIGHT)) ); 
		canny.copyTo( container(cv::Rect(WIDTH, HEIGHT, WIDTH, HEIGHT)) ); */
		cv::imshow( "Images", container );
		
		if( cv::waitKey( 2 ) == 27 ) break;
		
		clock_gettime( CLOCK_REALTIME, &t_end );
		
		unsigned long ss = t_start.tv_sec * 1000;
		unsigned long es = t_end.tv_sec * 1000;
		
		ss += t_start.tv_nsec / 1.0e6;
		es += t_end.tv_nsec / 1.0e6;
		
		unsigned long d = es - ss;
		
		//vidOut << frame;
		
		printf( "Frames: %d\n", 1000 / d );
	}
	
	
	// Write calibration data out
	std::ofstream fileOut("/home/pi/soccer/calibrate.txt");
	fileOut << H_MIN << "\n";
	fileOut << H_MAX << "\n";
	fileOut << S_MIN << "\n";
	fileOut << S_MAX << "\n";
	fileOut << V_MIN << "\n";
	fileOut << V_MAX;
	fileOut.close();

	printf( "[*] Release\n" );
	cam.release();
	printf( "[*] Finished!\n" );

	return 0;
}


bool loadCalibration() {
	
	printf( "[*] Open calibration file\n" );
	std::ifstream fileIn("/home/pi/soccer/calibrate.txt");
	if( !fileIn.is_open() ) {
		printf( "[*] Error: Failed to open calibration file\n" );
		return false;
	}
		
	std::string line = "EMPTY";
	size_t val;
	for( int i = 0; i < 6; i++ ) {
		if( !std::getline( fileIn, line ) ) {
			printf( "[*] Error: Calibration file wrong format\n" );
			return false;
		} 
			
		val = std::stoi( line.c_str(), &val );
		switch( i ) {
			case 0:
				H_MIN = val;
				printf( "H_MIN: %d\n", H_MIN );
				break;
			case 1:
				H_MAX = val;
				printf( "H_MAX: %d\n", H_MAX );
				break;
			case 2:
				S_MIN = val;
				printf( "S_MIN: %d\n", S_MIN );
				break;
			case 3:
				S_MAX = val;
				printf( "S_MAX: %d\n", S_MAX );
				break;
			case 4:
				V_MIN = val;
				printf( "V_MIN: %d\n", V_MIN );
				break;
			case 5:
				V_MAX = val;
				printf( "V_MAX: %d\n", V_MAX );
				break;
		}
	}
	
	fileIn.close();
	
	return true;
}