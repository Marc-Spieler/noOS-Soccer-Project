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
#endif
