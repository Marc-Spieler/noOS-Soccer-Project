#ifndef camera_h
#define camera_h



#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include "raspicam/raspicam_cv.h"
#include "area.hpp"
#define PI 3.14159265
#define WIDTH 320 //before:320
#define HEIGHT 240  //before:240



void *cameraTask(void *arguments);
extern volatile int frameBallReady;
extern volatile int frameGoalReady;
extern int TopBorder;
extern cv::Mat hsv;
extern cv::Mat frame;
extern cv::Point objBall;
extern cv::Point objGoal;
#define fac 0.5
#define facTWidth 0.9*fac
#define facTHeightLower 1*fac 
#define facTHeightUpper 1*fac
#define tml_par ( WIDTH / 2, HEIGHT - 26*facTHeightLower ) //before: 53 instead of 26
#define tll1_par ( WIDTH / 2 - WIDTH / 5*facTWidth, HEIGHT - 26*facTHeightLower )
#define tll2_par ( WIDTH / 2 - (2*WIDTH / 5*facTWidth), HEIGHT - 26*facTHeightLower )
#define trl1_par ( WIDTH / 2 + WIDTH / 5*facTWidth, HEIGHT - 26*facTHeightLower )
#define trl2_par ( WIDTH / 2 + (2*WIDTH / 5*facTWidth), HEIGHT - 26*facTHeightLower )

#define tmu_par ( WIDTH / 2, HEIGHT - 75*facTHeightUpper )//before: 103 intead of 75
#define tlu1_par ( WIDTH / 2 - WIDTH / 5*facTWidth, HEIGHT - 75*facTHeightUpper )
#define tlu2_par ( WIDTH / 2 - (2*WIDTH / 5*facTWidth), HEIGHT - 75*facTHeightUpper )
#define tru1_par ( WIDTH / 2 + WIDTH / 5*facTWidth, HEIGHT - 75*facTHeightUpper )
#define  tru2_par ( WIDTH / 2 + (2*WIDTH / 5*facTWidth), HEIGHT - 75*facTHeightUpper )

//old:
//#define tml_par ( WIDTH / 2, HEIGHT - 26 ) //before: 53 instead of 26
//#define tll1_par ( WIDTH / 2 - WIDTH / 5, HEIGHT - 26 )
//#define tll2_par ( WIDTH / 2 - (2*WIDTH / 5), HEIGHT - 26 )
//#define trl1_par ( WIDTH / 2 + WIDTH / 5, HEIGHT - 26 )
//#define trl2_par ( WIDTH / 2 + (2*WIDTH / 5), HEIGHT - 26 )

//#define tmu_par ( WIDTH / 2, HEIGHT - 75 )//before: 103 intead of 75
//#define tlu1_par ( WIDTH / 2 - WIDTH / 5, HEIGHT - 75 )
//#define tlu2_par ( WIDTH / 2 - (2*WIDTH / 5), HEIGHT - 75 )
//#define tru1_par ( WIDTH / 2 + WIDTH / 5, HEIGHT - 75 )
//#define  tru2_par ( WIDTH / 2 + (2*WIDTH / 5), HEIGHT - 75 )


#endif
