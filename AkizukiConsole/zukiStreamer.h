#pragma once
#ifndef ZUKI_STREAMER_H
#define ZUKI_STREAMER_H

#include <librealsense2/rs.hpp>
#include <opencv2\opencv.hpp>

#include "funcStream.h"
#include <librealsense2\rsutil.h>

#define pointerSize		5
#define pointerColor	cv::Scalar(255, 255, 255)	// white
#define pointerFontA	cv::FONT_HERSHEY_DUPLEX
#define pointerFontB	cv::FONT_HERSHEY_SIMPLEX
#define pointerColorFA	cv::Scalar(0, 0, 0)			// black
#define pointerColorFB	cv::Scalar(0, 255,255)		// yellow

class configStreamer
{
public:
	cv::Point pixelMouse = cv::Point(0, 0);
	std::string infoText = "";
};

class zukiStreamer
{
public:
	void streamerMain(cv::Mat & matOutput, rs2::depth_frame & depth, rs2_intrinsics & intrinsics, configZoomer & configZoomer);
	void streamerMouseHandler(int event, int x, int y, int flags, configZoomer & configZoomer);
	void streamerKeyboardHandler(stream & stream);

	configStreamer config;
private:
	void streamPointer(cv::Mat & matOutput, rs2::depth_frame & depth, rs2_intrinsics & intrinsics, configZoomer & configZoomer);
};

#endif
