#ifndef camera_h
#define camera_h



#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include "raspicam/raspicam_cv.h"
#include "area.hpp"
#define PI 3.14159265
#define WIDTH 320
#define HEIGHT 240



void *cameraTask(void *arguments);
extern int frameBallReady;
extern int frameGoalReady;
extern int TopBorder;
extern cv::Mat hsv;
extern cv::Point objBall;
extern cv::Point objGoal;

#define tml_par ( WIDTH / 2, HEIGHT - 53 )
#define tll1_par ( WIDTH / 2 - WIDTH / 5, HEIGHT - 53 )
#define tll2_par ( WIDTH / 2 - (2*WIDTH / 5), HEIGHT - 53 )
#define trl1_par ( WIDTH / 2 + WIDTH / 5, HEIGHT - 53 )
#define trl2_par ( WIDTH / 2 + (2*WIDTH / 5), HEIGHT - 53 )

#define tmu_par ( WIDTH / 2, HEIGHT - 103 )
#define tlu1_par ( WIDTH / 2 - WIDTH / 5, HEIGHT - 103 )
#define tlu2_par ( WIDTH / 2 - (2*WIDTH / 5), HEIGHT - 103 )
#define tru1_par ( WIDTH / 2 + WIDTH / 5, HEIGHT - 103 )
#define  tru2_par ( WIDTH / 2 + (2*WIDTH / 5), HEIGHT - 103 )

#endif
