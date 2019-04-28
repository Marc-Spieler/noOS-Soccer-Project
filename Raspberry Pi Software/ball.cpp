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

			float prop1 = (float) bigArea.getWidth() / (float) bigArea.getHeight();
			float prop2 = (float) bigArea.getHeight() / (float) bigArea.getWidth();
      
			// Ball found
			if( bigArea.getPixelCount() > 20 && prop1 < 5.0 && prop2 < 2.5 )
			{
				objBall = bigArea.getStart();
				objBall.x += bigArea.getWidth() / 2;
				objBall.y += bigArea.getHeight() / 2;

				float horizontal = 0, vertical = 0;
				horizontal = ((float)(objBall.x - (WIDTH / 2)) / (float)(WIDTH / 2)) * 31.0f;
				vertical = ((float) ((HEIGHT / 2) - objBall.y) / (float) HEIGHT) * 50.0f;

				if ( horizontal < -31.0f )
				{
					horizontal = -31.0f;
				}

				if ( horizontal > 31.0f )
				{
					horizontal = 31.0f;
				}

				horizontal += 32.0f;
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
			cv::Point tml( WIDTH / 2, HEIGHT - 53 );
			cv::Point tll1( WIDTH / 2 - WIDTH / 5, HEIGHT - 53 );
			cv::Point tll2( WIDTH / 2 - (2*WIDTH / 5), HEIGHT - 53 );
			cv::Point trl1( WIDTH / 2 + WIDTH / 5, HEIGHT - 53 );
			cv::Point trl2( WIDTH / 2 + (2*WIDTH / 5), HEIGHT - 53 );

			cv::Point tmu( WIDTH / 2, HEIGHT - 103 );
			cv::Point tlu1( WIDTH / 2 - WIDTH / 5, HEIGHT - 103 );
			cv::Point tlu2( WIDTH / 2 - (2*WIDTH / 5), HEIGHT - 103 );
			cv::Point tru1( WIDTH / 2 + WIDTH / 5, HEIGHT - 103 );
			cv::Point tru2( WIDTH / 2 + (2*WIDTH / 5), HEIGHT - 103 );
      
			// Check lower row testpoints
			int countM = 0;

			int countL = 0;
			if( flatted.at<uchar>( tll1.y, tll1.x ) > 0 )
			{ 
			countL++;
				countM++;
				//cv::line( frame, cv::Point(tll1.x - 2, tll1.y), cv::Point(tll1.x + 2, tll1.y), cv::Scalar(0, 255, 0), 2 );
				//cv::line( frame, cv::Point(tll1.x, tll1.y - 2), cv::Point(tll1.x, tll1.y + 2), cv::Scalar(0, 255, 0), 2 );
			}
			else
			{
				//cv::line( frame, cv::Point(tll1.x - 2, tll1.y), cv::Point(tll1.x + 2, tll1.y), cv::Scalar(0, 0, 255), 2 );
				//cv::line( frame, cv::Point(tll1.x, tll1.y - 2), cv::Point(tll1.x, tll1.y + 2), cv::Scalar(0, 0, 255), 2 );
			}

			if( flatted.at<uchar>( tll2.y, tll2.x ) > 0 )
			{
				countL++;
				//cv::line( frame, cv::Point(tll2.x - 2, tll2.y), cv::Point(tll2.x + 2, tll2.y), cv::Scalar(0, 255, 0), 2 );
				//cv::line( frame, cv::Point(tll2.x, tll2.y - 2), cv::Point(tll2.x, tll2.y + 2), cv::Scalar(0, 255, 0), 2 );
			}
			else
			{
				//cv::line( frame, cv::Point(tll2.x - 2, tll2.y), cv::Point(tll2.x + 2, tll2.y), cv::Scalar(0, 0, 255), 2 );
				//cv::line( frame, cv::Point(tll2.x, tll2.y - 2), cv::Point(tll2.x, tll2.y + 2), cv::Scalar(0, 0, 255), 2 );
			}

			if( flatted.at<uchar>( tml.y, tml.x ) > 0 )
			{
				countL++; 
				countM++;
				//cv::line( frame, cv::Point(tml.x - 2, tml.y), cv::Point(tml.x + 2, tml.y), cv::Scalar(0, 255, 0), 2 );
				//cv::line( frame, cv::Point(tml.x, tml.y - 2), cv::Point(tml.x, tml.y + 2), cv::Scalar(0, 255, 0), 2 );
			}
			else
			{
				//cv::line( frame, cv::Point(tml.x - 2, tml.y), cv::Point(tml.x + 2, tml.y), cv::Scalar(0, 0, 255), 2 );
				//cv::line( frame, cv::Point(tml.x, tml.y - 2), cv::Point(tml.x, tml.y + 2), cv::Scalar(0, 0, 255), 2 );
			}

			if( flatted.at<uchar>( trl1.y, trl1.x ) > 0 )
			{
				countL++;
				countM++;
				//cv::line( frame, cv::Point(trl1.x - 2, trl1.y), cv::Point(trl1.x + 2, trl1.y), cv::Scalar(0, 255, 0), 2 );
				//cv::line( frame, cv::Point(trl1.x, trl1.y - 2), cv::Point(trl1.x, trl1.y + 2), cv::Scalar(0, 255, 0), 2 );
			}
			else
			{
				//cv::line( frame, cv::Point(trl1.x - 2, trl1.y), cv::Point(trl1.x + 2, trl1.y), cv::Scalar(0, 0, 255), 2 );
				//cv::line( frame, cv::Point(trl1.x, trl1.y - 2), cv::Point(trl1.x, trl1.y + 2), cv::Scalar(0, 0, 255), 2 );
			}

			if( flatted.at<uchar>( trl2.y, trl2.x ) > 0 )
			{
				countL++;
				//cv::line( frame, cv::Point(trl2.x - 2, trl2.y), cv::Point(trl2.x + 2, trl2.y), cv::Scalar(0, 255, 0), 2 );
				//cv::line( frame, cv::Point(trl2.x, trl2.y - 2), cv::Point(trl2.x, trl2.y + 2), cv::Scalar(0, 255, 0), 2 );
			}
			else
			{
				//cv::line( frame, cv::Point(trl2.x - 2, trl2.y), cv::Point(trl2.x + 2, trl2.y), cv::Scalar(0, 0, 255), 2 );
				//cv::line( frame, cv::Point(trl2.x, trl2.y - 2), cv::Point(trl2.x, trl2.y + 2), cv::Scalar(0, 0, 255), 2 );
			}


			// Check upper row testpoints
			int countU = 0;
			if( flatted.at<uchar>( tlu1.y, tlu1.x ) > 0 )
			{
				countU++;
				countM++;
				//cv::line( frame, cv::Point(tlu1.x - 2, tlu1.y), cv::Point(tlu1.x + 2, tlu1.y), cv::Scalar(0, 255, 0), 2 );
				//cv::line( frame, cv::Point(tlu1.x, tlu1.y - 2), cv::Point(tlu1.x, tlu1.y + 2), cv::Scalar(0, 255, 0), 2 );
			}
			else
			{
				//cv::line( frame, cv::Point(tlu1.x - 2, tlu1.y), cv::Point(tlu1.x + 2, tlu1.y), cv::Scalar(0, 0, 255), 2 );
				//cv::line( frame, cv::Point(tlu1.x, tlu1.y - 2), cv::Point(tlu1.x, tlu1.y + 2), cv::Scalar(0, 0, 255), 2 );
			}
			if( flatted.at<uchar>( tlu2.y, tlu2.x ) > 0 )
			{
				countU++;
				//cv::line( frame, cv::Point(tlu2.x - 2, tlu2.y), cv::Point(tlu2.x + 2, tlu2.y), cv::Scalar(0, 255, 0), 2 );
				//cv::line( frame, cv::Point(tlu2.x, tlu2.y - 2), cv::Point(tlu2.x, tlu2.y + 2), cv::Scalar(0, 255, 0), 2 );
			}
			else
			{
				//cv::line( frame, cv::Point(tlu2.x - 2, tlu2.y), cv::Point(tlu2.x + 2, tlu2.y), cv::Scalar(0, 0, 255), 2 );
				//cv::line( frame, cv::Point(tlu2.x, tlu2.y - 2), cv::Point(tlu2.x, tlu2.y + 2), cv::Scalar(0, 0, 255), 2 );
			}
			if( flatted.at<uchar>( tmu.y, tml.x ) > 0 )
			{
				countU++;
				countM++;
				//cv::line( frame, cv::Point(tmu.x - 2, tmu.y), cv::Point(tmu.x + 2, tmu.y), cv::Scalar(0, 255, 0), 2 );
				//cv::line( frame, cv::Point(tmu.x, tmu.y - 2), cv::Point(tmu.x, tmu.y + 2), cv::Scalar(0, 255, 0), 2 );
			}
			else
			{
				//cv::line( frame, cv::Point(tmu.x - 2, tmu.y), cv::Point(tmu.x + 2, tmu.y), cv::Scalar(0, 0, 255), 2 );
				//cv::line( frame, cv::Point(tmu.x, tmu.y - 2), cv::Point(tmu.x, tmu.y + 2), cv::Scalar(0, 0, 255), 2 );
			}
			if( flatted.at<uchar>( tru1.y, tru1.x ) > 0 )
			{
				countU++;
				countM++;
				//cv::line( frame, cv::Point(tru1.x - 2, tru1.y), cv::Point(tru1.x + 2, tru1.y), cv::Scalar(0, 255, 0), 2 );
				//cv::line( frame, cv::Point(tru1.x, tru1.y - 2), cv::Point(tru1.x, tru1.y + 2), cv::Scalar(0, 255, 0), 2 );
			}
			else
			{
				//cv::line( frame, cv::Point(tru1.x - 2, tru1.y), cv::Point(tru1.x + 2, tru1.y), cv::Scalar(0, 0, 255), 2 );
				//cv::line( frame, cv::Point(tru1.x, tru1.y - 2), cv::Point(tru1.x, tru1.y + 2), cv::Scalar(0, 0, 255), 2 );
			}
			if( flatted.at<uchar>( tru2.y, tru2.x ) > 0 )
			{
				countU++;
				//cv::line( frame, cv::Point(tru2.x - 2, tru2.y), cv::Point(tru2.x + 2, tru2.y), cv::Scalar(0, 255, 0), 2 );
				//cv::line( frame, cv::Point(tru2.x, tru2.y - 2), cv::Point(tru2.x, tru2.y + 2), cv::Scalar(0, 255, 0), 2 );
			}
			else
			{
				//cv::line( frame, cv::Point(tru2.x - 2, tru2.y), cv::Point(tru2.x + 2, tru2.y), cv::Scalar(0, 0, 255), 2 );
				//cv::line( frame, cv::Point(tru2.x, tru2.y - 2), cv::Point(tru2.x, tru2.y + 2), cv::Scalar(0, 0, 255), 2 );
			}

			if( countU <= 2 && countL >= 3 && countL < 5 )
			{
				//cv::line( frame, cv::Point(0, 0), cv::Point(WIDTH, HEIGHT), cv::Scalar(0, 0, 255), 4 );
				infoBall.status.have1 = 1;
			}
			else if( countL >= 1 && countM > 2 && countM <= 4 )
			{
				//cv::line( frame, cv::Point(0, 0), cv::Point(WIDTH, HEIGHT), cv::Scalar(0, 0, 255), 4 );
				infoBall.status.have1 = 1;
			}
			else
			{
				infoBall.status.have1 = 0;
			}

			frameBallReady = 0;
			comBallReady = 1;
			//printf("BallPos %d %d\r\n", infoBall.ball1.horizontal, infoBall.ball1.vertical);
		}
		else
		{
			usleep(1000);
		}
	}
}
