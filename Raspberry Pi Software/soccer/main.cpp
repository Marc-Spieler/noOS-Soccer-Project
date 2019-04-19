#include <stdio.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <queue>
#include <thread>
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
#include <curses.h>
#include "packets.h"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include "raspicam/raspicam_cv.h"
#include "area.hpp"

#define PI 3.14159265
#define WIDTH 320
#define HEIGHT 240

typedef unsigned char uint8_t;

struct PacketBluetooth pBluetooth = { 0 };
struct PacketSPI_OUT pOut = { 0 };
struct PacketSPI_IN pIn = { 0 };
struct Info info;

// Images
cv::Mat frame, hsv, filtered, flatted;
cv::VideoWriter vidOut;

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
const char PI_THREE[18] = "B8:27:EB:70:96:37";


bdaddr_t temp = { 0 };
bdaddr_t* bdaddr_any = &temp;

void winquit() {
	endwin();
}	

char line8[100];
char line9[100];
int liveCounter = 0;

bool stateConnected = false;
bool connectionRunning = false;
bool connectionFinished = false;

int sock, client = 0;
int bytesRead, totalRead;
struct sockaddr_rc con = { 0 }, client_con = { 0 };
char dest[18], clientName[18], buffer[32];
socklen_t opt = sizeof( client_con );
int readError = 0;
int connectDelay = 100, sendDelay = 0;

void fConnect() {
	sprintf( line8, "[*] Connect 2" );
	liveCounter = 750;
	fcntl( sock, F_SETFL, ~O_NONBLOCK );
	int result = connect( sock, (struct sockaddr*) &con, sizeof(con) );
	if( result < 0 ) {
		//printf( "[*] Error: Failed to connect. %d\r\n" );
		sprintf(line8, "[*] Error: Failed to connect. %d(%s)", errno, strerror(errno) );
		stateConnected = false;
	} else if( result == 0 ) {
		// Connected
		//printf( "[*] Connected to master\r\n" );
		sprintf(line8, "[*] Connected to master\r\n" );
		stateConnected = true;		
	}
	liveCounter = 705;
	
	
	//sprintf( line8, "[*] Connection finished test" );
	//liveCounter = 500;	
	//connectionFinished = true;
}

void writeFrame(cv::Mat frame) {
	vidOut.write( frame );
}

