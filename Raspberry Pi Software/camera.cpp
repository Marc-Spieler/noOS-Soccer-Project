#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "camera.h"

// Images
static raspicam::RaspiCam_Cv cam;
static cv::Mat frame;
cv::Mat hsv;
cv::Point objBall;
cv::Point objGoal;
cv::VideoWriter vidOut;


int frameBallReady = 0;
int frameGoalReady = 0;
int TopBorder = 80;

void *cameraTask(void *arguments)
{
  int index = *((int *)arguments);
  
  cam.set( CV_CAP_PROP_FRAME_HEIGHT, HEIGHT );
  cam.set( CV_CAP_PROP_FRAME_WIDTH,  WIDTH );
  cam.set( CV_CAP_PROP_FORMAT, CV_8UC3 );
  
  printf( "[*] Open camera\r\n" );
  if( !cam.open() ) 
  {
    printf( "[*] Error: Could not open Camera\r\n" );
    exit( 0 );
  }
  
  vidOut.open( "cam.avi", CV_FOURCC('H', '2', '6', '4'), 30, cv::Size(WIDTH, HEIGHT), true );
  
  printf( "[*] Wait for stabilisize\r\n" );
  usleep( 3 * 1000000 );
  printf( "[*] Ready for capturing\r\n" );
  
  
  //time_t t = time( NULL );
  //struct tm ti = *localtime(&t);
  
  cv::Size sv = cv::Size( WIDTH, HEIGHT );
  cv::Point mid( WIDTH / 2, HEIGHT );

  
#if 0
  // setup NCURSES
  if( mDebug ) 
  {
    initscr();
    atexit(winquit);
    curs_set(0);
    move(0, 0);
  }
#endif
  frameBallReady = 0;
  frameGoalReady = 0;
  while (1)
  {
    if ((frameBallReady==0)&&(frameGoalReady==0))
    {
    //clock_gettime( CLOCK_REALTIME, &t_start );

    // Get picture
    cam.grab();
    cam.retrieve( frame );
    
    cv::Mat mask = frame( cv::Rect(0, 0, WIDTH, TopBorder) );
    mask.setTo( cv::Scalar(0, 0, 0) );
//    frameBall = frame.clone();
//    frameGoal = frame.clone();
//    frameBall.setTo( cv::Scalar(0, 0, 0) );
//    frameGoal.setTo( cv::Scalar(0, 0, 0) );
    
    // Convert image from BGR to HSV
    cv::cvtColor( frame, hsv, cv::COLOR_RGB2HSV );
    frameBallReady = 1;
    frameGoalReady = 1;
    //std::thread t( writeFrame, frame );
    //t.detach();

    while ((frameBallReady==1)&&(frameGoalReady==1));
	cv::line( frame, objBall, mid, cv::Scalar( 255, 100, 100 ), 4 );
	cv::line( frame, objGoal, mid, cv::Scalar( 100, 255, 100 ), 4 );
    vidOut.write( frame );
    /* if( mDebug ) {
      cv::imshow( "Frame", frame );
      cv::imshow( "Filter", filtered ); 
      cv::imshow( "HSV", hsv );
      cv::waitKey( 2 );
    }*/
    
    #if 0
    clock_gettime( CLOCK_REALTIME, &t_end );

    unsigned long ss = t_start.tv_sec * 1000;
    unsigned long es = t_end.tv_sec * 1000;

    ss += t_start.tv_nsec / 1.0e6;
    es += t_end.tv_nsec / 1.0e6;

    unsigned long d = es - ss;
    //*((unsigned long*) (shared+1+sos+sos+1)) = 1000 / d;
    #endif


}
else
  {
    usleep(10000);
  }
    

}
}
