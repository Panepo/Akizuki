#pragma once
#ifndef ZUKI_RECOGNIZER_H
#define ZUKI_RECOGNIZER_H

#include <librealsense2/rs.hpp>
#include <opencv2\opencv.hpp>

typedef enum pluginState
{
	STREAMERSTATE_COLOR,
	STREAMERSTATE_INFRARED,
	STREAMERSTATE_DEPTH,
} pluginState;

class configPlugin
{
public:
	pluginState state = STREAMERSTATE_COLOR;
	cv::Point pixelMouse = cv::Point(0, 0);
	cv::Point pixelZoom = cv::Point(0, 0);
	cv::Point pixelRoiZoom = cv::Point(0, 0);
	float scaleZoom = 1;
	std::string infoText = "";
};

class zukiPluginTemplate
{
public:
	void PluginMain(
		cv::Mat & matOutput,
		rs2::pipeline & pipeline, 
		rs2::spatial_filter & filterSpat, 
		rs2::temporal_filter & filterTemp,
		rs2_intrinsics & intrinsics
	);
  
  void streamerMouseHandler(int event, int x, int y, int flags);
	void streamerKeyboardHandler();

	configPlugin config;
private:
};

#endif
