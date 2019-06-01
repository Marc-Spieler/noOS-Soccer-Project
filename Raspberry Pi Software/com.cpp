#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "camera.h"
#include "com.h"
#include <wiringPiSPI.h>
#include <wiringPi.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <fcntl.h>

struct PacketSPI_OUT pOut = { 0 };
struct PacketSPI_IN pIn = { 0 };


cv::VideoWriter vidOut;

struct PacketBluetooth pBluetooth = { 0 };
struct Info infoBall;
struct Info infoGoal;
struct Info infoGeneral;

volatile int comBallReady;
volatile int comGoalReady;

int tml_stat = 0;
int tll1_stat = 0;
int tll2_stat = 0;
int trl1_stat = 0;
int trl2_stat = 0;

int tmu_stat = 0;
int tlu1_stat = 0;
int tlu2_stat = 0;
int tru1_stat = 0;
int tru2_stat = 0;

// modes
#define MASTER 1
#define SLAVE 0

// bluetooth devices
const char PI_ONE[18]  = "B8:27:EB:EA:F3:FD";
const char PI_TWO[18] = "B8:27:EB:0D:95:42";

bdaddr_t temp = { 0 };
bdaddr_t* bdaddr_any = &temp;
int sock = 0;
int client = 0;
struct sockaddr_rc con = { 0 };
struct sockaddr_rc client_con = { 0 };
bool stateConnected = false;
bool connectionRunning = false;
bool connectionFinished = false;
char clientName[18];
socklen_t opt = sizeof( client_con );


// Testpoints for having ball [Test][Left/Mid/Right][Lower/Upper]
static cv::Point tml tml_par;
static cv::Point tll1 tll1_par;
static cv::Point tll2 tll2_par;
static cv::Point trl1 trl1_par;
static cv::Point trl2 trl2_par;

//static cv::Point tmu tmu_par;
//static cv::Point tlu1 tlu1_par;
//static cv::Point tlu2 tlu2_par;
//static cv::Point tru1 tru1_par;
//static cv::Point tru2 tru2_par;


