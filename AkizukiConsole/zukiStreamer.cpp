#include "stdafx.h"
#include "zukiStreamer.h"

void zukiStreamer::streamerMain(cv::Mat & matOutput, rs2::pipeline & pipeline, rs2::spatial_filter & filterSpat, rs2::temporal_filter & filterTemp, rs2_intrinsics & intrinsics)
{
	rs2::align alignTo(RS2_STREAM_COLOR);
	rs2::frameset data = pipeline.wait_for_frames();
	rs2::frameset alignedFrame = alignTo.process(data);

	rs2::depth_frame depth = alignedFrame.get_depth_frame();
	depth = filterSpat.process(depth);
	depth = filterTemp.process(depth);

	switch (config.state)
	{
	case STREAMERSTATE_COLOR:
		streamerColor(matOutput, alignedFrame, depth, intrinsics);
		break;
	case STREAMERSTATE_INFRARED:
		// =========================================================================
		// align infrared to color is not possible, so call data not call alignedFrame here
		// =========================================================================
		streamerInfrared(matOutput, data, depth, intrinsics);
		break;
	case STREAMERSTATE_DEPTH:
		streamerDepth(matOutput, depth, intrinsics);
		break;
	default:
		break;
	}
}

void zukiStreamer::streamerMouseHandler(int event, int x, int y, int flags)
{
	int value;

	switch (event)
	{
	case CV_EVENT_MOUSEMOVE:
		config.pixelMouse.x = x;
		config.pixelMouse.y = y;
		break;
	case CV_EVENT_MOUSEWHEEL:
		config.pixelZoom.x = x;
		config.pixelZoom.y = y;

		value = cv::getMouseWheelDelta(flags);
		if (value > 0 && config.scaleZoom < zoomerScaleMax)
			config.scaleZoom += (float) 0.1;
		else if (value < 0 && config.scaleZoom > zoomerScaleMin)
			config.scaleZoom -= (float) 0.1;
		break;
	default:
		break;
	}
}

void zukiStreamer::streamerKeyboardHandler()
{
	switch (config.state)
	{
	case STREAMERSTATE_COLOR:
		config.state = STREAMERSTATE_INFRARED;
		break;
	case STREAMERSTATE_INFRARED:
		config.state = STREAMERSTATE_DEPTH;
		break;
	case STREAMERSTATE_DEPTH:
		config.state = STREAMERSTATE_COLOR;
		break;
	}
}

void zukiStreamer::streamerColor(cv::Mat & matOutput, rs2::frameset & alignedFrame, rs2::depth_frame & depth, rs2_intrinsics & intrinsics)
{
	cv::Mat matColor = funcFormat::frame2Mat(alignedFrame.get_color_frame());
	cv::Mat matColorOrig = matColor.clone();
	
	funcStream::streamZoomer(matColorOrig, matColor, config.pixelZoom, config.pixelRoiZoom, config.scaleZoom);
	streamPointer(matColor, depth, intrinsics);
	matOutput = matColor.clone();
}

void zukiStreamer::streamerInfrared(cv::Mat & matOutput, rs2::frameset & alignedFrame, rs2::depth_frame & depth, rs2_intrinsics & intrinsics)
{
	cv::Mat matInf = funcFormat::frame2Mat(alignedFrame.get_infrared_frame());
	cv::Mat matInfOrig = matInf.clone();

	funcStream::streamZoomer(matInfOrig, matInf, config.pixelZoom, config.pixelRoiZoom, config.scaleZoom);
	streamPointer(matInf, depth, intrinsics);
	matOutput = matInf.clone();
}

void zukiStreamer::streamerDepth(cv::Mat & matOutput, rs2::depth_frame & depth, rs2_intrinsics & intrinsics)
{
	rs2::colorizer colorize;
	colorize.set_option(RS2_OPTION_COLOR_SCHEME, 0);
	rs2::frame depthColor = colorize(depth);
	cv::Mat matDepth = funcFormat::frame2Mat(depthColor);
	cv::Mat matDepthOrig = matDepth.clone();

	funcStream::streamZoomer(matDepthOrig, matDepth, config.pixelZoom, config.pixelRoiZoom, config.scaleZoom);
	streamPointer(matDepth, depth, intrinsics);
	matOutput = matDepth.clone();
}

void zukiStreamer::streamPointer(cv::Mat & matOutput, rs2::depth_frame & depth, rs2_intrinsics & intrinsics)
{
	float pixel[2] = { 0, 0 };
	float point[3] = { 0, 0, 0 };

	if (config.scaleZoom == 1)
	{
		pixel[0] = (float)config.pixelMouse.x;
		pixel[1] = (float)config.pixelMouse.y;
	}
	else
	{
		pixel[0] = (float)(config.pixelMouse.x * config.scaleZoom + config.pixelRoiZoom.x);
		pixel[1] = (float)(config.pixelMouse.y * config.scaleZoom + config.pixelRoiZoom.y);
	}

	float depthPixel = depth.get_distance((int)pixel[0], (int)pixel[1]);
	rs2_deproject_pixel_to_point(point, &intrinsics, pixel, depthPixel * 1000);

	cv::circle(matOutput, config.pixelMouse, pointerSize, pointerColor, -1);
	config.infoText = "(" + std::to_string((int)point[0]) + ", " + std::to_string((int)point[1]) + ", " + std::to_string((int)point[2]) + ")";
	std::string text = std::to_string((int)point[2]) + "mm";

	cv::Point textLoc = cv::Point(config.pixelMouse.x - 100, config.pixelMouse.y + 40);

	cv::putText(matOutput, text, textLoc, pointerFontA, 1, pointerColorFA, 1, cv::LINE_AA);
	cv::putText(matOutput, text, textLoc, pointerFontB, 1, pointerColorFB, 1, cv::LINE_AA);
}




