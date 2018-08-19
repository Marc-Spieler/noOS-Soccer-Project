#include <stdio.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <queue>
#include <sstream>
#include <fstream>
#include <math.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <wiringPiSPI.h>
#include <wiringPi.h>
#include "packets.h"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include "raspicam/raspicam_cv.h"
#include "area.hpp"

#define PI 3.14159265
#define WIDTH 640
#define HEIGHT 480

typedef unsigned char uint8_t;

struct PacketBluetooth pBluetooth = { 0 };
struct PacketSPI_OUT pOut = { 0 };
struct PacketSPI_IN pIn = { 0 };
struct Info info;

// Images
cv::Mat frame, hsv, filtered, flatted;
//cv::VideoWriter vidOut;


// Filter Values
int H_MIN = 0, H_MAX = 255;
int S_MIN = 0, S_MAX = 255;
int V_MIN = 0, V_MAX = 255;

// Camera
raspicam::RaspiCam_Cv cam;

// modes
#define MASTER 1
#define SLAVE 0

// bluetooth devices
const char PI_ONE[18]  = "B8:27:EB:EA:F3:FD";
const char PI_TWO[18] = "B8:27:EB:0D:95:42";
const char PI_THREE[18] = "B8:27:EB:55:6B:64";


bdaddr_t temp = { 0 };
bdaddr_t* bdaddr_any = &temp;

