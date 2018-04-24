#include "stdafx.h"
#include "zukiStreamer.h"

// =================================================================================
// Plugin main process
// =================================================================================

void zukiStreamer::streamerMain(cv::Mat & matOutput, rs2::depth_frame & depth, rs2_intrinsics & intrinsics, configZoomer & configZoomer)
{
	streamPointer(matOutput, depth, intrinsics, configZoomer);
}

// =================================================================================
// Plugin events
// =================================================================================

void zukiStreamer::streamerMouseHandler(int event, int x, int y, int flags, configZoomer & configZoomer)
{
	int value;

	switch (event)
	{
	case CV_EVENT_MOUSEMOVE:
		config.pixelMouse.x = x;
		config.pixelMouse.y = y;
		break;
	case CV_EVENT_MOUSEWHEEL:
		configZoomer.pixelZoom.x = x;
		configZoomer.pixelZoom.y = y;

		value = cv::getMouseWheelDelta(flags);
		if (value > 0 && configZoomer.scaleZoom < zoomerScaleMax)
			configZoomer.scaleZoom += (float) 0.1;
		else if (value < 0 && configZoomer.scaleZoom > zoomerScaleMin)
			configZoomer.scaleZoom -= (float) 0.1;
		break;
	default:
		break;
	}
}

void zukiStreamer::streamerKeyboardHandler(stream & stream)
{
	switch (stream)
	{
	case STREAM_COLOR:
		stream = STREAM_INFRARED;
		break;
	case STREAM_INFRARED:
		stream = STREAM_DEPTH;
		break;
	case STREAM_DEPTH:
		stream = STREAM_COLOR;
		break;
	}
}

// =================================================================================
// Plugin sub functions
// =================================================================================

void zukiStreamer::streamPointer(cv::Mat & matOutput, rs2::depth_frame & depth, rs2_intrinsics & intrinsics, configZoomer & configZoomer)
{
	cv::Point pixel;
	funcStream::streamZoomPixelTrans(config.pixelMouse, pixel, configZoomer);
	
	float pixelArray[2] = { (float)pixel.x, (float)pixel.y };
	float point[3] = { 0, 0, 0 };

	float depthPixel = depth.get_distance((int)pixelArray[0], (int)pixelArray[1]);
	rs2_deproject_pixel_to_point(point, &intrinsics, pixelArray, depthPixel * 1000);

	cv::circle(matOutput, config.pixelMouse, pointerSize, pointerColor, -1);
	config.infoText = "(" + std::to_string((int)point[0]) + ", " + std::to_string((int)point[1]) + ", " + std::to_string((int)point[2]) + ")";
	std::string text = std::to_string((int)point[2]) + "mm";

	cv::Point textLoc = cv::Point(config.pixelMouse.x - 100, config.pixelMouse.y + 40);

	cv::putText(matOutput, text, textLoc, pointerFontA, 1, pointerColorFA, 1, cv::LINE_AA);
	cv::putText(matOutput, text, textLoc, pointerFontB, 1, pointerColorFB, 1, cv::LINE_AA);
}




