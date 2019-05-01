/************************************************************************/
/* Author: Tobias Bungard                                               */
/* Team: noOS                                                           */
/* Created: 01.01.18                                                    */
/************************************************************************/

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"

class Area
{
	public:
		Area(cv::Point pixel);

		cv::Point getStart();
		cv::Point getEnd();

		int getWidth();
		int getHeight();
		int getPixelCount();

		void addPixel(cv::Point pixel);
		void draw(cv::Mat* image, cv::Scalar color1, cv::Scalar color2);

		bool isNear(cv::Point pixel, int range);

	private:
		cv::Point start, end;
		int pixelCount;
};
