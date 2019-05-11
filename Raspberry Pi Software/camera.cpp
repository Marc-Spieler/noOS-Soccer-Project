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

// Testpoints for having ball [Test][Left/Mid/Right][Lower/Upper]
static cv::Point tml tml_par;
static cv::Point tll1 tll1_par;
static cv::Point tll2 tll2_par;
static cv::Point trl1 trl1_par;
static cv::Point trl2 trl2_par;

static cv::Point tmu tmu_par;
static cv::Point tlu1 tlu1_par;
static cv::Point tlu2 tlu2_par;
static cv::Point tru1 tru1_par;
static cv::Point tru2 tru2_par;

void *cameraTask(void *arguments)
{
  int index = *((int *)arguments);
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
    // add TopBorder mask
    cv::Mat mask = frame( cv::Rect(0, 0, WIDTH, TopBorder) );
    mask.setTo( cv::Scalar(0, 0, 0) );
//    frameBall = frame.clone();
//    frameGoal = frame.clone();
//    frameBall.setTo( cv::Scalar(0, 0, 0) );
//    frameGoal.setTo( cv::Scalar(0, 0, 0) );
    
    // Convert image from BGR to HSV
    cv::cvtColor( frame, hsv, cv::COLOR_RGB2HSV );
    frameBallReady = 1; //signal for ball thread to begin its task
    frameGoalReady = 1; //signal for goal thread to begin its task
    //std::thread t( writeFrame, frame );
    //t.detach();
    
	//draw line to goal/ball for video output
    while ((frameBallReady==1)&&(frameGoalReady==1));
	cv::line( frame, objBall, mid, cv::Scalar( 255, 100, 100 ), 4 );
	cv::line( frame, objGoal, mid, cv::Scalar( 100, 255, 100 ), 4 );
	
	
	cv::line( frame, cv::Point(tll1.x - 2, tll1.y), cv::Point(tll1.x + 2, tll1.y), cv::Scalar(0, 255, 0), 2 );
	cv::line( frame, cv::Point(tll1.x, tll1.y - 2), cv::Point(tll1.x, tll1.y + 2), cv::Scalar(0, 255, 0), 2 );
	cv::line( frame, cv::Point(tll2.x - 2, tll2.y), cv::Point(tll2.x + 2, tll2.y), cv::Scalar(0, 255, 0), 2 );
	cv::line( frame, cv::Point(tll2.x, tll2.y - 2), cv::Point(tll2.x, tll2.y + 2), cv::Scalar(0, 255, 0), 2 );
	cv::line( frame, cv::Point(tml.x - 2, tml.y), cv::Point(tml.x + 2, tml.y), cv::Scalar(0, 255, 0), 2 );
	cv::line( frame, cv::Point(tml.x, tml.y - 2), cv::Point(tml.x, tml.y + 2), cv::Scalar(0, 255, 0), 2 );
	cv::line( frame, cv::Point(trl1.x - 2, trl1.y), cv::Point(trl1.x + 2, trl1.y), cv::Scalar(0, 255, 0), 2 );
	cv::line( frame, cv::Point(trl1.x, trl1.y - 2), cv::Point(trl1.x, trl1.y + 2), cv::Scalar(0, 255, 0), 2 );
	cv::line( frame, cv::Point(trl2.x - 2, trl2.y), cv::Point(trl2.x + 2, trl2.y), cv::Scalar(0, 255, 0), 2 );
	cv::line( frame, cv::Point(trl2.x, trl2.y - 2), cv::Point(trl2.x, trl2.y + 2), cv::Scalar(0, 255, 0), 2 );
	cv::line( frame, cv::Point(tlu1.x - 2, tlu1.y), cv::Point(tlu1.x + 2, tlu1.y), cv::Scalar(0, 255, 0), 2 );
	cv::line( frame, cv::Point(tlu1.x, tlu1.y - 2), cv::Point(tlu1.x, tlu1.y + 2), cv::Scalar(0, 255, 0), 2 );
	cv::line( frame, cv::Point(tlu2.x - 2, tlu2.y), cv::Point(tlu2.x + 2, tlu2.y), cv::Scalar(0, 255, 0), 2 );
	cv::line( frame, cv::Point(tlu2.x, tlu2.y - 2), cv::Point(tlu2.x, tlu2.y + 2), cv::Scalar(0, 255, 0), 2 );
	cv::line( frame, cv::Point(tmu.x - 2, tmu.y), cv::Point(tmu.x + 2, tmu.y), cv::Scalar(0, 255, 0), 2 );
	cv::line( frame, cv::Point(tmu.x, tmu.y - 2), cv::Point(tmu.x, tmu.y + 2), cv::Scalar(0, 255, 0), 2 );
	cv::line( frame, cv::Point(tru1.x - 2, tru1.y), cv::Point(tru1.x + 2, tru1.y), cv::Scalar(0, 255, 0), 2 );
	cv::line( frame, cv::Point(tru1.x, tru1.y - 2), cv::Point(tru1.x, tru1.y + 2), cv::Scalar(0, 255, 0), 2 );
	cv::line( frame, cv::Point(tru2.x - 2, tru2.y), cv::Point(tru2.x + 2, tru2.y), cv::Scalar(0, 255, 0), 2 );
	cv::line( frame, cv::Point(tru2.x, tru2.y - 2), cv::Point(tru2.x, tru2.y + 2), cv::Scalar(0, 255, 0), 2 );
	
	
	
	
	
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
