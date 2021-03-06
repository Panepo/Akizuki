#pragma once
#ifndef ZUKI_RULER_H
#define ZUKI_RULER_H

#include <librealsense2/rs.hpp>
#include <opencv2\opencv.hpp>

#include <vector>

#include "funcStream.h"
#include "funcGeometry3D.h"

#define rulerTrans		0.4
#define rulerTransO		(1 - rulerTrans)
#define rulerPointSize	10
#define rulerPointFill	2
#define rulerLineSize1	1
#define rulerLineSize2	2
#define rulerLineColor	cv::Scalar(0, 255,255)		// yellow
#define rulerPointColor	cv::Scalar(0, 255,0)		// green

#define rulerTextFontA		cv::FONT_HERSHEY_DUPLEX
#define rulerTextFontB		cv::FONT_HERSHEY_SIMPLEX
#define rulerTextColorFA	cv::Scalar(0, 0, 0)			// black
#define rulerTextColorFB	cv::Scalar(0, 255,255)		// yellow

#define sectionHeight	720
#define sectionPreSize	(sectionHeight / 10)
#define sectionColor	cv::Scalar(0, 255,255)		// yellow
#define sectionBGColor	cv::Scalar(0, 0, 0)			// black
#define sectionMapSize	3
#define sectionMapColor cv::Scalar(0, 255,0)		// green

typedef enum rulerState
{
	RULERSTATE_WAIT,
	RULERSTATE_PAINT,
	RULERSTATE_LINE,
} rulerState;

class configRuler
{
public:
	rulerState state = RULERSTATE_WAIT;
	cv::Point pixelMouse = cv::Point(0, 0);
	cv::Point pixelRuler[2] = { cv::Point(0, 0), cv::Point(0, 0) };
	std::string infoText = "";
};

class zukiRuler
{
public:
	void rulerMain(cv::Mat & matOutput, rs2::depth_frame & depth, rs2_intrinsics & intrinsics, configZoomer & configZoomer);
	void rulerMouseHandler(int event, int x, int y, int flags, configZoomer & configZoomer);
	void rulerKeyboardHandler(stream & stream);

	configRuler config;
private:
	void rulerPainter(cv::Mat & matOutput);
	void rulerPointer(cv::Mat & matOutput, const rs2::depth_frame & depth, const rs2_intrinsics & intrin, configZoomer & configZoomer);
	void rulerDrawer(cv::Mat & matOutput, const rs2::depth_frame & depth, configZoomer & configZoomer);
};

#endif
