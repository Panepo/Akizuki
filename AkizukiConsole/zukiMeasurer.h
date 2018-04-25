#pragma once
#ifndef ZUKI_MEASURER_H
#define ZUKI_MEASURER_H

#include <librealsense2/rs.hpp>
#include <opencv2\opencv.hpp>

#include "funcStream.h"
#include "funcGeometry3D.h"
#include "funcOpenCV.h"

#define measTrans		0.4
#define measTransO		(1 - measTrans)
#define measRectColor1	cv::Scalar(0, 255,255)		// yellow
#define measRectSize1	1
#define measRectColor2	cv::Scalar(0, 255,0)		// green
#define measRectSize2	2
#define measContColor	cv::Scalar(255,255,0)		// teal
#define measContSize	2
#define measApproxMin	4
#define measApproxMax	10
#define measLineColor	cv::Scalar(0, 255,255)		// yellow
#define measLineSize	2

#define measFontA		cv::FONT_HERSHEY_DUPLEX
#define measFontB		cv::FONT_HERSHEY_SIMPLEX
#define measColorFA		cv::Scalar(0, 0, 0)			// black
#define measColorFB		cv::Scalar(0, 255,255)		// yellow

typedef enum measurerState
{
	MEASURER_WAIT,
	MEASURER_PAINT,
	MEASURER_RECT,
} pluginState;

class configMeasurer
{
public:
	measurerState state = MEASURER_WAIT;
	cv::Point pixelMouse = cv::Point(0, 0);
	cv::Point pixelMeasurer[2] = { cv::Point(0, 0), cv::Point(0, 0) };
	std::string infoText = "";
};

class zukiMeasurer
{
public:
	void measurerMain(cv::Mat & matOutput, rs2::depth_frame & depth, rs2_intrinsics & intrinsics, configZoomer & configZoomer);
  
	void measurerMouseHandler(int event, int x, int y, int flags, configZoomer & configZoomer);
	void measurerKeyboardHandler(stream & stream);

	configMeasurer config;
private:
	void measurerPaint(cv::Mat & matOutput);
	void measurerRect(cv::Mat & matOutput, const rs2::depth_frame & depth, rs2_intrinsics & intrinsics, configZoomer & configZoomer);
};

#endif
