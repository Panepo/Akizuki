#include "stdafx.h"
#include "zukiMeasurer.h"

// =================================================================================
// Plugin main process
// =================================================================================

void zukiMeasurer::measurerMain(cv::Mat & matOutput, rs2::depth_frame & depth, rs2_intrinsics & intrinsics, configZoomer & configZoomer)
{
	switch (config.state)
	{
	case MEASURER_WAIT:
		break;
	case MEASURER_PAINT:
		measurerPaint(matOutput);
		break;
	case MEASURER_RECT:
		measurerRect(matOutput, depth, intrinsics, configZoomer);
		break;
	default:
		break;
	}
}

// =================================================================================
// Plugin events
// =================================================================================

void zukiMeasurer::measurerMouseHandler(int event, int x, int y, int flags, configZoomer & configZoomer)
{
	int value;

	switch (event)
	{
	case CV_EVENT_MOUSEMOVE:
		config.pixelMouse.x = x;
		config.pixelMouse.y = y;
		if (config.state == MEASURER_PAINT)
		{
			config.pixelMeasurer[1].x = x;
			config.pixelMeasurer[1].y = y;
		}
		break;
	case CV_EVENT_LBUTTONDOWN:
		config.state = MEASURER_PAINT;
		config.pixelMeasurer[0].x = x;
		config.pixelMeasurer[0].y = y;
		config.pixelMeasurer[1].x = x;
		config.pixelMeasurer[1].y = y;
		break;
	case CV_EVENT_LBUTTONUP:
		config.state = MEASURER_RECT;
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

void zukiMeasurer::measurerKeyboardHandler(stream & stream)
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

void zukiMeasurer::measurerPaint(cv::Mat & matOutput)
{
	cv::Mat overlay = matOutput.clone();
	cv::rectangle(overlay, config.pixelMeasurer[0], config.pixelMeasurer[1], measRectColor1, measRectSize1);
	cv::addWeighted(overlay, measTrans, matOutput, measTransO, 0, matOutput);
}

void zukiMeasurer::measurerRect(cv::Mat & matOutput, const rs2::depth_frame & depth, rs2_intrinsics & intrinsics, configZoomer & configZoomer)
{
	cv::Mat overlay = matOutput.clone();

	cv::Rect roi = cv::Rect(config.pixelMeasurer[0], config.pixelMeasurer[1]);
	cv::Mat inputRoi = overlay(roi);
	cv::Mat inputRoiEdge;
	funcOpenCV::cannyBlur(inputRoi, inputRoiEdge, 50, 150);
	//funcOpenCV::cannySharp(inputRoi, inputRoiEdge, 50, 150);

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(inputRoiEdge, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
	std::sort(contours.begin(), contours.end(), funcOpenCV::contourSorter);

	if ((int)contours.size() > 0)
	{
		std::vector<cv::Point> approx;
		for (int i = 0; i < (int)contours.size(); i += 1)
		{
			cv::approxPolyDP(contours[i], approx, cv::arcLength(contours[i], true) * 0.01, true);

			if ((int)approx.size() >= measApproxMin && (int)approx.size() <= measApproxMax)
			{
				cv::drawContours(inputRoi, contours, i, measContColor, measContSize);

				double area = funcGeometry3D::calcArea3D(contours[i], &inputRoi, &depth, &intrinsics,
					config.pixelMeasurer[0], measLineColor, measLineSize, configZoomer);

				std::ostringstream strs;
				strs << area;
				config.infoText = strs.str() + "cm2";

				break;
			}
		}

		inputRoi.copyTo(overlay(roi));

		cv::Point textCoord;
		textCoord.x = config.pixelMeasurer[1].x + 10;
		textCoord.y = config.pixelMeasurer[1].y + 10;

		cv::putText(matOutput, config.infoText, textCoord, measFontA, 1, measColorFA, 1, cv::LINE_AA);
		cv::putText(matOutput, config.infoText, textCoord, measFontB, 1, measColorFB, 1, cv::LINE_AA);
	}

	cv::rectangle(overlay, config.pixelMeasurer[0], config.pixelMeasurer[1], measRectColor2, measRectSize2);
	cv::addWeighted(overlay, measTrans, matOutput, measTransO, 0, matOutput);
}