int main(int argc, char** argv) {
	
	memset( &info, 0x0, sizeof(info) );
	
	// Check wich mode
	int mode = MASTER;
	bool mDebug = true;
	if( argc > 2 && !strcmp( argv[1], "SLAVE" ) ) {
		mode = SLAVE;
	}
	if( argc > 1 && !strcmp( argv[1], "DEBUG" ) ) {
		mDebug = true; 
	}
	if( argc > 3 && !strcmp( argv[3], "DEBUG" ) ) {
		mDebug = true;
	}	

	
	
	// setup SPI
	wiringPiSPISetup( 0, 1000000 );
	unsigned char bytes[4];
	
	// 'Setup pin listener
	wiringPiSetup() ;
	//pinMode( 1, INPUT );
	
	// - Communcation process
	// -- Camera process
	int pidCamera = -1;
	
	// Shared Memory 
	// [Status] [Angle]
	size_t sos = sizeof(short);
	size_t som = sizeof(cv::Mat);
	size_t sol = sizeof(unsigned long);
	
	unsigned char* shared = (unsigned char*) mmap( 
		NULL, 
		1 + sos + sos + 1 + sol,
		PROT_READ | PROT_WRITE, 
		MAP_SHARED | MAP_ANONYMOUS, 
		-1, 
	0 );

	// setup Bluetooth
	
	
	fd_set readfds, writefds, exceptfds;
	timeval tv;
	tv.tv_sec = 2;
	tv.tv_usec = 100 * 1000;
	
	int stimeouts;
	int sendCounter = 0, readCounter = 0;
	uint16_t recvID = 0;
	uint16_t sendID = 0;
	
	
	if( mode == MASTER ) {
		printf( "[*] Starting as Master\r\n" );
		
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
		printf( "[*] Starting as Slave\r\n" );
		
		if( argc < 3 ) {
			printf( "[*] Error: Destination not given\r\n" );
			return 0;
		}
		
		if( !strcmp( argv[2], "PI_ONE" ) )
			memcpy( dest, PI_ONE, 18 );
		else if( !strcmp( argv[2], "PI_TWO" ) )
			memcpy( dest, PI_TWO, 18 );
		else if( !strcmp( argv[2], "PI_THREE" ) )
			memcpy( dest, PI_THREE, 18 );
		else {
			printf( "[*] Error: Unknown Destination\r\n" );
			return 0;
		}
		
		printf( "[*] Master is %s\r\n", dest );
		
		// Set connection
		con.rc_family = AF_BLUETOOTH;
		con.rc_channel = (uint8_t) 1;
		str2ba( dest, &con.rc_bdaddr );
		
		sock = socket( AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM );
		//fcntl( sock, F_SETFL, O_NONBLOCK );
		
		int status = connect( sock, (struct sockaddr*) &con, sizeof(con) );
		if( status < 0 ) {
			printf( "[*] Error: Failed to connect. %s(%d)\r\n", strerror(errno), errno );
			//sprintf(line8, "[*] Error: Failed to connect. %d(%s)", errno, strerror(errno) );
			stateConnected = false;
		} else if( status == 0 ) {
			// Connected
			printf( "[*] Connected to master\r\n" );
			//sprintf(line8, "[*] Connected to master\r\n" );
			stateConnected = true;	
		}
	}
	
	printf( "[*] Bluetooth was set up\r\n" );
	
	 
	// Create child process for camera
	pidCamera = fork();
	if( pidCamera == -1 ) {
		printf( "[*] Error: Failed to fork camera process\r\n" );
		return 0;
	}
	
	if( pidCamera == 0 ) {
		printf( "[*] Open calibration file\r\n" );
		std::ifstream file("/home/pi/soccer/calibrate.txt");
		if( !file.is_open() ) {
			printf( "[*] Error: Failed to open calibration file\r\n" );
			*shared = 0xFF;
			exit( 0 );
		}
		
		std::string line = "EMPTY";
		size_t val;
		for( int i = 0; i < 6; i++ ) {
			if( !std::getline( file, line ) ) {
				printf( "[*] Error: Calibration file wrong format\r\n" );
				*shared = 0xFF;
				exit( 0 );
			} 
			
			val = std::stoi( line.c_str(), &val );
			switch( i ) {
				case 0:
					H_MIN = val;
					printf( "[*] H_MIN: %d\r\n", H_MIN );
					break;
				case 1:
					H_MAX = val;
					printf( "[*] H_MAX: %d\r\n", H_MAX );
					break;
				case 2:
					S_MIN = val;
					printf( "[*] S_MIN: %d\r\n", S_MIN );
					break;
				case 3:
					S_MAX = val;
					printf( "[*] S_MAX: %d\r\n", S_MAX );
					break;
				case 4:
					V_MIN = val;
					printf( "[*] V_MIN: %d\r\n", V_MIN );
					break;
				case 5:
					V_MAX = val;
					printf( "[*] V_MAX: %d\r\n", V_MAX );
					break;
			}
		}
		
		cam.set( CV_CAP_PROP_FRAME_HEIGHT, HEIGHT );
		cam.set( CV_CAP_PROP_FRAME_WIDTH,  WIDTH );
		cam.set( CV_CAP_PROP_FORMAT, CV_8UC3 );
		
		printf( "[*] Open camera\r\n" );
		if( !cam.open() ) {
			printf( "[*] Error: Could not open Camera\r\n" );
			*shared = 0xFF;
			exit( 0 );
		}
		
		vidOut.open( "cam.avi", CV_FOURCC('H', '2', '6', '4'), 30, cv::Size(WIDTH, HEIGHT), true );
		
		printf( "[*] Wait for stabilisize\r\n" );
		usleep( 3 * 1000000 );
		printf( "[*] Ready for capturing\r\n" );
		
		
		time_t t = time( NULL );
		struct tm ti = *localtime(&t);
		
		cv::Size sv = cv::Size( WIDTH, HEIGHT );
		
		*shared = 0x1; 
	}
	
	// Wait for camera to be ready
	if( pidCamera > 0 ) {
		while( !(*shared) );
		if( *shared == 0xFF ) exit( 0 );
		printf( "[*] Child process ready\r\n" );
		*shared = 0; 
	} 

	// setup NCURSES
	if( mDebug ) {
		initscr();
		atexit(winquit);
		curs_set(0);
		move(0, 0);
	} 
	
	struct timespec t_start, t_end;
	struct timeval t_now, t_ball, t_button;
	unsigned short lastBall;
	
	gettimeofday( &t_button, NULL );
	while( *shared != 0xFF ) {
		
		sendDelay++;
		
		gettimeofday( &t_now, NULL );
		if( mDebug ) clear();
		
		// Bluetooth Master
		if( pidCamera > 0 && mode == MASTER ) {
			
			int cs = accept( sock, (struct sockaddr*) &client_con, &opt );
			if( cs > 0 ) {
					
				// Client has connected
				ba2str( &client_con.rc_bdaddr, clientName );
				//printf("[*] %s connected\r\n", clientName );
				sprintf(line8, "[*] %s connected", clientName );
				liveCounter = 50;
				
				// Check if client is authorized
				if( 0 && strcmp(clientName, PI_ONE) && strcmp(clientName, PI_TWO) && strcmp(clientName, PI_THREE) ) {
					close( cs );
				} else {
					client = cs;
					fcntl( client, F_SETFL, O_NONBLOCK );
				}
			}
		}
		
		connectDelay++;
		// Bluetooth Client
		if( 0 && pidCamera > 0 && mode == SLAVE && !stateConnected && connectDelay > 200 && !connectionRunning ) {
			connectDelay = 0;
		
			// Set connection
			con.rc_family = AF_BLUETOOTH;
			con.rc_channel = (uint8_t) 1;
			str2ba( dest, &con.rc_bdaddr );
		
			// Connect
			sprintf(line8, "[*] Connect to master");
			liveCounter = 75;
			
			close( sock );
			sock = socket( AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM );
			//fcntl( sock, F_SETFL, O_NONBLOCK );
			
			connectionRunning = true;
			sprintf( line8, "[*] Connect 1" );
			std::thread connect_thread( fConnect ); 
			connect_thread.detach();
			
			/* 
			int result = connect( sock, (struct sockaddr*) &con, sizeof(con) );
			sprintf( line8, "[*] Connect Try: %d - %s(%d)", result, strerror(errno), errno );
			liveCounter = 75;
			if( result == -1 && errno != EINPROGRESS ) {
				sprintf( line8, "[*] Connection-Error: %s(%d)", strerror(errno), errno );
				liveCounter = 75;
				connectionRunning = false;
			} else {
				connectionRunning = true;
				FD_ZERO( &readfds );
				FD_ZERO( &writefds );
				FD_ZERO( &exceptfds );
				
				FD_SET( sock, &readfds );
				FD_SET( sock, &writefds );
				FD_SET( sock, &exceptfds );
				
				stimeouts = 0;
			} */
			
			/*if( status < 0 ) {
				//printf( "[*] Error: Failed to connect. %d\r\n" );
				sprintf(line8, "[*] Error: Failed to connect. %d(%s)", errno, strerror(errno) );
				stateConnected = false;
			} else if( status == 0 ) {
				// Connected
				//printf( "[*] Connected to master\r\n" );
				sprintf(line8, "[*] Connected to master\r\n" );
				stateConnected = true;
				
			}*/
			
			//liveCounter = 25;
		}
		
		if( pidCamera > 0 && mode == SLAVE && connectionFinished ) {
			connectionRunning = false;
			connectionFinished = false;
			
			/*int result = select( sock+1, &readfds, &writefds, &exceptfds, &tv );
			if( result == -1 ) {
				sprintf( line8, "[*] Select Failed: %s(%d)", strerror(errno), errno );
				connectionRunning = false;
			}
			if( result == 0 ) {
				stimeouts++;
				sprintf( line8, "[*] Select timeout [%d]", stimeouts );
			
				if( stimeouts > 500 ) {
					connectionRunning = false;
					stateConnected = false;
				}
			}
			if( FD_ISSET( sock, &exceptfds ) ) {
				sprintf( line8, "[*] Connection failed" );
				connectionRunning = false;
			}
			if( FD_ISSET( sock, &readfds ) ) {
				sprintf( line8, "[*] Connected!!!" );
				connectionRunning = false;
				stateConnected = true;
			}
			if( FD_ISSET( sock, &writefds ) ) {
				sprintf( line8, "[*] Connected!!!" );
				connectionRunning = false;
				stateConnected = true;
			} */
		}

		 
		// Camera process
		if( pidCamera == 0 ) {
			clock_gettime( CLOCK_REALTIME, &t_start );
			
			// Get picture
			cam.grab();
			cam.retrieve( frame );
			
			cv::Mat mask = frame( cv::Rect(0, 0, WIDTH, 80) );
			mask.setTo( cv::Scalar(0, 0, 0) );
			
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
							flatted.at<uchar>( p.y, p.x - 1 ) = 254;
							queue.push( cv::Point( p.x - 1, p.y ) );
						}
						if( p.y > 0 && flatted.at<uchar>( p.y - 1, p.x ) == 255 ) {
							a.addPixel( cv::Point( p.x, p.y - 1 ) );
							flatted.at<uchar>( p.y - 1, p.x ) = 254;
							queue.push( cv::Point( p.x, p.y - 1 ) );
						}	
						if( p.x < flatted.cols - 1 && flatted.at<uchar>( p.y, p.x + 1 ) == 255 ) {
							a.addPixel( cv::Point( p.x + 1, p.y ) );
							flatted.at<uchar>( p.y, p.x + 1) = 254;
							queue.push( cv::Point( p.x + 1, p.y ) );
						}
						if( p.y < flatted.rows - 1 && flatted.at<uchar>( p.y + 1, p.x ) == 255 ) {
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
			for( std::vector<Area>::iterator it = areas.begin(); it != areas.end(); it++ ) {
				Area a = (Area) *it;

				if( a.getPixelCount() > bigArea.getPixelCount() ) bigArea = a;
			}
			
			float prop1 = (float) bigArea.getWidth() / (float) bigArea.getHeight();
			float prop2 = (float) bigArea.getHeight() / (float) bigArea.getWidth();
			
			// Ball found
			if( bigArea.getPixelCount() > 20 && prop1 < 5.0 && prop2 < 2.5 ) {
				cv::Point obj = bigArea.getStart();
				obj.x += bigArea.getWidth() / 2;
				obj.y += bigArea.getHeight() / 2;
				
				float diffX = obj.x - mid.x;
				float diffY = mid.y - obj.y;
				
				cv::line( frame, obj, mid, cv::Scalar( 255, 100, 100 ), 4 );
				
				float horizontal = 0, vertical = 0;
				horizontal = ((float) (obj.x - mid.x) / ((float) WIDTH / 2.0f)) * 31.0f + 31.0f + 100;
				vertical = ((float) (mid.y - obj.y) / (float) HEIGHT) * 50.0f;
		
				unsigned short shorizontal = (unsigned short) horizontal;
				unsigned short svertical = (unsigned short) vertical;
				*((unsigned short*)(shared+1)) = (unsigned short) shorizontal;
				*((unsigned short*)(shared+1+sos)) = (unsigned short) svertical;
				
			} 	else {
				// Ball not found
				*((unsigned short*)(shared+1)) = 0xFFFF;
				*((unsigned short*)(shared+1+sos)) = 0x0FFF;
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
			if( flatted.at<uchar>( tll1.y, tll1.x ) > 0 ) { 
				countL++;
				countM++;
				cv::line( frame, cv::Point(tll1.x - 2, tll1.y), cv::Point(tll1.x + 2, tll1.y), cv::Scalar(0, 255, 0), 2 );
				cv::line( frame, cv::Point(tll1.x, tll1.y - 2), cv::Point(tll1.x, tll1.y + 2), cv::Scalar(0, 255, 0), 2 );
			} else {
				cv::line( frame, cv::Point(tll1.x - 2, tll1.y), cv::Point(tll1.x + 2, tll1.y), cv::Scalar(0, 0, 255), 2 );
				cv::line( frame, cv::Point(tll1.x, tll1.y - 2), cv::Point(tll1.x, tll1.y + 2), cv::Scalar(0, 0, 255), 2 );
			}
			
			if( flatted.at<uchar>( tll2.y, tll2.x ) > 0 ) {
				countL++;
				cv::line( frame, cv::Point(tll2.x - 2, tll2.y), cv::Point(tll2.x + 2, tll2.y), cv::Scalar(0, 255, 0), 2 );
				cv::line( frame, cv::Point(tll2.x, tll2.y - 2), cv::Point(tll2.x, tll2.y + 2), cv::Scalar(0, 255, 0), 2 );
			} else {
				cv::line( frame, cv::Point(tll2.x - 2, tll2.y), cv::Point(tll2.x + 2, tll2.y), cv::Scalar(0, 0, 255), 2 );
				cv::line( frame, cv::Point(tll2.x, tll2.y - 2), cv::Point(tll2.x, tll2.y + 2), cv::Scalar(0, 0, 255), 2 );
			}
			
			if( flatted.at<uchar>( tml.y, tml.x ) > 0 ) {
				countL++; 
				countM++;
				cv::line( frame, cv::Point(tml.x - 2, tml.y), cv::Point(tml.x + 2, tml.y), cv::Scalar(0, 255, 0), 2 );
				cv::line( frame, cv::Point(tml.x, tml.y - 2), cv::Point(tml.x, tml.y + 2), cv::Scalar(0, 255, 0), 2 );
			} else {
				cv::line( frame, cv::Point(tml.x - 2, tml.y), cv::Point(tml.x + 2, tml.y), cv::Scalar(0, 0, 255), 2 );
				cv::line( frame, cv::Point(tml.x, tml.y - 2), cv::Point(tml.x, tml.y + 2), cv::Scalar(0, 0, 255), 2 );
			}
			
			if( flatted.at<uchar>( trl1.y, trl1.x ) > 0 ) {
				countL++;
				countM++;
				cv::line( frame, cv::Point(trl1.x - 2, trl1.y), cv::Point(trl1.x + 2, trl1.y), cv::Scalar(0, 255, 0), 2 );
				cv::line( frame, cv::Point(trl1.x, trl1.y - 2), cv::Point(trl1.x, trl1.y + 2), cv::Scalar(0, 255, 0), 2 );
			} else {
				cv::line( frame, cv::Point(trl1.x - 2, trl1.y), cv::Point(trl1.x + 2, trl1.y), cv::Scalar(0, 0, 255), 2 );
				cv::line( frame, cv::Point(trl1.x, trl1.y - 2), cv::Point(trl1.x, trl1.y + 2), cv::Scalar(0, 0, 255), 2 );
			}
			
			if( flatted.at<uchar>( trl2.y, trl2.x ) > 0 ) {
				countL++;
				cv::line( frame, cv::Point(trl2.x - 2, trl2.y), cv::Point(trl2.x + 2, trl2.y), cv::Scalar(0, 255, 0), 2 );
				cv::line( frame, cv::Point(trl2.x, trl2.y - 2), cv::Point(trl2.x, trl2.y + 2), cv::Scalar(0, 255, 0), 2 );
			} else {
				cv::line( frame, cv::Point(trl2.x - 2, trl2.y), cv::Point(trl2.x + 2, trl2.y), cv::Scalar(0, 0, 255), 2 );
				cv::line( frame, cv::Point(trl2.x, trl2.y - 2), cv::Point(trl2.x, trl2.y + 2), cv::Scalar(0, 0, 255), 2 );
			}
		
		
			// Check upper row testpoints
			int countU = 0;
			if( flatted.at<uchar>( tlu1.y, tlu1.x ) > 0 ) {
				countU++;
				countM++;
				cv::line( frame, cv::Point(tlu1.x - 2, tlu1.y), cv::Point(tlu1.x + 2, tlu1.y), cv::Scalar(0, 255, 0), 2 );
				cv::line( frame, cv::Point(tlu1.x, tlu1.y - 2), cv::Point(tlu1.x, tlu1.y + 2), cv::Scalar(0, 255, 0), 2 );
			} else {
				cv::line( frame, cv::Point(tlu1.x - 2, tlu1.y), cv::Point(tlu1.x + 2, tlu1.y), cv::Scalar(0, 0, 255), 2 );
				cv::line( frame, cv::Point(tlu1.x, tlu1.y - 2), cv::Point(tlu1.x, tlu1.y + 2), cv::Scalar(0, 0, 255), 2 );
			}
			if( flatted.at<uchar>( tlu2.y, tlu2.x ) > 0 ) {
				countU++;
				cv::line( frame, cv::Point(tlu2.x - 2, tlu2.y), cv::Point(tlu2.x + 2, tlu2.y), cv::Scalar(0, 255, 0), 2 );
				cv::line( frame, cv::Point(tlu2.x, tlu2.y - 2), cv::Point(tlu2.x, tlu2.y + 2), cv::Scalar(0, 255, 0), 2 );
			} else {
				cv::line( frame, cv::Point(tlu2.x - 2, tlu2.y), cv::Point(tlu2.x + 2, tlu2.y), cv::Scalar(0, 0, 255), 2 );
				cv::line( frame, cv::Point(tlu2.x, tlu2.y - 2), cv::Point(tlu2.x, tlu2.y + 2), cv::Scalar(0, 0, 255), 2 );
			}
			if( flatted.at<uchar>( tmu.y, tml.x ) > 0 ) {
				countU++;
				countM++;
				cv::line( frame, cv::Point(tmu.x - 2, tmu.y), cv::Point(tmu.x + 2, tmu.y), cv::Scalar(0, 255, 0), 2 );
				cv::line( frame, cv::Point(tmu.x, tmu.y - 2), cv::Point(tmu.x, tmu.y + 2), cv::Scalar(0, 255, 0), 2 );
			} else {
				cv::line( frame, cv::Point(tmu.x - 2, tmu.y), cv::Point(tmu.x + 2, tmu.y), cv::Scalar(0, 0, 255), 2 );
				cv::line( frame, cv::Point(tmu.x, tmu.y - 2), cv::Point(tmu.x, tmu.y + 2), cv::Scalar(0, 0, 255), 2 );
			}
			if( flatted.at<uchar>( tru1.y, tru1.x ) > 0 ) {
				countU++;
				countM++;
				cv::line( frame, cv::Point(tru1.x - 2, tru1.y), cv::Point(tru1.x + 2, tru1.y), cv::Scalar(0, 255, 0), 2 );
				cv::line( frame, cv::Point(tru1.x, tru1.y - 2), cv::Point(tru1.x, tru1.y + 2), cv::Scalar(0, 255, 0), 2 );
			} else {
				cv::line( frame, cv::Point(tru1.x - 2, tru1.y), cv::Point(tru1.x + 2, tru1.y), cv::Scalar(0, 0, 255), 2 );
				cv::line( frame, cv::Point(tru1.x, tru1.y - 2), cv::Point(tru1.x, tru1.y + 2), cv::Scalar(0, 0, 255), 2 );
			}
			if( flatted.at<uchar>( tru2.y, tru2.x ) > 0 ) {
				countU++;
				cv::line( frame, cv::Point(tru2.x - 2, tru2.y), cv::Point(tru2.x + 2, tru2.y), cv::Scalar(0, 255, 0), 2 );
				cv::line( frame, cv::Point(tru2.x, tru2.y - 2), cv::Point(tru2.x, tru2.y + 2), cv::Scalar(0, 255, 0), 2 );
			} else {
				cv::line( frame, cv::Point(tru2.x - 2, tru2.y), cv::Point(tru2.x + 2, tru2.y), cv::Scalar(0, 0, 255), 2 );
				cv::line( frame, cv::Point(tru2.x, tru2.y - 2), cv::Point(tru2.x, tru2.y + 2), cv::Scalar(0, 0, 255), 2 );
			}
			
			if( countU <= 2 && countL >= 3 && countL < 5 ) {
				cv::line( frame, cv::Point(0, 0), cv::Point(WIDTH, HEIGHT), cv::Scalar(0, 0, 255), 4 );
				*((unsigned char*) (shared+1+sos+sos+1)) = 1;
			} else if( countL >= 1 && countM > 2 && countM <= 4 ) {
				cv::line( frame, cv::Point(0, 0), cv::Point(WIDTH, HEIGHT), cv::Scalar(0, 0, 255), 4 );
				*((unsigned char*) (shared+1+sos+sos+1)) = 1;
			} else {
				*((unsigned char*) (shared+1+sos+sos+1)) = 0;
			}
			
			
			
			//std::thread t( writeFrame, frame );
			//t.detach();
			vidOut.write( frame );
			/* if( mDebug ) {
				cv::imshow( "Frame", frame );
				cv::imshow( "Filter", filtered ); 
				cv::imshow( "HSV", hsv );
				cv::waitKey( 2 );
			}*/
			
			
			clock_gettime( CLOCK_REALTIME, &t_end );
		
			unsigned long ss = t_start.tv_sec * 1000;
			unsigned long es = t_end.tv_sec * 1000;
		
			ss += t_start.tv_nsec / 1.0e6;
			es += t_end.tv_nsec / 1.0e6;
		
			unsigned long d = es - ss;
			//*((unsigned long*) (shared+1+sos+sos+1)) = 1000 / d;
			
			
		}
		
		// Main process
		if( pidCamera > 0) { 
		
			// Get value from camera
			unsigned short horizontal = *((unsigned short*)(shared+1));
			unsigned short vertical = *((unsigned short*)(shared+1+sos));
			
			if( horizontal == 0xFFFF ) {
				long long elapsed = (t_now.tv_sec * 1000000 + t_now.tv_usec) - (t_ball.tv_sec * 1000000 + t_ball.tv_usec);
				if( elapsed < 250000 ) {
					horizontal = lastBall;
				}
			} else {
				lastBall = horizontal;
				gettimeofday(&t_ball, NULL );	
			}
			
			
			info.ball1.horizontal = horizontal;
			info.ball1.vertical = vertical;
			info.status.have1 = *((unsigned char*) (shared+1+sos+sos+1));
			
			
			pOut.roboPos.x = info.roboPos2.x;
			pOut.roboPos.y = info.roboPos2.y;
			pOut.ballPos.x = info.ballPos.x;
			pOut.ballPos.y = info.ballPos.y; 
			pOut.bits.ball = info.ball1.horizontal;
			//pOut.bits.have = (info.ball1.vertical <= 12) ? 1 : 0;
			pOut.bits.have = info.status.have1;
			pOut.bits.rsvd = 0b11;
			
			if( !info.go ) pOut.bits.ball = 0x0;
			
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
			
			//info.ball1.horizontal = compass + (info.ball1.horizontal - 32 );
			//if( info.ball1.horizontal >= 360 ) info.ball1.horizontal = info.ball1.horizontal - 360;
			//if( info.ball1.horizontal < 0 ) info.ball1.horizontal = info.ball1.horizontal + 360;
			
			// Exchange data with other robot
			if( mode == MASTER && sendDelay > 3 ) {
				char tm[16];
				sendDelay = 0;
				
				if( client > 0 ) {
				
					
					pBluetooth.roboPos.x = info.roboPos1.x;
					pBluetooth.roboPos.y = info.roboPos2.y;
					pBluetooth.bits.horizontal = info.ball1.horizontal;
					pBluetooth.bits.vertical = info.ball1.vertical;
					pBluetooth.bits.onField = info.status.s1;
					pBluetooth.bits.valid = info.status.valid1;
					pBluetooth.bits.have = info.status.have1;
					pBluetooth.id = sendID;
					sendID++;
					
					
					//printf( "[*] Writing data to client..." );
					//scanf( "%s", tm );
					memcpy( buffer, &pBluetooth, sizeof(pBluetooth) );
					write( client,  buffer, sizeof(pBluetooth) );
					sendCounter++;
					
					/* printf( "-------- SEND --------\n" );
					printf( "%x %x\n", pBluetooth.roboPos.x, pBluetooth.roboPos.y );
					printf( "%x\n", pBluetooth.bits.horizontal );
					printf( "%x\n", pBluetooth.bits.vertical );
					printf( "%x\n", pBluetooth.bits.onField );
					printf( "-------- SEND --------\n" ); */
					//scanf( "%s", tm );
					
					
					//printf( "[*] Reading data from client..." );
					usleep( 2000 );
					totalRead = 0;
					do {
						bytesRead = read( client, buffer+totalRead, 32 );
						totalRead += bytesRead;
						
					} while( totalRead < sizeof(pBluetooth) && bytesRead >= 0 );
					
					
					
					if( bytesRead < 0 ) {
						//printf( "[*] Error: Read data from bluetooth failed\n" );				
						readError++;
						sprintf(line9, "[*] Error: Read data from bluetooth failed.\n" );					
						liveCounter = 20;
						if( readError > 3 ) {
							info.status.s2 = 0;
						}
					} else {
						readError = 0;
						readCounter++;
						memcpy( &pBluetooth, buffer, sizeof(pBluetooth) );
						info.roboPos2.x = pBluetooth.roboPos.x;
						info.roboPos2.y = pBluetooth.roboPos.y;
						info.ball2.horizontal = pBluetooth.bits.horizontal;
						info.ball2.vertical = pBluetooth.bits.vertical;
						info.status.s2 = pBluetooth.bits.onField;
						info.status.valid2 = pBluetooth.bits.valid;
						info.status.have2 = pBluetooth.bits.have;
						recvID = pBluetooth.id;
					
						/* printf( "-------- RECV --------\n");
						printf( "%x %x\n", pBluetooth.roboPos.x, pBluetooth.roboPos.y );
						printf( "%x\n", pBluetooth.bits.horizontal );
						printf( "%x\n", pBluetooth.bits.vertical );
						printf( "%x\n", pBluetooth.bits.onField );
						printf( "-------- RECV --------\n"); */
						//scanf( "%s", tm );
					}
				}
				
			}
			
			if( mode == SLAVE && stateConnected && sendDelay > 3 ) {
				sendDelay = 0;
				usleep( 1000 );
				
				// Reading bluetooth data
				int nloop = 0;
				totalRead = 0;
				do {
					fcntl( sock, F_SETFL, O_NONBLOCK );
					nloop++;
					bytesRead = read( sock, buffer+totalRead, 32 );
					totalRead += bytesRead;
				} while( totalRead < sizeof(pBluetooth) && bytesRead >= 0 );
				
				if( bytesRead < 0 ) {
					//printf( "[*] Error: Read data from bluetooth failed\n" );
					readError++;
					sprintf(line9, "[*] Error: Read data from bluetooth failed[%d]\n", readError );
					liveCounter = 50;
					if( readError > 3 ) {
						info.status.s2 = 0;
					}
					
					if( readError >= 100 ) {
						//stateConnected = false;
						//readError = 0;
					}
				} else {
					readError = 0;
					readCounter++;
					memcpy( &pBluetooth, buffer, sizeof(pBluetooth) );
					info.roboPos2.x = pBluetooth.roboPos.x;
					info.roboPos2.y = pBluetooth.roboPos.y;
					info.ball2.horizontal = pBluetooth.bits.horizontal;
					info.ball2.vertical = pBluetooth.bits.vertical;
					info.status.s2 = pBluetooth.bits.onField;
					info.status.valid2 = pBluetooth.bits.valid;
					info.status.have2 = pBluetooth.bits.have; 
					recvID = pBluetooth.id;
					
					/* printf( "-------- RECEIVED--------\n");
					printf( "%x %x\n", pBluetooth.roboPos.x, pBluetooth.roboPos.y );
					printf( "%x\n", pBluetooth.bits.horizontal );
					printf( "%x\n", pBluetooth.bits.vertical );
					printf( "%x\n", pBluetooth.bits.onField );
					printf( "-------- RECEIVED--------\n"); */
					//scanf( "%s", ts );
				}
				
				
				pBluetooth.roboPos.x = info.roboPos1.x;
				pBluetooth.roboPos.y = info.roboPos2.y;
				pBluetooth.bits.horizontal = info.ball1.horizontal;
				pBluetooth.bits.vertical = info.ball1.vertical;
				pBluetooth.bits.onField = info.status.s1;
				pBluetooth.bits.valid = info.status.valid1;
				pBluetooth.bits.have = info.status.have1;
				pBluetooth.id = sendID;
				sendID++;
	
				memcpy( buffer, &pBluetooth, sizeof(pBluetooth) );
				write( sock, buffer, sizeof(pBluetooth) );
				sendCounter++;
				
				/* printf( "-------- SEND --------\n");
				printf( "%x %x\n", pBluetooth.roboPos.x, pBluetooth.roboPos.y );
				printf( "%x\n", pBluetooth.bits.horizontal );
				printf( "%x\n", pBluetooth.bits.vertical );
				printf( "%x\n", pBluetooth.bits.onField );
				printf( "-------- SEND --------\n"); */
				//scanf( "%s", ts );
			}
			

			if( mDebug ) {
				clear();
				mvprintw( 0, 0, "Mode: %s", mode == MASTER ? "MASTER" : "SLAVE" );
				mvprintw( 1, 0, "My Position: (%d/%d)", info.roboPos1.x, info.roboPos1.y );
				mvprintw( 2, 0, "Other Position: (%d/%d)", info.roboPos2.x, info.roboPos2.y );
				mvprintw( 3, 0, "My Ball: %d:%d", info.ball1.horizontal, info.ball1.vertical );
				mvprintw( 4, 0, "Other Ball %d:%d", info.ball2.horizontal, info.ball2.vertical );
				mvprintw( 5, 0, "On Field: %d %d", info.status.s1, info.status.s2 );
				mvprintw( 6, 0, "Having ball: %d - %d", info.status.have1, info.status.have2 );
				//mvprintw( 7, 0, "WIFI: %s", pIn.bits.WLAN ? "ON" : "OFF" );
				mvprintw( 7, 0, "FPS: %d", *((unsigned long*) (shared+1+sos+sos+1)) );
				mvprintw( 8, 0, line8 );
				mvprintw( 9, 0, line9 );
				mvprintw( 12, 0, "BLUETOOTH SendID: %d", sendID - 1 );
				mvprintw( 13, 0, "BLUETOOTH RecvID: %d", recvID );
				refresh();
				
				
				if( liveCounter == 0 ) {
					sprintf(line8, " " );
					sprintf(line9, " " );
				} else {
					liveCounter--;
				}
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
			printf( "Having ball: %d\n", pOut.bits.have );
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
			/* if( info.wlanOld != info.wlanNew ) {
				if( info.wlanNew ) system("sudo ifconfig wlan0 up");
				if( !info.wlanNew ) system("sudo ifconfig wlan0 down");
			} */
			
			
			if( info.status.have2 ) info.go = false;
			if( info.ball2.vertical < info.ball1.vertical ) info.go = false;
			
			if( info.status.have1 ) info.go = true;
			if( !info.status.s2 ) info.go = true;
			if( info.ball1.vertical <= info.ball2.vertical ) info.go = true;
			
			
			
			/* 
			// Shutdown pin
			if( !digitalRead( 1 ) ) {
				long long elapsed = (t_now.tv_sec * 1000000 + t_now.tv_usec) - (t_button.tv_sec * 1000000 + t_button.tv_usec);
				if( elapsed > 2000000 ) {
					*shared = 0xFF;
				}
				
			} else {
				gettimeofday( &t_button, NULL );
			}			 */
			
			
					
			if( pidCamera > 0 ) usleep( 30 * 1000 );
		}
		
	}

	int state;
	if( pidCamera == 0 ) {
		printf( "[*] Camera process ended\n" );
	}
	
	if( pidCamera > 0 ) {
		if( mDebug ) endwin();
		waitpid( pidCamera, &state, 0 );
		printf( "[*] DONE\n" );
	}
	exit( 0 );
}