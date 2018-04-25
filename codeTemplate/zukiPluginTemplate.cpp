#include "stdafx.h"
#include "zukiPluginTemplate.h"

// =================================================================================
// Plugin main process
// =================================================================================

void zukiPluginTemplate::PluginMain(cv::Mat & matOutput, rs2::depth_frame & depth, rs2_intrinsics & intrinsics, configZoomer & configZoomer)
{
	function(matOutput, depth, intrinsics, configZoomer);
}

// =================================================================================
// Plugin events
// =================================================================================

void zukiPluginTemplate::pluginMouseHandler(int event, int x, int y, int flags, configZoomer & configZoomer)
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

void zukiPluginTemplate::pluginKeyboardHandler(stream & stream)
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

