#include "stdafx.h"
#include "zukiStreamer.h"

void zukiStreamer::streamerMain(cv::Mat & matOutput, configStreamer & configStreamer, rs2::pipeline & pipeline, rs2::spatial_filter & filterSpat, rs2::temporal_filter & filterTemp, rs2_intrinsics & intrinsics)
{
	rs2::align alignTo(RS2_STREAM_COLOR);
	rs2::frameset data = pipeline.wait_for_frames();
	rs2::frameset alignedFrame = alignTo.process(data);

	rs2::depth_frame depth = alignedFrame.get_depth_frame();
	depth = filterSpat.process(depth);
	depth = filterTemp.process(depth);

	switch (configStreamer.state)
	{
	case STREAMERSTATE_COLOR:
		streamerColor(matOutput, configStreamer, alignedFrame, depth, intrinsics);
		break;
	default:
		break;
	}

	//cv::Mat matColor = funcFormat::frame2Mat(alignedFrame.get_color_frame());
	//cv::Mat matInfrared = funcFormat::frame2Mat(data.get_infrared_frame());
	//
	//rs2::colorizer colorize;
	//colorize.set_option(RS2_OPTION_COLOR_SCHEME, 0);
	//rs2::frame depthColor = colorize(depth);
	//cv::Mat matDepth = funcFormat::frame2Mat(depthColor);

	//matOutput = matColor.clone();
}

void zukiStreamer::streamerMouseHandler(configStreamer & configStreamer, int event, int x, int y, int flags)
{
	int value;

	switch (event)
	{
	case CV_EVENT_MOUSEMOVE:
		configStreamer.pixelMouse.x = x;
		configStreamer.pixelMouse.y = y;
		break;
	case CV_EVENT_MOUSEWHEEL:
		configStreamer.pixelZoom.x = x;
		configStreamer.pixelZoom.y = y;

		value = cv::getMouseWheelDelta(flags);
		if (value > 0 && configStreamer.scaleZoom < zoomerScaleMax)
			configStreamer.scaleZoom += (float) 0.1;
		else if (value < 0 && configStreamer.scaleZoom > zoomerScaleMin)
			configStreamer.scaleZoom -= (float) 0.1;
		break;
	default:
		break;
	}
}

void zukiStreamer::streamerColor(cv::Mat & matOutput, configStreamer & configStreamer, rs2::frameset & alignedFrame, rs2::depth_frame & depth, rs2_intrinsics & intrinsics)
{
	cv::Mat matColor = funcFormat::frame2Mat(alignedFrame.get_color_frame());
	cv::Mat matColorOrig = matColor.clone();
	
	funcOpenCV::genZoomFrame(matColorOrig, matColor, configStreamer.pixelZoom, configStreamer.pixelRoiZoom, configStreamer.scaleZoom);
	streamPointer(matColor, depth, intrinsics, configStreamer);
	matOutput = matColor.clone();
}

void zukiStreamer::streamPointer(cv::Mat & matOutput, rs2::depth_frame & depth, rs2_intrinsics & intrinsics, configStreamer & configStreamer)
{
	float pixel[2] = { 0, 0 };
	float point[3] = { 0, 0, 0 };

	if (configStreamer.scaleZoom == 1)
	{
		pixel[0] = (float)configStreamer.pixelMouse.x;
		pixel[1] = (float)configStreamer.pixelMouse.y;
	}
	else
	{
		pixel[0] = (float)(configStreamer.pixelMouse.x * configStreamer.scaleZoom + configStreamer.pixelRoiZoom.x);
		pixel[1] = (float)(configStreamer.pixelMouse.y * configStreamer.scaleZoom + configStreamer.pixelRoiZoom.y);
	}

	float depthPixel = depth.get_distance((int)pixel[0], (int)pixel[1]);
	rs2_deproject_pixel_to_point(point, &intrinsics, pixel, depthPixel * 1000);

	cv::circle(matOutput, configStreamer.pixelMouse, pointerSize, pointerColor, -1);
	//std::string text = std::to_string((int)point[0]) + " " + std::to_string((int)point[1])
	//	+ " " + std::to_string((int)point[2]);
	std::string text = std::to_string((int)point[2]) + "mm";

	cv::Point textLoc = cv::Point(configStreamer.pixelMouse.x - 100, configStreamer.pixelMouse.y + 40);

	cv::putText(matOutput, text, textLoc, pointerFontA, 1, pointerColorFA, 1, cv::LINE_AA);
	cv::putText(matOutput, text, textLoc, pointerFontB, 1, pointerColorFB, 1, cv::LINE_AA);
}




