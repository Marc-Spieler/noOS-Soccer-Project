#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "camera.h"
#include "ball.h"
#include <queue>
#include <sstream>
#include <fstream>
#include <math.h>
#include <fcntl.h>
#include "com.h"

static cv::Mat filtered;
static cv::Mat flatted;

// Filter Values
static int H_MIN = 0, H_MAX = 255;
static int S_MIN = 0, S_MAX = 255;
static int V_MIN = 0, V_MAX = 255;

void *goalTask(void *arguments){
	std::ifstream fileIn;	
	int isBlue = *((int *)arguments);
	int frameGoalReadyLocal = 0;
	
	printf( "isBlue = %d\r\n", isBlue );

	// get values from calibrateGoal.txt
	printf( "[*Goal] Open calibration file\r\n" );
	if ( isBlue==0 )
	{

		printf( "[*] Open calibrationGoal_yellow file\n" );
		fileIn.open( "/home/pi/soccer/calibrateGoal_yellow.txt" );
		if( !fileIn.is_open() ) 
		{
		  printf( "[*] Error: Failed to open calibrationGoal_yellow file\n" );
		  exit( 0 );
		}
			
		}
		else
		{

		printf( "[*] Open calibrationGoal_blue file\n" );
		fileIn.open( "/home/pi/soccer/calibrateGoal_blue.txt" );
		if( !fileIn.is_open() ) 
		{
		  printf( "[*] Error: Failed to open calibrationGoal_blue file\n" );
		  exit( 0 );
		}
	}
		
	
		
	std::string line = "EMPTY";
	size_t val;
	for( int i = 0; i < 6; i++ )
	{
		if( !std::getline( fileIn, line ) )
		{
			printf( "[*Goal] Error: Calibration file wrong format\r\n" );
			exit( 0 );
		} 

		val = std::stoi( line.c_str(), &val );
		switch( i )
		{
			case 0:
				H_MIN = val;
				printf( "[*Goal] H_MIN: %d\r\n", H_MIN );
				break;
			case 1:
				H_MAX = val;
				printf( "[*Goal] H_MAX: %d\r\n", H_MAX );
				break;
			case 2:
				S_MIN = val;
				printf( "[*Goal] S_MIN: %d\r\n", S_MIN );
				break;
			case 3:
				S_MAX = val;
				printf( "[*Goal] S_MAX: %d\r\n", S_MAX );
				break;
			case 4:
				V_MIN = val;
				printf( "[*Goal] V_MIN: %d\r\n", V_MIN );
				break;
			case 5:
				V_MAX = val;
				printf( "[*Goal] V_MAX: %d\r\n", V_MAX );
				break;
		}
	}
	fileIn.close();
	
	while (1)
	{  
        pthread_mutex_lock(&ready_mutex);
        frameGoalReadyLocal = frameGoalReady;	
        pthread_mutex_unlock(&ready_mutex);	
        	   
		if (frameGoalReadyLocal==1)
		{
			// Filter pixel for specific values in given range
			cv::inRange( hsv, cv::Scalar( H_MIN, S_MIN, V_MIN ), cv::Scalar( H_MAX, S_MAX, V_MAX ), filtered );
			flatted = filtered.clone();
      
			// Found areas
			std::vector<Area> areas;
      
			// Search every 4th pixel 
			for( int y = 0; y < flatted.rows; y+=8 )
			{
				for( int x = 0; x < flatted.cols; x+=8 )
				{
					// Read value
					uchar value = flatted.at<uchar>( y, x );

					// Pixel is not white
					if( value != 255 ) continue;

					// New area
					Area a( cv::Point( x, y ) );

					// Queue for searching pixel
					std::queue<cv::Point> queue;
					queue.push( cv::Point( x, y ) );

					// Flood fill using a queue
					bool bEmpty = false;
					while( !bEmpty )
					{
						cv::Point p = queue.front();
						if( p.x > 0 && flatted.at<uchar>( p.y, p.x - 1 ) == 255 )
						{
							a.addPixel( cv::Point( p.x - 1, p.y ) );
							flatted.at<uchar>( p.y, p.x - 1 ) = 254;
							queue.push( cv::Point( p.x - 1, p.y ) );
						}
						if( p.y > 0 && flatted.at<uchar>( p.y - 1, p.x ) == 255 )
						{
							a.addPixel( cv::Point( p.x, p.y - 1 ) );
							flatted.at<uchar>( p.y - 1, p.x ) = 254;
							queue.push( cv::Point( p.x, p.y - 1 ) );
						}	
						if( p.x < flatted.cols - 1 && flatted.at<uchar>( p.y, p.x + 1 ) == 255 )
						{
							a.addPixel( cv::Point( p.x + 1, p.y ) );
							flatted.at<uchar>( p.y, p.x + 1) = 254;
							queue.push( cv::Point( p.x + 1, p.y ) );
						}
						if( p.y < flatted.rows - 1 && flatted.at<uchar>( p.y + 1, p.x ) == 255 )
						{
							a.addPixel( cv::Point( p.x, p.y + 1 ) );
							flatted.at<uchar>( p.y + 1, p.x ) = 254;
							queue.push( cv::Point( p.x, p.y + 1) );
						}

						queue.pop();
						bEmpty = queue.empty();
					}

					// Area found
					areas.push_back( a );
				}
			}
      
			Area bigArea( cv::Point(0,0) );

			// Search biggest area
			for( std::vector<Area>::iterator it = areas.begin(); it != areas.end(); it++ )
			{
				Area a = (Area) *it;

				if( a.getPixelCount() > bigArea.getPixelCount() ) bigArea = a;
			}
			
			// get area proportions
			float prop1 = (float) bigArea.getWidth() / (float) bigArea.getHeight();
			float prop2 = (float) bigArea.getHeight() / (float) bigArea.getWidth();
      
			// Goal found
			if( bigArea.getPixelCount() > 20) // check size of area
			//if( bigArea.getPixelCount() > 20 && prop1 < 5.0 && prop2 < 2.5 )
			//if( bigArea.getPixelCount() > 20 && prop1 > 1.0 && prop2 < 1.0 )
			{
				objGoal = bigArea.getStart();
				objGoal.x += bigArea.getWidth() / 2;
				objGoal.y += bigArea.getHeight() / 2;

				float horizontal = 0, vertical = 0, GoalHalfWidth = 0;
				horizontal = ((float)(objGoal.x - (WIDTH / 2)) / (float)(WIDTH / 2)) * 31.0f; // horizontal goal position on a scale from -31.0 to 31.0
				vertical = ((float) ((HEIGHT / 2) - objGoal.y) / (float) HEIGHT) * 50.0f;
				GoalHalfWidth = ((float)(bigArea.getWidth()/2) / (float) WIDTH ) * 62.0f;
				//printf("HalfWidth: %f  Horizontal: %f\r\n",GoalHalfWidth, horizontal);
				
				if ( horizontal < -31.0f )
				{
					horizontal = -31.0f;
				}

				if ( horizontal > 31.0f )
				{
					horizontal = 31.0f;
				}

				horizontal += 32.0f; // horizontal values are now 1.0 - 63.0
//				printf("Obj.x mid.x WIDTH :%d %d %d\r\n", obj.x, mid.x, WIDTH);
				infoGoal.ball1.horizontal = (unsigned short) horizontal;
				infoGoal.ball1.vertical = (unsigned short) vertical;
				infoGoal.ball1.GoalHalfWidth = (unsigned short) GoalHalfWidth;
				infoGoal.status.see = 1;
			}
			else
			{
				// Goal not found
				infoGoal.status.see = 0;
			}	
   
      

		    pthread_mutex_lock(&ready_mutex);	
			frameGoalReady = 0; //signal for camera thread to begin its task
			comGoalReady = 1; //signal for com thread to begin its task
			pthread_mutex_unlock(&ready_mutex);	
			
			


			//printf("GoalPos %d %d\r\n", infoGoal.goal1.horizontal, infoGoal.goal1.vertical);
		}
		else
		{
			usleep(1000);
		}
	   
	}
   
}