int main(int argc, char** argv) {
	
	memset( &info, 0x0, sizeof(info) );
	
	// Check wich mode
	int mode = MASTER;
	if( argc > 2 && !strcmp( argv[1], "SLAVE" ) ) {
		mode = SLAVE;
	}
	
	// setup SPI
	wiringPiSPISetup( 0, 1000000 );
	unsigned char bytes[4];
	
	// 'Setup pin listener
	wiringPiSetup() ;
	pinMode( 1, INPUT );
	
	// - Communcation process
	// -- Camera process
	int pidCamera = -1;
	
	// Shared Memory 
	// [Status] [Angle]
	size_t sos = sizeof(short);
	
	unsigned char* shared = (unsigned char*) mmap( 
		NULL, 
		1 + sos + sos,
		PROT_READ | PROT_WRITE, 
		MAP_SHARED | MAP_ANONYMOUS, 
		-1, 
	0 );

	// setup Bluetooth
	int sock, client = 0;
	int bytesRead, totalRead;
	struct sockaddr_rc con = { 0 }, client_con = { 0 };
	char dest[18], clientName[18], buffer[32];
	socklen_t opt = sizeof( client_con );
	
	if( mode == MASTER ) {
		printf( "[*] Starting as Master\n" );
		
		// create server-socket
		sock = socket( AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM );
		
		// bind socket to port 1
		con.rc_family = AF_BLUETOOTH;
		con.rc_bdaddr = *bdaddr_any;
		con.rc_channel = (uint8_t) 1;
		bind( sock, (struct sockaddr*) &con, sizeof(con) );
		
		// listen to port 1
		listen( sock, 1 );
		fcntl( sock, F_SETFL, O_NONBLOCK );
	}
	
	if( mode == SLAVE ) {
		printf( "[*] Starting as Slave\n" );
		
		if( argc < 3 ) {
			printf( "[*] Error: Destination not given\n" );
			return 0;
		}
		
		if( !strcmp( argv[2], "PI_ONE" ) )
			memcpy( dest, PI_ONE, 18 );
		else if( !strcmp( argv[2], "PI_TWO" ) )
			memcpy( dest, PI_TWO, 18 );
		else if( !strcmp( argv[2], "PI_THREE" ) )
			memcpy( dest, PI_THREE, 18 );
		else {
			printf( "[*] Error: Unknown Destination\n" );
			return 0;
		}
		
		printf( "[*] Master is %s\n", dest );
		
		// Create socket
		sock = socket( AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM );
		
		// Set connection
		con.rc_family = AF_BLUETOOTH;
		con.rc_channel = (uint8_t) 1;
		str2ba( dest, &con.rc_bdaddr );
		
		// Connect
		printf( "[*] Connect to master\n" );
		int status = connect( sock, (struct sockaddr*) &con, sizeof(con) );
		if( status < 0 ) {
			printf( "[*] Error: Failed to connect. %d\n" );
			return 0;
		} else if( status == 0 ) {
			// Connected
			printf( "[*] Connected to master\n" );
		}
	}
	
	printf("[*] Bluetooth was set up\n" );
	
	
	// Create child process for camera
	pidCamera = fork();
	if( pidCamera == -1 ) {
		printf( "[*] Error: Failed to fork camera process\n" );
		return 0;
	}
	
	if( pidCamera == 0 ) {
		printf( "[*] Open calibration file\n" );
		std::ifstream file("/home/pi/soccer/calibrate.txt");
		if( !file.is_open() ) {
			printf( "[*] Error: Failed to open calibration file\n" );
			*shared = 0xFF;
			exit( 0 );
		}
		
		std::string line = "EMPTY";
		size_t val;
		for( int i = 0; i < 6; i++ ) {
			if( !std::getline( file, line ) ) {
				printf( "[*] Error: Calibration file wrong format\n" );
				*shared = 0xFF;
				exit( 0 );
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
		
		cam.set( CV_CAP_PROP_FRAME_HEIGHT, HEIGHT );
		cam.set( CV_CAP_PROP_FRAME_WIDTH,  WIDTH );
		cam.set( CV_CAP_PROP_FORMAT, CV_8UC3 );
		
		printf( "[*] Open camera\n" );
		if( !cam.open() ) {
			printf( "[*] Error: Could not open Camera\n" );
			*shared = 0xFF;
			exit( 0 );
		}
		
		printf( "[*] Wait for stabilisize\n" );
		usleep( 3 * 1000000 );
		printf( "[*] Ready for capturing\n" );
		
		time_t t = time( NULL );
		struct tm ti = *localtime(&t);
		
		cv::Size sv = cv::Size( WIDTH, HEIGHT );
		
		char vidName[26];
		sprintf( vidName, "%d_%d_%d-%d_%d.avi", ti.tm_mday, ti.tm_mon + 1, ti.tm_year + 1900, ti.tm_hour, ti.tm_min ); 
		printf( "[*] VideoName: %s\n", vidName );
		int fourcc = CV_FOURCC('H','2','6','4');
		
		//vidOut.open( vidName, fourcc, 30, sv );
		
		*shared = 0x1; 
	}
	
	// Wait for camera to be ready
	if( pidCamera > 0 ) {
		while( !(*shared) );
		if( *shared == 0xFF ) exit( 0 );
		printf( "[*] Child process ready\n" );
		*shared = 0; 
	}
	
	struct timeval t_now, t_ball, t_button;
	unsigned short lastBall;
	
	gettimeofday( &t_button, NULL );
	while( *shared != 0xFF ) {
		gettimeofday( &t_now, NULL );
		
		// Check for Bluetooth connection
		if( pidCamera > 0 && mode == MASTER ) {
			int cs = accept( sock, (struct sockaddr*) &client_con, &opt );
			if( cs > 0 ) {
					
				// Client has connected
				ba2str( &client_con.rc_bdaddr, clientName );
				printf( "[*] %s connected\n", clientName );
					
				// Check if client is authorized
				if( 0 && strcmp(clientName, PI_ONE) && strcmp(clientName, PI_TWO) && strcmp(clientName, PI_THREE) ) {
					close( cs );
				} else {
					client = cs;
				}
			}
		}
		
		// Camera process
		if( pidCamera == 0 ) {
			
			// Get picture
			cam.grab();
			cam.retrieve( frame );
			
			cv::Point mid( frame.cols / 2, frame.rows );
			
			// Convert image from BGR to HSV
			cv::cvtColor( frame, hsv, cv::COLOR_RGB2HSV );
			
			// Filter pixel for specific values in given range
			cv::inRange( hsv, cv::Scalar( H_MIN, S_MIN, V_MIN ), cv::Scalar( H_MAX, S_MAX, V_MAX ), filtered );
			flatted = filtered.clone();
			
			// Found areas
			std::vector<Area> areas;
			
			// Search every 4th pixel 
			for( int y = 0; y < flatted.rows; y+=8 ) {
				for( int x = 0; x < flatted.cols; x+=8 ) {
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
					while( !bEmpty ) {
						cv::Point p = queue.front();
						if( p.x > 0 && flatted.at<uchar>( p.y, p.x - 1 ) == 255 ) {
							a.addPixel( cv::Point( p.x - 1, p.y ) );
							flatted.at<uchar>( p.y, p.x - 1 ) = 0;
							queue.push( cv::Point( p.x - 1, p.y ) );
						}
						if( p.y > 0 && flatted.at<uchar>( p.y - 1, p.x ) == 255 ) {
							a.addPixel( cv::Point( p.x, p.y - 1 ) );
							flatted.at<uchar>( p.y - 1, p.x ) = 0;
							queue.push( cv::Point( p.x, p.y - 1 ) );
						}	
						if( p.x < flatted.cols - 1 && flatted.at<uchar>( p.y, p.x + 1 ) == 255 ) {
							a.addPixel( cv::Point( p.x + 1, p.y ) );
							flatted.at<uchar>( p.y, p.x + 1) = 0;
							queue.push( cv::Point( p.x + 1, p.y ) );
						}
						if( p.y < flatted.rows - 1 && flatted.at<uchar>( p.y + 1, p.x ) == 255 ) {
							a.addPixel( cv::Point( p.x, p.y + 1 ) );
							flatted.at<uchar>( p.y + 1, p.x ) = 0;
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
			for( std::vector<Area>::iterator it = areas.begin(); it != areas.end(); it++ ) {
				Area a = (Area) *it;
				//printf( "[*] Difference: %d\n", difference );

				//if( a.getWidth() * a.getHeight() > bigArea.getWidth() * bigArea.getHeight() && a.getPixelCount() > 20) bigArea = a;
				if( a.getPixelCount() > bigArea.getPixelCount() ) bigArea = a;
			}
			
			// Ball found
			if( bigArea.getPixelCount() > 20 ) {
				cv::Point obj = bigArea.getStart();
				obj.x += bigArea.getWidth() / 2;
				obj.y += bigArea.getHeight() / 2;
				
				float diffX = obj.x - mid.x;
				float diffY = mid.y - obj.y;
				
				cv::line( frame, obj, mid, cv::Scalar( 255, 100, 100 ) );
				
				float horizontal = 0, vertical = 0;
				horizontal = ((float) (obj.x - mid.x) / ((float) WIDTH / 2.0f)) * 31.0f + 31.0f;
				vertical = ((float) (mid.y - obj.y) / (float) HEIGHT) * 50.0f;
		
				unsigned short shorizontal = (unsigned short) horizontal;
				unsigned short svertical = (unsigned short) vertical;
				*((unsigned short*)(shared+1)) = (unsigned short) shorizontal;
				*((unsigned short*)(shared+1+sos)) = (unsigned short) svertical;
				
			} 	else {
				// Ball not found
				//printf( "Ball not found\n" );
				*((unsigned short*)(shared+1)) = 0xFFFF;
				*((unsigned short*)(shared+1+sos)) = 0xFFFF;
			}
			
			//vidOut.write( frame );
			 
				
			
			/* cv::imshow( "Frame", frame );
			cv::imshow( "Filter", filtered ); 
			cv::waitKey( 2 ); */
 			
			
		}
		
		// Main process
		if( pidCamera > 0) { 
		
			// Get value from camera
			unsigned short horizontal = *((unsigned short*)(shared+1));
			unsigned short vertical = *((unsigned short*)(shared+1+sos));
			
			if( horizontal == 0xFFFF ) {
				long long elapsed = (t_now.tv_sec * 1000000 + t_now.tv_usec) - (t_ball.tv_sec * 1000000 + t_ball.tv_usec);
				if( elapsed < 500000 ) {
					horizontal = lastBall;
				}
				
			} else {
				lastBall = horizontal;
				gettimeofday(&t_ball, NULL );	
			}
			
			
			info.ball1.horizontal = horizontal;
			info.ball1.vertical = vertical;
			
			
			pOut.roboPos.x = info.roboPos2.x;
			pOut.roboPos.y = info.roboPos2.y;
			pOut.ballPos.x = info.ballPos.x;
			pOut.ballPos.y = info.ballPos.y; 
			pOut.bits.ball = info.ball1.horizontal;
			pOut.bits.rsvd = 0b111;
			
			if( !info.go ) pOut.bits.ball = 0xFFF;
			
			memcpy( bytes, &pOut, sizeof(pOut) );
			
			uint8_t tmp = bytes[0];
			for( int i = 0; i < sizeof(pOut) - 1; i++ ) {
				bytes[i] = bytes[i+1];
			}
			bytes[sizeof(pOut) - 1] = tmp;
			
			wiringPiSPIDataRW( 0, bytes, sizeof(pOut) );
			memcpy( &pIn, bytes, sizeof(pIn) );
			
			info.roboPos1.x = pIn.roboPos.x;
			info.roboPos1.y = pIn.roboPos.y;
			info.status.s1 = pIn.bits.onField;
			info.wlanOld = info.wlanNew;
			info.wlanNew = pIn.bits.WLAN;
			info.status.valid1 = pIn.bits.validX && pIn.bits.validY;
			
			int compass = pIn.info.compass - 180;
			if( compass < 0 ) compass = 360 - compass;
			
			info.ball1.horizontal = compass + (info.ball1.horizontal - 32 );
			if( info.ball1.horizontal >= 360 ) info.ball1.horizontal = info.ball1.horizontal - 360;
			if( info.ball1.horizontal < 0 ) info.ball1.horizontal = info.ball1.horizontal + 360;
			
			// Exchange data with other robot
			if( mode == MASTER) {
				char tm[16];
				
				if( client > 0 ) {
				
					
					pBluetooth.roboPos.x = info.roboPos1.x;
					pBluetooth.roboPos.y = info.roboPos2.y;
					pBluetooth.bits.horizontal = info.ball1.horizontal;
					pBluetooth.bits.vertical = info.ball1.vertical;
					pBluetooth.bits.onField = info.status.s1;
					pBluetooth.bits.valid = info.status.valid1;
					pBluetooth.bits.have = info.status.have1;
					
					
					
					memcpy( buffer, &pBluetooth, sizeof(pBluetooth) );
					write( client,  buffer, sizeof(pBluetooth) );
					
					printf( "-------- SEND --------\n");
					printf( "%x %x\n", pBluetooth.roboPos.x, pBluetooth.roboPos.y );
					printf( "%x\n", pBluetooth.bits.horizontal );
					printf( "%x\n", pBluetooth.bits.vertical );
					printf( "%x\n", pBluetooth.bits.onField );
					printf( "-------- SEND --------\n");
					scanf( "%s", tm );
					
					totalRead = 0;
					do {
						printf("ACTION!");
						bytesRead = read( client, buffer+totalRead, 32 );
						totalRead += bytesRead;
					} while( totalRead < sizeof(pBluetooth) && bytesRead >= 0 );
					printf("\n");
					
					if( bytesRead < 0 ) {
						printf( "[*] Error: Read data from bluetooth failed\n" );
						info.status.s2 = 0;
					} else {
						printf( "[*] Read Bluetooth" );
						memcpy( &pBluetooth, buffer, sizeof(pBluetooth) );
						info.roboPos2.x = pBluetooth.roboPos.x;
						info.roboPos2.y = pBluetooth.roboPos.y;
						info.ball2.horizontal = pBluetooth.bits.horizontal;
						info.ball2.vertical = pBluetooth.bits.vertical;
						info.status.s2 = pBluetooth.bits.onField;
						info.status.valid2 = pBluetooth.bits.valid;
						info.status.have2 = pBluetooth.bits.have;
					
						printf( "-------- RECV --------\n");
						printf( "%x %x\n", pBluetooth.roboPos.x, pBluetooth.roboPos.y );
						printf( "%x\n", pBluetooth.bits.horizontal );
						printf( "%x\n", pBluetooth.bits.vertical );
						printf( "%x\n", pBluetooth.bits.onField );
						printf( "-------- RECV --------\n");
						scanf( "%s", tm );
					}
				}
				
			}
			
			if( mode == SLAVE ) {
				char ts[16];
				
				
				totalRead = 0;
				do {
					bytesRead = read( sock, buffer+totalRead, 32 );
					totalRead += bytesRead;
				} while( totalRead < sizeof(pBluetooth) && bytesRead >= 0 );
				
				if( bytesRead < 0 ) {
					printf( "[*] Error: Read data from bluetooth failed\n" );
					info.status.s2 = 0;
				} else {
					memcpy( &pBluetooth, buffer, sizeof(pBluetooth) );
					info.roboPos2.x = pBluetooth.roboPos.x;
					info.roboPos2.y = pBluetooth.roboPos.y;
					info.ball2.horizontal = pBluetooth.bits.horizontal;
					info.ball2.vertical = pBluetooth.bits.vertical;
					info.status.s2 = pBluetooth.bits.onField;
					info.status.valid2 = pBluetooth.bits.valid;
					info.status.have2 = pBluetooth.bits.have; 
					
					printf( "-------- RECEIVED--------\n");
					printf( "%x %x\n", pBluetooth.roboPos.x, pBluetooth.roboPos.y );
					printf( "%x\n", pBluetooth.bits.horizontal );
					printf( "%x\n", pBluetooth.bits.vertical );
					printf( "%x\n", pBluetooth.bits.onField );
					printf( "-------- RECEIVED--------\n");
					scanf( "%s", ts );
				}
				
				
				pBluetooth.roboPos.x = info.roboPos1.x;
				pBluetooth.roboPos.y = info.roboPos2.y;
				pBluetooth.bits.horizontal = info.ball1.horizontal;
				pBluetooth.bits.vertical = info.ball1.vertical;
				pBluetooth.bits.onField = info.status.s1;
				pBluetooth.bits.valid = info.status.valid1;
				pBluetooth.bits.have = info.status.have1;
				
				memcpy( buffer, &pBluetooth, sizeof(pBluetooth) );
				write( sock, buffer, sizeof(pBluetooth) );
				
				printf( "-------- SEND --------\n");
				printf( "%x %x\n", pBluetooth.roboPos.x, pBluetooth.roboPos.y );
				printf( "%x\n", pBluetooth.bits.horizontal );
				printf( "%x\n", pBluetooth.bits.vertical );
				printf( "%x\n", pBluetooth.bits.onField );
				printf( "-------- SEND --------\n");
				scanf( "%s", ts );
			}
			
			/* 
			printf( "Mode: %s\n", mode == MASTER ? "MASTER" : "SLAVE" );
			printf( "My Position (%d/%d)\n", info.roboPos1.x, info.roboPos1.y );
			printf( "Other Position: (%d/%d)\n", info.roboPos2.x, info.roboPos2.y );
			printf( "My Ball horizontal: %d\n", info.ball1.horizontal );
			printf( "My Ball vertical: %d\n", info.ball1.vertical );
			printf( "Other Ball horizontal: %d\n", info.ball2.horizontal );
			printf( "Other Ball vertical: %d\n", info.ball2.vertical );
			printf( "On Field: %d %d\n", info.status.s1, info.status.s2 );
			printf( "\n" );
			printf( "\n" );
			printf( "\n" );
			printf( "\n" );
			printf( "\n" );
			printf( "\n" );
			printf( "\n" );
			printf( "\n" );
			printf( "\n" );
			printf( "\n" );
			printf( "\n" );
			printf( "\n" );
			printf( "\n" );
			printf( "\n" );
			printf( "\n" );
			printf( "\n" );
			*/
			
			
			// Interpretation
			if( info.wlanOld != info.wlanNew ) {
				if( info.wlanNew ) system("sudo ifconfig wlan0 up");
				if( !info.wlanNew ) system("sudo ifconfig wlan0 down");
			}
			
			
			if( info.status.have2 ) info.go = false;
			if( info.ball2.vertical < info.ball1.vertical ) info.go = false;
			
			if( info.status.have1 ) info.go = true;
			if( !info.status.s2 ) info.go = true;
			if( info.ball1.vertical <= info.ball2.vertical ) info.go = true;
			
			
			
			// Shutdown pin
			if( !digitalRead( 1 ) ) {
				long long elapsed = (t_now.tv_sec * 1000000 + t_now.tv_usec) - (t_button.tv_sec * 1000000 + t_button.tv_usec);
				if( elapsed > 2000000 ) {
					*shared = 0xFF;
				}
				
			} else {
				gettimeofday( &t_button, NULL );
			}			
			
			
					
			usleep( 1000 * 10 );
		}
		
	}

	int state;
	if( pidCamera == 0 ) {
		//vidOut.release();
		printf( "[*] Camera process ended\n" );
	}
	
	if( pidCamera > 0 ) {
		waitpid( pidCamera, &state, 0 );
		printf( "[*] DONE\n" );
	}
	exit( 0 );
}
