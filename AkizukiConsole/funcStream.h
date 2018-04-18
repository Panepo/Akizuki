#pragma once
#ifndef FUNCSTREAM_H
#define FUNCSTREAM_H

#include <opencv2/opencv.hpp>

#include "funcOpenCV.h"

#define zoomerScaleMin	0.1
#define zoomerScaleMax	1
#define zoomerLineSize	5
#define zoomerLineColor cv::Scalar(0, 255,255)		// yellow
#define zoomerMapSize	3
#define zoomerMapColor  cv::Scalar(0, 255,0)		// green

namespace funcStream
{
	void streamZoomer(cv::Mat & input, cv::Mat & output, cv::Point & pixelZoom, cv::Point & pixelRoiZoom, float & scaleZoom);
}


#endif