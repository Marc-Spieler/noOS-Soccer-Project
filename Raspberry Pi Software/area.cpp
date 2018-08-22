/************************************************************************/
/* Author: Tobias Bungard                                               */
/* Team: noOS                                                           */
/* Created: 01.01.18                                                    */
/************************************************************************/

#include "area.hpp"

// Set start and end to give pixexl -> size is 0
Area::Area(cv::Point pixel)
{
	start = end = pixel;
	pixelCount = 1;
}

// Return start
cv::Point Area::getStart()
{
	return start;
}

// Return end
cv::Point Area::getEnd()
{
	return end;
}

// Width is calculated from the difference between the
// x-Value of startpoint and x-Value of endpoint
int Area::getWidth()
{
	return end.x - start.x;
}

// Height is calclulated from the difference between the
// y-Value of startpoint and y-Value of endpoint
int Area::getHeight()
{
	return end.y - start.y;
}

int Area::getPixelCount()
{
	return pixelCount;
}

// If x or y-Coordinates of the pixel are outside of the area
// expand the area so the pixel is inside
void Area::addPixel(cv::Point pixel)
{
	if(pixel.x < start.x) start.x = pixel.x;
	if(pixel.x > end.x) end.x = pixel.x;
	if(pixel.y < start.y) start.y = pixel.y;
	if(pixel.y > end.y) end.y = pixel.y;
	pixelCount++;
}

// Draw the area on given image.
void Area::draw(cv::Mat* image, cv::Scalar color1, cv::Scalar color2)
{
	// Draw the outline
	cv::line(*image, start, cv::Point( end.x, start.y), color1);
	cv::line(*image, end, cv::Point( start.x, end.y), color1);
	cv::line(*image, start, cv::Point( start.x, end.y), color1);
	cv::line(*image, end, cv::Point( end.x, start.y), color1);

	// Calculate mid of area
	int midX = start.x + getWidth() / 2;
	int midY = start.y + getHeight() / 2;

	// Draw mid of area
	cv::line(*image, cv::Point(midX - 10, midY), cv::Point(midX + 10, midY), color2);
	cv::line(*image, cv::Point(midX, midY - 10), cv::Point(midX, midY + 10), color2);

}

// Test if
//	pixel.x <= maxDistance left of area
//	pixel.x <= maxDistance right of area
// 	pixel.y <= maxDistance above the area
//	pixel.y <= maxDistance under the area
bool Area::isNear(cv::Point pixel, int maxDistance)
{
	if(start.x - pixel.x < maxDistance && end.x - pixel.x > 0
       || pixel.x - end.x < maxDistance && pixel.x - start.x > 0
       || start.y - pixel.y < maxDistance && end.y - pixel.y > 0
       || pixel.y - end.y < maxDistance && pixel.y - start.y > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}


