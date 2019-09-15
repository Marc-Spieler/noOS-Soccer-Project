#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "camera.h"

//#define measureFramerate


// Images
static raspicam::RaspiCam_Cv cam;
cv::Mat frame;
cv::Mat frameTmp;
cv::Mat frameOut;
cv::Mat hsv;

cv::Point objBall;
cv::Point objGoal;


int frameCount = 0;
int frameBallReady = 1;	// will be cleared after ball init
int frameGoalReady = 1;	// will be cleared after goal init

//pthread_mutex_t ready_mutex = PTHREAD_MUTEX_INITIALIZER;	

int TopBorder = 80;
uint8_t minT = 255;
uint8_t maxT = 0;
uint16_t avgT = 0;


void *cameraTask(void *arguments)
{
  struct timespec t_start, t_end;
  int frameBallReadyLocal = 0;
  int frameGoalReadyLocal = 0;
  //open camera
  cam.set( CV_CAP_PROP_FRAME_HEIGHT, HEIGHT );
  cam.set( CV_CAP_PROP_FRAME_WIDTH,  WIDTH );
  cam.set( CV_CAP_PROP_FORMAT, CV_8UC3 );
  
  printf( "[*] Open camera\r\n" );
  if( !cam.open() ) 
  {
    printf( "[*] Error: Could not open Camera\r\n" );
    exit( 0 );
  }
  
  
  printf( "[*] Wait for stabilisize\r\n" );
  usleep( 3 * 1000000 );
  printf( "[*] Ready for capturing\r\n" );

  // moved into corresponding files
  //frameBallReady = 0;
  //frameGoalReady = 0;

  while (1)
  {
    //pthread_mutex_lock(&ready_mutex);
    frameBallReadyLocal = frameBallReady;
    frameGoalReadyLocal = frameGoalReady;
    //pthread_mutex_unlock(&ready_mutex);
    
    if ((frameBallReadyLocal==0)&&(frameGoalReadyLocal==0))
    {
		
#ifdef measureFramerate
      clock_gettime( CLOCK_REALTIME, &t_start );
#endif


      // Get picture
      cam.grab();
      cam.retrieve( frame );
      // add TopBorder mask
      cv::Mat mask = frame( cv::Rect(0, 0, WIDTH, TopBorder) );
      mask.setTo( cv::Scalar(0, 0, 0) );

      // Convert image from BGR to HSV
      cv::cvtColor( frame, hsv, cv::COLOR_RGB2HSV );
      
      frameTmp = frame.clone();
      
      //pthread_mutex_lock(&ready_mutex);
      frameBallReady = 1; //signal for ball thread to begin its task
      frameGoalReady = 1; //signal for goal thread to begin its task
      //pthread_mutex_unlock(&ready_mutex);
      
      
#ifdef measureFramerate
      clock_gettime( CLOCK_REALTIME, &t_end );
	  //whole secounds to microsecounds
      unsigned long ss = t_start.tv_sec * 1000;
      unsigned long es = t_end.tv_sec * 1000;

	  //nanosecounds to microsecounds
      ss += t_start.tv_nsec / 1.0e6;
      es += t_end.tv_nsec / 1.0e6;

      unsigned long d = es - ss;
      uint8_t framerate = 1000 / d;
    
      if (minT > framerate)
      {
         minT = framerate;
      }
	
      if (maxT < framerate)
      {
         maxT = framerate;
      }
	
      avgT = ( avgT*95 + framerate*5 ) / 100;
      if (++frameCount == 100)
      {
        //*((unsigned long*) (shared+1+sos+sos+1)) = 1000 / d;
	    printf( "Frames: %d minT: %d maxT: %d avgT: %d\n", framerate, minT, maxT, avgT );
	    frameCount = 0;
      }
#endif

    }  // if ((frameBallReady==0)&&(frameGoalReady==0))
    else
    {
      usleep(1000);
    }
    
  
  }  // while (1)
}