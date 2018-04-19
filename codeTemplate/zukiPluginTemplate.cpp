#include "stdafx.h"
#include "zukiPluginTemplate.h"

// =================================================================================
// Plugin main process
// =================================================================================

void zukiPluginTemplate::PluginMain(cv::Mat & matOutput, rs2::pipeline & pipeline, rs2::spatial_filter & filterSpat, rs2::temporal_filter & filterTemp, rs2_intrinsics & intrinsics)
{
	rs2::align alignTo(RS2_STREAM_COLOR);
	rs2::frameset data = pipeline.wait_for_frames();
	rs2::frameset alignedFrame = alignTo.process(data);

	cv::Mat matColor = funcFormat::frame2Mat(alignedFrame.get_color_frame());

	//cv::Mat matInfrared = funcFormat::frame2Mat(alignedFrame.get_infrared_frame());
	cv::Mat matInfrared = funcFormat::frame2Mat(data.get_infrared_frame());
	
	rs2::colorizer colorize;
	colorize.set_option(RS2_OPTION_COLOR_SCHEME, 0);
	rs2::depth_frame depth = alignedFrame.get_depth_frame();
	depth = filterSpat.process(depth);
	depth = filterTemp.process(depth);
	rs2::frame depthColor = colorize(depth);
	cv::Mat matDepth = funcFormat::frame2Mat(depthColor);

	matOutput = matColor.clone();
}

// =================================================================================
// Plugin events
// =================================================================================

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

// =================================================================================
// Plugin sub functions
// =================================================================================

