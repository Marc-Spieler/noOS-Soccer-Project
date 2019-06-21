#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "camera.h"
#include "com.h"
#include <wiringPiSPI.h>
#include <wiringPi.h>

struct PacketSPI_OUT pOut = { 0 };
struct PacketSPI_IN pIn = { 0 };


cv::VideoWriter vidOut;

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


void *comTask(void *arguments)
{		
	int index = *((int *)arguments);

	
	int comBallReadyLocal = 0;
	int comGoalReadyLocal = 0;
 

	// setup SPI
	wiringPiSPISetup( 0, 1000000 );
	unsigned char bytes[4];

	// 'Setup pin listener
	wiringPiSetup() ;
	//pinMode( 1, INPUT );

	time_t rawtime;
	struct tm *dateTime;
	char filenameBuffer[80];

	// wait until others threads are initialized
	while((frameBallReady==1) || (frameGoalReady==1));

	time( &rawtime );
	dateTime = localtime( &rawtime );
	strftime( filenameBuffer, 80,"cam_%H_%M.avi", dateTime );
	printf("%s",filenameBuffer);
	vidOut.open( filenameBuffer, CV_FOURCC('H', '2', '6', '4'), 30, cv::Size(WIDTH, HEIGHT), true );
	
	cv::Size sv = cv::Size( WIDTH, HEIGHT );
	cv::Point mid( WIDTH / 2, HEIGHT );

	while (1)
	{	
		//pthread_mutex_lock(&ready_mutex);
        comBallReadyLocal = comBallReady;
        comGoalReadyLocal = comGoalReady;
        //pthread_mutex_unlock(&ready_mutex);
        
		if((comBallReadyLocal==1)&&(comGoalReadyLocal==1))
		{		
			
			pOut.bits.ball = infoBall.ball1.horizontal;
			pOut.bits.GoalHalfWidth = infoGoal.ball1.GoalHalfWidth;
			pOut.bits.seeBall = infoBall.status.see;
			pOut.bits.have = infoBall.status.have1;
			pOut.bits.haveFar = infoBall.status.haveFar;
			pOut.bits.rsvd_1 = 0;
			pOut.bits.rsvd_2 = 0;

			pOut.bits.goal = infoGoal.ball1.horizontal;			
			pOut.bits.seeGoal = infoGoal.status.see;
			
			
			memcpy( bytes, &pOut, sizeof(pOut) );

			wiringPiSPIDataRW( 0, bytes, sizeof(pOut) );
			memcpy( &pIn, bytes, sizeof(pIn) );
			//draw info
			//cv::putText(frame, "test", (40, 40), cv:FONT_HERSHEY_SCRIPT_SIMPLEX, 2, cv::Scalar::all(255), 3, 8);
			
			//draw line to goal/ball for video output
			if (infoBall.status.see == 1)
			{				
			cv::line( frameOut, objBall, mid, cv::Scalar( 255, 100, 100 ), 4 );
			}
			
			if ( infoGoal.status.see == 1)
			{
			cv::line( frameOut, objGoal, mid, cv::Scalar( 100, 255, 100 ), 4 );
			}
		  
			
			
			if (tll1_stat==1)
			{
			cv::line( frameOut, cv::Point(tll1.x - 2, tll1.y), cv::Point(tll1.x + 2, tll1.y), cv::Scalar(0, 255, 0), 2 );
			cv::line( frameOut, cv::Point(tll1.x, tll1.y - 2), cv::Point(tll1.x, tll1.y + 2), cv::Scalar(0, 255, 0), 2 );
			}
			else
			{
			cv::line( frameOut, cv::Point(tll1.x - 2, tll1.y), cv::Point(tll1.x + 2, tll1.y), cv::Scalar(255, 0, 0), 2 );
			cv::line( frameOut, cv::Point(tll1.x, tll1.y - 2), cv::Point(tll1.x, tll1.y + 2), cv::Scalar(255, 0, 0), 2 );	
			}
			
			if ( tll2_stat==1 )
			{
			cv::line( frameOut, cv::Point(tll2.x - 2, tll2.y), cv::Point(tll2.x + 2, tll2.y), cv::Scalar(0, 255, 0), 2 );
			cv::line( frameOut, cv::Point(tll2.x, tll2.y - 2), cv::Point(tll2.x, tll2.y + 2), cv::Scalar(0, 255, 0), 2 );
			}
			else
			{
			cv::line( frameOut, cv::Point(tll2.x - 2, tll2.y), cv::Point(tll2.x + 2, tll2.y), cv::Scalar(255, 0, 0), 2 );
			cv::line( frameOut, cv::Point(tll2.x, tll2.y - 2), cv::Point(tll2.x, tll2.y + 2), cv::Scalar(255, 0, 0), 2 );	
			}
			
			if ( tml_stat==1 )
			{
			cv::line( frameOut, cv::Point(tml.x - 2, tml.y), cv::Point(tml.x + 2, tml.y), cv::Scalar(0, 255, 0), 2 );
			cv::line( frameOut, cv::Point(tml.x, tml.y - 2), cv::Point(tml.x, tml.y + 2), cv::Scalar(0, 255, 0), 2 );
			}
			else
			{
			cv::line( frameOut, cv::Point(tml.x - 2, tml.y), cv::Point(tml.x + 2, tml.y), cv::Scalar(255, 0, 0), 2 );
			cv::line( frameOut, cv::Point(tml.x, tml.y - 2), cv::Point(tml.x, tml.y + 2), cv::Scalar(255, 0, 0), 2 );
			}
			if ( trl1_stat==1 )
			{
			cv::line( frameOut, cv::Point(trl1.x - 2, trl1.y), cv::Point(trl1.x + 2, trl1.y), cv::Scalar(0, 255, 0), 2 );
			cv::line( frameOut, cv::Point(trl1.x, trl1.y - 2), cv::Point(trl1.x, trl1.y + 2), cv::Scalar(0, 255, 0), 2 );
			}
			else
			{
			cv::line( frameOut, cv::Point(trl1.x - 2, trl1.y), cv::Point(trl1.x + 2, trl1.y), cv::Scalar(255, 0, 0), 2 );
			cv::line( frameOut, cv::Point(trl1.x, trl1.y - 2), cv::Point(trl1.x, trl1.y + 2), cv::Scalar(255, 0, 0), 2 );
			}
			if ( trl2_stat==1 )
			{
			cv::line( frameOut, cv::Point(trl2.x - 2, trl2.y), cv::Point(trl2.x + 2, trl2.y), cv::Scalar(0, 255, 0), 2 );
			cv::line( frameOut, cv::Point(trl2.x, trl2.y - 2), cv::Point(trl2.x, trl2.y + 2), cv::Scalar(0, 255, 0), 2 );
			}
			else
			{
			cv::line( frameOut, cv::Point(trl2.x - 2, trl2.y), cv::Point(trl2.x + 2, trl2.y), cv::Scalar(255, 0, 0), 2 );
			cv::line( frameOut, cv::Point(trl2.x, trl2.y - 2), cv::Point(trl2.x, trl2.y + 2), cv::Scalar(255, 0, 0), 2 );
			}
			

			if ( tlu1_stat==1 )
			{
			cv::line( frameOut, cv::Point(tlu1.x - 2, tlu1.y), cv::Point(tlu1.x + 2, tlu1.y), cv::Scalar(0, 255, 0), 2 );
			cv::line( frameOut, cv::Point(tlu1.x, tlu1.y - 2), cv::Point(tlu1.x, tlu1.y + 2), cv::Scalar(0, 255, 0), 2 );
			}
			else
			{
			cv::line( frameOut, cv::Point(tlu1.x - 2, tlu1.y), cv::Point(tlu1.x + 2, tlu1.y), cv::Scalar(255, 0, 0), 2 );
			cv::line( frameOut, cv::Point(tlu1.x, tlu1.y - 2), cv::Point(tlu1.x, tlu1.y + 2), cv::Scalar(255, 0, 0), 2 );
			}
			//if ( tlu2_stat==1 )
			//{
			//cv::line( frameOut, cv::Point(tlu2.x - 2, tlu2.y), cv::Point(tlu2.x + 2, tlu2.y), cv::Scalar(0, 255, 0), 2 );
			//cv::line( frameOut, cv::Point(tlu2.x, tlu2.y - 2), cv::Point(tlu2.x, tlu2.y + 2), cv::Scalar(0, 255, 0), 2 );
			//}
			//else
			//{
			//cv::line( frameOut, cv::Point(tlu2.x - 2, tlu2.y), cv::Point(tlu2.x + 2, tlu2.y), cv::Scalar(255, 0, 0), 2 );
			//cv::line( frameOut, cv::Point(tlu2.x, tlu2.y - 2), cv::Point(tlu2.x, tlu2.y + 2), cv::Scalar(255, 0, 0), 2 );
			//}
			if ( tmu_stat==1 )
			{
			cv::line( frameOut, cv::Point(tmu.x - 2, tmu.y), cv::Point(tmu.x + 2, tmu.y), cv::Scalar(0, 255, 0), 2 );
			cv::line( frameOut, cv::Point(tmu.x, tmu.y - 2), cv::Point(tmu.x, tmu.y + 2), cv::Scalar(0, 255, 0), 2 );
			}
			else
			{
			cv::line( frameOut, cv::Point(tmu.x - 2, tmu.y), cv::Point(tmu.x + 2, tmu.y), cv::Scalar(255, 0, 0), 2 );
			cv::line( frameOut, cv::Point(tmu.x, tmu.y - 2), cv::Point(tmu.x, tmu.y + 2), cv::Scalar(255, 0, 0), 2 );
			}
			if ( tru1_stat==1 )
			{
			cv::line( frameOut, cv::Point(tru1.x - 2, tru1.y), cv::Point(tru1.x + 2, tru1.y), cv::Scalar(0, 255, 0), 2 );
			cv::line( frameOut, cv::Point(tru1.x, tru1.y - 2), cv::Point(tru1.x, tru1.y + 2), cv::Scalar(0, 255, 0), 2 );
			}
			else
			{
			cv::line( frameOut, cv::Point(tru1.x - 2, tru1.y), cv::Point(tru1.x + 2, tru1.y), cv::Scalar(255, 0, 0), 2 );
			cv::line( frameOut, cv::Point(tru1.x, tru1.y - 2), cv::Point(tru1.x, tru1.y + 2), cv::Scalar(255, 0, 0), 2 );
			}
			//if ( tru2_stat==1 )
			//{
			//cv::line( frameOut, cv::Point(tru2.x - 2, tru2.y), cv::Point(tru2.x + 2, tru2.y), cv::Scalar(0, 255, 0), 2 );
			//cv::line( frameOut, cv::Point(tru2.x, tru2.y - 2), cv::Point(tru2.x, tru2.y + 2), cv::Scalar(0, 255, 0), 2 );
			//}
			//else
			//{
			//cv::line( frameOut, cv::Point(tru2.x - 2, tru2.y), cv::Point(tru2.x + 2, tru2.y), cv::Scalar(255, 0, 0), 2 );
			//cv::line( frameOut, cv::Point(tru2.x, tru2.y - 2), cv::Point(tru2.x, tru2.y + 2), cv::Scalar(255, 0, 0), 2 );
			//}

    
    
    
			vidOut.write( frameOut );
			//pthread_mutex_lock(&ready_mutex);
			comBallReady=0; //reset signal for com thread to begin its task
			comGoalReady=0;	//reset signal for com thread to begin its task	
			//pthread_mutex_unlock(&ready_mutex);	
		}
		else
		{
			usleep(1000);
		} 
	}
}
