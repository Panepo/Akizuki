#pragma once
#ifndef APP_H
#define APP_H

#include <librealsense2\rs.hpp>
#include <opencv2\opencv.hpp>

#include <chrono>

#include "configCamera.h"
#include "funcStream.h"

#include "zukiStreamer.h"
#include "zukiRuler.h"
#include "zukiScanner.h"
#include "zukiMeasurer.h"

typedef enum appState
{
	APPSTATE_EXIT,
	APPSTATE_STREAMER,
	APPSTATE_RULER,
	APPSTATE_SCANNER,
	APPSTATE_MEASURER,
} appState;

class app
{
public:
	// application main process
	app(std::string title);
	void cameraInitial();
	void cameraProcess();

	// application config settings
	void setResolution(int stream, int width, int height, int fps);
	void setVisualPreset(std::string preset);

	appState state = APPSTATE_EXIT;
	cv::Mat matOutput;
	//rs2::depth_frame depth;

private:
	// realsense private parameters
	rs2::pipeline pipeline;
	rs2_intrinsics intrinsics;
	rs2::decimation_filter filterDec;
	rs2::spatial_filter filterSpat;
	rs2::temporal_filter filterTemp;
	stream stream;
	configZoomer configZoomer;

	// application private parameters
	std::string windowTitle = "Akizuki";
	std::string visualPreset = "High Density";
	int ColorWidth = 640;
	int ColorHeight = 480;
	int ColorFPS = 30;
	int DepthWidth = 640;
	int DepthHeight = 480;
	int DepthFPS = 30;

	// application events
	void eventKeyboard();
	static void eventMouseS(int event, int x, int y, int flags, void* userdata);
	void eventMouse(int event, int x, int y, int flags);
	void eventSaveImage();

	// declare application plugins
	zukiStreamer streamer;
	zukiRuler ruler;
	zukiScanner scanner;
	zukiMeasurer measurer;
};

#endif