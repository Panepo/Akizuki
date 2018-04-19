#pragma once
#ifndef ZUKI_STREAMER_H
#define ZUKI_STREAMER_H

#include <librealsense2/rs.hpp>
#include <opencv2\opencv.hpp>

#include "funcFormat.h"
#include "funcStream.h"
#include <librealsense2\rsutil.h>

#define pointerSize		5
#define pointerColor	cv::Scalar(255, 255, 255)	// white
#define pointerFontA	cv::FONT_HERSHEY_DUPLEX
#define pointerFontB	cv::FONT_HERSHEY_SIMPLEX
#define pointerColorFA	cv::Scalar(0, 0, 0)			// black
#define pointerColorFB	cv::Scalar(0, 255,255)		// yellow

typedef enum streamerState
{
	STREAMERSTATE_COLOR,
	STREAMERSTATE_INFRARED,
	STREAMERSTATE_DEPTH,
} streamerState;

class configStreamer
{
public:
	streamerState state = STREAMERSTATE_COLOR;
	cv::Point pixelMouse = cv::Point(0, 0);
	cv::Point pixelZoom = cv::Point(0, 0);
	cv::Point pixelRoiZoom = cv::Point(0, 0);
	float scaleZoom = 1;
	std::string infoText = "";
};

class zukiStreamer
{
public:
	void streamerMain(
		cv::Mat & matOutput,
		rs2::pipeline & pipeline, 
		rs2::spatial_filter & filterSpat, 
		rs2::temporal_filter & filterTemp,
		rs2_intrinsics & intrinsics
	);
	void streamerMouseHandler(int event, int x, int y, int flags);
	void streamerKeyboardHandler();

	configStreamer config;
private:
	void streamerColor(
		cv::Mat & matOutput,
		rs2::frameset & alignedFrame,
		rs2::depth_frame & depth,
		rs2_intrinsics & intrinsics
	);
	void streamerInfrared(
		cv::Mat & matOutput,
		rs2::frameset & alignedFrame,
		rs2::depth_frame & depth,
		rs2_intrinsics & intrinsics
	);
	void streamerDepth(
		cv::Mat & matOutput,
		rs2::depth_frame & depth,
		rs2_intrinsics & intrinsics
	);


	void streamPointer(cv::Mat & matOutput, rs2::depth_frame & depth, rs2_intrinsics & intrinsics);
};

#endif
