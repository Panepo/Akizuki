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
	std::string infoText = "";
};

class zukiPluginTemplate
{
public:
	void pluginMain(cv::Mat & matOutput, rs2::depth_frame & depth, rs2_intrinsics & intrinsics, configZoomer & configZoomer);
  
  void pluginMouseHandler(int event, int x, int y, int flags, configZoomer & configZoomer);
	void pluginKeyboardHandler(stream & stream);

	configPlugin config;
private:
};

#endif
