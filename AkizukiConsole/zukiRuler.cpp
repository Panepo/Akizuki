#include "stdafx.h"
#include "zukiRuler.h"

// =================================================================================
// Plugin main process
// =================================================================================

void zukiRuler::rulerMain(cv::Mat & matOutput, rs2::depth_frame & depth, rs2_intrinsics & intrinsics, configZoomer & configZoomer)
{
	switch (config.state)
	{
	case RULERSTATE_WAIT:
		break;
	case RULERSTATE_PAINT:
		rulerPainter(matOutput);
		break;
	case RULERSTATE_LINE:
		rulerPointer(matOutput, depth, intrinsics, configZoomer);
		rulerDrawer(matOutput, depth, configZoomer);
		break;
	default:
		break;
	}
}

// =================================================================================
// Plugin events
// =================================================================================

void zukiRuler::rulerMouseHandler(int event, int x, int y, int flags, configZoomer & configZoomer)
{
	int value;

	switch (event)
	{
	case CV_EVENT_LBUTTONDOWN:
		config.state = RULERSTATE_PAINT;
		config.pixelRuler[0].x = x;
		config.pixelRuler[0].y = y;
		config.pixelRuler[1].x = x;
		config.pixelRuler[1].y = y;
		config.infoText = "";
		break;
	case CV_EVENT_MOUSEMOVE:
		config.pixelMouse.x = x;
		config.pixelMouse.y = y;
		if (config.state == RULERSTATE_PAINT)
		{
			config.pixelRuler[1].x = x;
			config.pixelRuler[1].y = y;
		}
		break;
	case CV_EVENT_LBUTTONUP:
		config.state = RULERSTATE_LINE;
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

void zukiRuler::rulerKeyboardHandler(stream & stream)
{
	//config.state = RULERSTATE_WAIT;
	//config.infoText = "";
	
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

void zukiRuler::rulerPainter(cv::Mat & matOutput)
{
	cv::Mat overlay = matOutput.clone();
	cv::line(overlay, config.pixelRuler[0], config.pixelRuler[1], rulerLineColor, rulerLineSize1);
	cv::addWeighted(overlay, rulerTrans, matOutput, rulerTransO, 0, matOutput);
}

void zukiRuler::rulerPointer(cv::Mat & matOutput, const rs2::depth_frame & depth, const rs2_intrinsics & intrin, configZoomer & configZoomer)
{
	cv::Mat overlay = matOutput.clone();
	
	cv::Point pixelA, pixelB;
	funcStream::streamZoomPixelTrans(config.pixelRuler[0], pixelA, configZoomer);
	funcStream::streamZoomPixelTrans(config.pixelRuler[1], pixelB, configZoomer);
	float posA[2] = { (float)pixelA.x, (float)pixelA.y };
	float posB[2] = { (float)pixelB.x, (float)pixelB.y };
	
	cv::circle(overlay, config.pixelRuler[0], rulerPointSize, rulerPointColor, rulerPointFill);
	cv::circle(overlay, config.pixelRuler[1], rulerPointSize, rulerPointColor, rulerPointFill);
	cv::line(overlay, config.pixelRuler[0], config.pixelRuler[1], rulerPointColor, rulerLineSize2);

	cv::addWeighted(overlay, rulerTrans, matOutput, rulerTransO, 0, matOutput);

	float dist = funcGeometry3D::calcDist3D(posA, posB, &depth, &intrin);

	std::ostringstream strs;
	strs << dist;
	config.infoText = strs.str() + "cm";

	cv::Point textCoord;
	textCoord.x = (config.pixelRuler[0].x + config.pixelRuler[1].x) / 2;
	textCoord.y = (config.pixelRuler[0].y + config.pixelRuler[1].y) / 2;

	cv::putText(matOutput, config.infoText, textCoord, rulerTextFontA, 1, rulerTextColorFA, 1, cv::LINE_AA);
	cv::putText(matOutput, config.infoText, textCoord, rulerTextFontB, 1, rulerTextColorFB, 1, cv::LINE_AA);
}

void zukiRuler::rulerDrawer(cv::Mat & matOutput, const rs2::depth_frame & depth, configZoomer & configZoomer)
{
	cv::Point pixelA, pixelB;
	funcStream::streamZoomPixelTrans(config.pixelRuler[0], pixelA, configZoomer);
	funcStream::streamZoomPixelTrans(config.pixelRuler[1], pixelB, configZoomer);
	float posA[2] = { (float)pixelA.x, (float)pixelA.y };
	float posB[2] = { (float)pixelB.x, (float)pixelB.y };
	
	float xdiff = abs(posA[0] - posB[0]);
	float ydiff = abs(posA[1] - posB[1]);
	int posX = 0, posY = 0;
	float dist = 0, parm = 1;
	float directX = 1, directY = 1;
	std::vector<float> output;

	// get depth data of the route
	if (posA[0] - posB[0] < 0)
		directX = -1;

	if (posA[1] - posB[1] < 0)
		directY = -1;

	if (xdiff < ydiff)
	{
		for (int i = 0; i < ydiff; i += 1)
			//for (int i : boost::irange<int>(0, (int)ydiff))
		{
			posX = (int)floor(posB[0] + directX * (float)i * xdiff / ydiff);
			posY = (int)floor(posB[1] + directY * (float)i);
			dist = depth.get_distance(posX, posY);
			output.push_back(dist);
		}
	}
	else
	{
		for (int i = 0; i < xdiff; i += 1)
			//for (int i : boost::irange<int>(0, (int)xdiff))
		{
			posX = (int)floor(posB[0] + directX * (float)i);
			posY = (int)floor(posB[1] + directY * (float)i * ydiff / xdiff);
			dist = depth.get_distance(posX, posY);
			output.push_back(dist);
		}
	}

	if (output.size() > 0)
	{
		// get min and max value to calculate drawing parameter
		auto outMinMax = std::minmax_element(output.begin(), output.end());

		if ((float)(*outMinMax.second - *outMinMax.first) < 0.2)
			parm = sectionHeight * 5;
		else
			parm = sectionHeight / (*outMinMax.second - *outMinMax.first);

		// draw sectional drawing
		cv::Mat minimap = cv::Mat(sectionHeight, (int)output.size(), CV_8UC3, sectionColor);

		//#pragma omp parallel for
		for (int i = 0; i < (int)output.size(); i += 1)
			//for (int i : boost::irange<int>(0, (int)output.size()))
		{
			cv::line(minimap, cv::Point(i, 0),
				cv::Point(i, (int)((output[i] - *outMinMax.first) * parm)), sectionBGColor, 1);
		}

		// flip image if inverse
		if (posA[0] < posB[0])
		{
			cv::Mat minimapFlip;
			cv::flip(minimap, minimapFlip, 1);
			minimap = minimapFlip.clone();
		}

		// generate preserve border
		cv::copyMakeBorder(minimap, minimap, sectionPreSize, 0, 0, 0, cv::BORDER_CONSTANT, sectionBGColor);
		cv::copyMakeBorder(minimap, minimap, 0, sectionPreSize, 0, 0, cv::BORDER_CONSTANT, sectionColor);

		// generate mini-map
		cv::Size size = matOutput.size();
		cv::Size sizeMap = cv::Size((int)(size.width / 8), (int)(size.height / 8));
		funcStream::streamMapperRD(matOutput, minimap, matOutput, sizeMap, sectionMapSize, sectionMapColor);
	}
}
