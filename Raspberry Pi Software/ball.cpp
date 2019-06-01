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

void *ballTask(void *arguments)
{		
	int index = *((int *)arguments);
	// get values from calibrateBall.txt
	printf( "[*Ball] Open calibration file\r\n" );
	std::ifstream file("/home/pi/soccer/calibrateBall.txt");
	if( !file.is_open() )
	{
		printf( "[*Ball] Error: Failed to open calibration file\r\n" );
		exit( 0 );
	}
		
	std::string line = "EMPTY";
	size_t val;
	for( int i = 0; i < 7; i++ )
	{
		if( !std::getline( file, line ) )
		{
			printf( "[*Ball] Error: Calibration file wrong format\r\n" );
			exit( 0 );
		} 

		val = std::stoi( line.c_str(), &val );
		switch( i )
		{
			case 0:
				H_MIN = val;
				printf( "[*Ball] H_MIN: %d\r\n", H_MIN );
				break;
			case 1:
				H_MAX = val;
				printf( "[*Ball] H_MAX: %d\r\n", H_MAX );
				break;
			case 2:
				S_MIN = val;
				printf( "[*Ball] S_MIN: %d\r\n", S_MIN );
				break;
			case 3:
				S_MAX = val;
				printf( "[*Ball] S_MAX: %d\r\n", S_MAX );
				break;
			case 4:
				V_MIN = val;
				printf( "[*Ball] V_MIN: %d\r\n", V_MIN );
				break;
			case 5:
				V_MAX = val;
				printf( "[*Ball] V_MAX: %d\r\n", V_MAX );
				break;
			case 6:
				TopBorder = val;
				printf( "[*Ball] TopBorder: %d\r\n", TopBorder );
				break;
		}
	}

	while (1)
	{  
		if (frameBallReady==1)
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
      
			// Ball found
			if( (bigArea.getPixelCount() > 20 && prop1 < 5.0 && prop2 < 2.5) ||
			   ( bigArea.getPixelCount() > 4 && infoBall.ball1.horizontal < (7) )||
			    (bigArea.getPixelCount() > 4 && infoBall.ball1.horizontal > (57) ))  // check proportions of the area //or out of "proportion border": bounding box around ball not possible 
			{
				objBall = bigArea.getStart();
				objBall.x += bigArea.getWidth() / 2;
				objBall.y += bigArea.getHeight() / 2;

				float horizontal = 0, vertical = 0;
				horizontal = ((float)(objBall.x - (WIDTH / 2)) / (float)(WIDTH / 2)) * 31.0f; // horizontal ball position on a scale from -31.0 to 31.0
				vertical = ((float) ((HEIGHT / 2) - objBall.y) / (float) HEIGHT) * 50.0f;

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
				infoBall.ball1.horizontal = (unsigned short) horizontal;
				infoBall.ball1.vertical = (unsigned short) vertical;
				infoBall.status.see = 1;
			}
			else
			{
				// Ball not found
				infoBall.status.see = 0;
			}	
   
			// Testpoints for having ball [Test][Left/Mid/Right][Lower/Upper]
			static cv::Point tml tml_par;
			static cv::Point tll1 tll1_par;
			static cv::Point tll2 tll2_par;
			static cv::Point trl1 trl1_par;
			static cv::Point trl2 trl2_par;

			static cv::Point tmu tmu_par;
			static cv::Point tlu1 tlu1_par;
			//static cv::Point tlu2 tlu2_par;
			static cv::Point tru1 tru1_par;
			//static cv::Point tru2 tru2_par;
      
			// Check lower row testpoints
			int countM = 0;

			int countL = 0;
			if( flatted.at<uchar>( tll1.y, tll1.x ) > 0 )
			{ 
			countL++;
				countM++;
				tll1_stat = 1;
			}
			else
			{
				tll1_stat = 0;
			}

			if( flatted.at<uchar>( tll2.y, tll2.x ) > 0 )
			{
				countL++;
				tll2_stat = 1;
			}
			else
			{
				tll2_stat = 0;
			}

			if( flatted.at<uchar>( tml.y, tml.x ) > 0 )
			{
				countL++; 
				countM++;
				tml_stat = 1;
			}
			else
			{
				tml_stat = 0;
			}

			if( flatted.at<uchar>( trl1.y, trl1.x ) > 0 )
			{
				countL++;
				countM++;
				trl1_stat = 1;
			}
			else
			{
				trl1_stat = 0;
			}

			if( flatted.at<uchar>( trl2.y, trl2.x ) > 0 )
			{
				countL++;
				trl2_stat = 1;
			}
			else
			{
				trl2_stat = 0;
			}


			// Check upper row testpoints
			int countU = 0;
			if( flatted.at<uchar>( tlu1.y, tlu1.x ) > 0 )
			{
				countU++;
				countM++;
				tlu1_stat = 1;
			}
			else
			{
				tlu1_stat = 0;
			}
			//if( flatted.at<uchar>( tlu2.y, tlu2.x ) > 0 )
			//{
				//countU++;
				//tlu2_stat = 1;
			//}
			//else
			//{
				//tlu2_stat = 0;
			//}
			if( flatted.at<uchar>( tmu.y, tml.x ) > 0 )
			{
				countU++;
				countM++;
				tmu_stat = 1;
			}
			else
			{
				tmu_stat = 0;
			}
			if( flatted.at<uchar>( tru1.y, tru1.x ) > 0 )
			{
				countU++;
				countM++;
				tru1_stat = 1;
			}
			else
			{
				tru1_stat = 0;
			}
			//if( flatted.at<uchar>( tru2.y, tru2.x ) > 0 )
			//{
				//countU++;
				//tru2_stat = 1;
			//}
			//else
			//{
				//tru2_stat = 0;
			//}
			//if( countU <= 2 && countL >= 3 && countL < 5 ) //originalvalues
			//if( countU <= 3 && countL >= 3 ) //working
			if( countU >= 2) //testLower
			{
				//cv::line( frame, cv::Point(0, 0), cv::Point(WIDTH, HEIGHT), cv::Scalar(0, 0, 255), 4 );
				infoBall.status.have2 = 1;
			}
			else
			{
				infoBall.status.have2 = 0;
			}
			
			if( countL >= 3) //testLower
			{
				//cv::line( frame, cv::Point(0, 0), cv::Point(WIDTH, HEIGHT), cv::Scalar(0, 0, 255), 4 );
				infoBall.status.have1 = 1;
			}
			//else if( countL >= 2 && countM > 3 && countM <= 5 ) //testvalues
			//else if( countL >= 1 && countM > 2 && countM <= 4 ) //working
			//{
				//cv::line( frame, cv::Point(0, 0), cv::Point(WIDTH, HEIGHT), cv::Scalar(0, 0, 255), 4 );
			//	infoBall.status.have1 = 1;
			//}
			else
			{
				infoBall.status.have1 = 0;
			}

			frameBallReady = 0; //signal for camera thread to begin its task
			comBallReady = 1; //signal for com thread to begin its task
			//printf("BallPos %d %d\r\n", infoBall.ball1.horizontal, infoBall.ball1.vertical);
		}
		else
		{
			usleep(1000);
		}
	}
}