void *comTask(void *arguments)
{		
	int isSLAVE = *((int *)arguments);
	printf( "isSLAVE = %d\r\n", isSLAVE );
	
	// setup SPI
	wiringPiSPISetup( 0, 1000000 );
	unsigned char bytes[4];

	// 'Setup pin listener
	wiringPiSetup() ;
	//pinMode( 1, INPUT );
	
	////connect bluetooth
	//printf( "[*] Connect bluetooth\r\n" );
	////liveCounter = 750;
	//fcntl( sock, F_SETFL, ~O_NONBLOCK );
	//int result = connect( sock, (struct sockaddr*) &con, sizeof(con) );
	//if( result < 0 ) {
		////printf( "[*] Error: Failed to connect. %d\r\n" );
		//printf("[*] Error: Failed to connect. %d(%s)\r\n", errno, strerror(errno) );
		//stateConnected = false;
	//} else if( result == 0 ) {
		//// Connected
		////printf( "[*] Connected to master\r\n" );
		//printf("[*] Connected to master\r\n" );
		//stateConnected = true;		
	//}
	////liveCounter = 705;
	
	
	
	time_t rawtime;
	struct tm *dateTime;
	char filenameBuffer[80];

	time( &rawtime );

	dateTime = localtime( &rawtime );
	strftime( filenameBuffer, 80,"cam_%H_%M.avi", dateTime );
	
	printf("%s",filenameBuffer);
	vidOut.open( filenameBuffer, CV_FOURCC('H', '2', '6', '4'), 30, cv::Size(WIDTH, HEIGHT), true );
	
	cv::Size sv = cv::Size( WIDTH, HEIGHT );
	cv::Point mid( WIDTH / 2, HEIGHT );
#if 0	
	//Master/Slave
	
	if( isSLAVE == 0 ) 
	//if( mode == MASTER ) 
	{
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
	
	if( isSLAVE != 0 ) 
	//if( mode == SLAVE ) 
	{
		printf( "[*] Starting as Slave\r\n" );

		
		// Set connection
		con.rc_family = AF_BLUETOOTH;
		con.rc_channel = (uint8_t) 1;
		str2ba( PI_ONE, &con.rc_bdaddr );
		
		sock = socket( AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM );
		//fcntl( sock, F_SETFL, O_NONBLOCK );
		

		if( connect( sock, (struct sockaddr*) &con, sizeof(con) ) != 0 ) 
		{
			printf( "[*] Error: Failed to connect. %s(%d)\r\n", strerror(errno), errno );
			//sprintf(line8, "[*] Error: Failed to connect. %d(%s)", errno, strerror(errno) );
			stateConnected = false;
		} else 
		{
			// Connected
			printf( "[*] Connected to master\r\n" );
			//sprintf(line8, "[*] Connected to master\r\n" );
			stateConnected = true;	
		}
	}
	
	printf( "[*] Bluetooth was set up\r\n" );
	
	while (1)
	{
		if( isSLAVE == 0 ) {
			
			int cs = accept( sock, (struct sockaddr*) &client_con, &opt );
			if( cs > 0 ) {
					
				// Client has connected
				ba2str( &client_con.rc_bdaddr, clientName );
				//printf("[*] %s connected\r\n", clientName );
				printf( "[*] %s connected", clientName );
				//liveCounter = 50;
				
				// Check if client is authorized
				if( strcmp(clientName, PI_TWO) ) {
					close( cs );
				} else {
					client = cs;
					fcntl( client, F_SETFL, O_NONBLOCK );
				}
			}
		}
	}
#endif	
	while (1)
	{
		if((comBallReady==1)&&(comGoalReady==1))
		{		
			
			pOut.bits.ball = infoBall.ball1.horizontal;
			pOut.bits.GoalHalfWidth = infoGoal.ball1.GoalHalfWidth;
			pOut.bits.seeBall = infoBall.status.see;
			pOut.bits.have = infoBall.status.have1;
			pOut.bits.rsvd_1 = 0;
			pOut.bits.rsvd_2 = 0;

			pOut.bits.goal = infoGoal.ball1.horizontal;			
			pOut.bits.seeGoal = infoGoal.status.see;
			
			
			memcpy( bytes, &pOut, sizeof(pOut) );

			wiringPiSPIDataRW( 0, bytes, sizeof(pOut) );
			memcpy( &pIn, bytes, sizeof(pIn) );
			
			//draw line to goal/ball for video output
			if (infoBall.status.see == 1)
			{				
			cv::line( frame, objBall, mid, cv::Scalar( 255, 100, 100 ), 4 );
			}
			
			if ( infoGoal.status.see == 1)
			{
			cv::line( frame, objGoal, mid, cv::Scalar( 100, 255, 100 ), 4 );
			}
		  
			
			
			if (tll1_stat==1)
			{
			cv::line( frame, cv::Point(tll1.x - 2, tll1.y), cv::Point(tll1.x + 2, tll1.y), cv::Scalar(0, 255, 0), 2 );
			cv::line( frame, cv::Point(tll1.x, tll1.y - 2), cv::Point(tll1.x, tll1.y + 2), cv::Scalar(0, 255, 0), 2 );
			}
			else
			{
			cv::line( frame, cv::Point(tll1.x - 2, tll1.y), cv::Point(tll1.x + 2, tll1.y), cv::Scalar(255, 0, 0), 2 );
			cv::line( frame, cv::Point(tll1.x, tll1.y - 2), cv::Point(tll1.x, tll1.y + 2), cv::Scalar(255, 0, 0), 2 );	
			}
			
			if ( tll2_stat==1 )
			{
			cv::line( frame, cv::Point(tll2.x - 2, tll2.y), cv::Point(tll2.x + 2, tll2.y), cv::Scalar(0, 255, 0), 2 );
			cv::line( frame, cv::Point(tll2.x, tll2.y - 2), cv::Point(tll2.x, tll2.y + 2), cv::Scalar(0, 255, 0), 2 );
			}
			else
			{
			cv::line( frame, cv::Point(tll2.x - 2, tll2.y), cv::Point(tll2.x + 2, tll2.y), cv::Scalar(255, 0, 0), 2 );
			cv::line( frame, cv::Point(tll2.x, tll2.y - 2), cv::Point(tll2.x, tll2.y + 2), cv::Scalar(255, 0, 0), 2 );	
			}
			
			if ( tml_stat==1 )
			{
			cv::line( frame, cv::Point(tml.x - 2, tml.y), cv::Point(tml.x + 2, tml.y), cv::Scalar(0, 255, 0), 2 );
			cv::line( frame, cv::Point(tml.x, tml.y - 2), cv::Point(tml.x, tml.y + 2), cv::Scalar(0, 255, 0), 2 );
			}
			else
			{
			cv::line( frame, cv::Point(tml.x - 2, tml.y), cv::Point(tml.x + 2, tml.y), cv::Scalar(255, 0, 0), 2 );
			cv::line( frame, cv::Point(tml.x, tml.y - 2), cv::Point(tml.x, tml.y + 2), cv::Scalar(255, 0, 0), 2 );
			}
			if ( trl1_stat==1 )
			{
			cv::line( frame, cv::Point(trl1.x - 2, trl1.y), cv::Point(trl1.x + 2, trl1.y), cv::Scalar(0, 255, 0), 2 );
			cv::line( frame, cv::Point(trl1.x, trl1.y - 2), cv::Point(trl1.x, trl1.y + 2), cv::Scalar(0, 255, 0), 2 );
			}
			else
			{
			cv::line( frame, cv::Point(trl1.x - 2, trl1.y), cv::Point(trl1.x + 2, trl1.y), cv::Scalar(255, 0, 0), 2 );
			cv::line( frame, cv::Point(trl1.x, trl1.y - 2), cv::Point(trl1.x, trl1.y + 2), cv::Scalar(255, 0, 0), 2 );
			}
			if ( trl2_stat==1 )
			{
			cv::line( frame, cv::Point(trl2.x - 2, trl2.y), cv::Point(trl2.x + 2, trl2.y), cv::Scalar(0, 255, 0), 2 );
			cv::line( frame, cv::Point(trl2.x, trl2.y - 2), cv::Point(trl2.x, trl2.y + 2), cv::Scalar(0, 255, 0), 2 );
			}
			else
			{
			cv::line( frame, cv::Point(trl2.x - 2, trl2.y), cv::Point(trl2.x + 2, trl2.y), cv::Scalar(255, 0, 0), 2 );
			cv::line( frame, cv::Point(trl2.x, trl2.y - 2), cv::Point(trl2.x, trl2.y + 2), cv::Scalar(255, 0, 0), 2 );
			}
			
#if 0
			if ( tlu1_stat==1 )
			{
			cv::line( frame, cv::Point(tlu1.x - 2, tlu1.y), cv::Point(tlu1.x + 2, tlu1.y), cv::Scalar(0, 255, 0), 2 );
			cv::line( frame, cv::Point(tlu1.x, tlu1.y - 2), cv::Point(tlu1.x, tlu1.y + 2), cv::Scalar(0, 255, 0), 2 );
			}
			else
			{
			cv::line( frame, cv::Point(tlu1.x - 2, tlu1.y), cv::Point(tlu1.x + 2, tlu1.y), cv::Scalar(255, 0, 0), 2 );
			cv::line( frame, cv::Point(tlu1.x, tlu1.y - 2), cv::Point(tlu1.x, tlu1.y + 2), cv::Scalar(255, 0, 0), 2 );
			}
			if ( tlu2_stat==1 )
			{
			cv::line( frame, cv::Point(tlu2.x - 2, tlu2.y), cv::Point(tlu2.x + 2, tlu2.y), cv::Scalar(0, 255, 0), 2 );
			cv::line( frame, cv::Point(tlu2.x, tlu2.y - 2), cv::Point(tlu2.x, tlu2.y + 2), cv::Scalar(0, 255, 0), 2 );
			}
			else
			{
			cv::line( frame, cv::Point(tlu2.x - 2, tlu2.y), cv::Point(tlu2.x + 2, tlu2.y), cv::Scalar(255, 0, 0), 2 );
			cv::line( frame, cv::Point(tlu2.x, tlu2.y - 2), cv::Point(tlu2.x, tlu2.y + 2), cv::Scalar(255, 0, 0), 2 );
			}
			if ( tmu_stat==1 )
			{
			cv::line( frame, cv::Point(tmu.x - 2, tmu.y), cv::Point(tmu.x + 2, tmu.y), cv::Scalar(0, 255, 0), 2 );
			cv::line( frame, cv::Point(tmu.x, tmu.y - 2), cv::Point(tmu.x, tmu.y + 2), cv::Scalar(0, 255, 0), 2 );
			}
			else
			{
			cv::line( frame, cv::Point(tmu.x - 2, tmu.y), cv::Point(tmu.x + 2, tmu.y), cv::Scalar(255, 0, 0), 2 );
			cv::line( frame, cv::Point(tmu.x, tmu.y - 2), cv::Point(tmu.x, tmu.y + 2), cv::Scalar(255, 0, 0), 2 );
			}
			if ( tru1_stat==1 )
			{
			cv::line( frame, cv::Point(tru1.x - 2, tru1.y), cv::Point(tru1.x + 2, tru1.y), cv::Scalar(0, 255, 0), 2 );
			cv::line( frame, cv::Point(tru1.x, tru1.y - 2), cv::Point(tru1.x, tru1.y + 2), cv::Scalar(0, 255, 0), 2 );
			}
			else
			{
			cv::line( frame, cv::Point(tru1.x - 2, tru1.y), cv::Point(tru1.x + 2, tru1.y), cv::Scalar(255, 0, 0), 2 );
			cv::line( frame, cv::Point(tru1.x, tru1.y - 2), cv::Point(tru1.x, tru1.y + 2), cv::Scalar(255, 0, 0), 2 );
			}
			if ( tru2_stat==1 )
			{
			cv::line( frame, cv::Point(tru2.x - 2, tru2.y), cv::Point(tru2.x + 2, tru2.y), cv::Scalar(0, 255, 0), 2 );
			cv::line( frame, cv::Point(tru2.x, tru2.y - 2), cv::Point(tru2.x, tru2.y + 2), cv::Scalar(0, 255, 0), 2 );
			}
			else
			{
			cv::line( frame, cv::Point(tru2.x - 2, tru2.y), cv::Point(tru2.x + 2, tru2.y), cv::Scalar(255, 0, 0), 2 );
			cv::line( frame, cv::Point(tru2.x, tru2.y - 2), cv::Point(tru2.x, tru2.y + 2), cv::Scalar(255, 0, 0), 2 );
			}
#endif
    
    
    
    vidOut.write( frame );

			comBallReady=0; //reset signal for com thread to begin its task
			comGoalReady=0;	//reset signal for com thread to begin its task		
		}
			
		usleep(10*1000); 
	}
}
