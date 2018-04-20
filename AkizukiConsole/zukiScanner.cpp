#include "stdafx.h"
#include "zukiScanner.h"

// =================================================================================
// Plugin main process
// =================================================================================

void zukiScanner::scannerMain(cv::Mat & matOutput, rs2::pipeline & pipeline, rs2::spatial_filter & filterSpat, rs2::temporal_filter & filterTemp, rs2_intrinsics & intrinsics)
{
	rs2::align alignTo(RS2_STREAM_COLOR);
	rs2::frameset data = pipeline.wait_for_frames();
	rs2::frameset alignedFrame = alignTo.process(data);

	rs2::depth_frame depth = alignedFrame.get_depth_frame();
	depth = filterSpat.process(depth);
	depth = filterTemp.process(depth);

	funcStream::streamSelector(matOutput, config.stream, alignedFrame, data, depth, config.pixelZoom, config.pixelRoiZoom, config.scaleZoom);

	switch (config.state)
	{
	case SCANNERSTATE_BLUR:
		scannerBlur(matOutput, depth, intrinsics);
		break;
	default:
		break;
	}
}

// =================================================================================
// Plugin events
// =================================================================================

void zukiScanner::scannerMouseHandler(int event, int x, int y, int flags)
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

void zukiScanner::scannerKeyboardHandler()
{
	switch (config.stream)
	{
	case STREAM_COLOR:
		config.stream = STREAM_INFRARED;
		break;
	case STREAM_INFRARED:
		config.stream = STREAM_DEPTH;
		break;
	case STREAM_DEPTH:
		config.stream = STREAM_COLOR;
		break;
	default:
		break;
	}
}

// =================================================================================
// Plugin sub functions
// =================================================================================

void zukiScanner::scannerBlur(cv::Mat & matOutput, rs2::depth_frame & depth, rs2_intrinsics & intrinsics)
{
	cv::Mat edge;
	funcOpenCV::cannyBlur(matOutput, edge, 50, 150);

	std::vector<std::vector<cv::Point>> contour;
	double contourArea;
	std::vector<cv::Point> approx;
	int approxSize;
	int idx;
	scannerDetector(edge, contour, contourArea, approx, approxSize, idx);
	
	switch (approxSize)
	{
	case 4:
		scannerDrawerRect(matOutput, contour, contourArea, approx, idx, depth, intrinsics);
		break;
	case 5:
	case 6:
		scannerDrawerPoly(matOutput, contour, contourArea, approx, idx, depth, intrinsics);
		break;
	default:
		break;
	}
}

void zukiScanner::scannerDetector(cv::Mat & edge, std::vector<std::vector<cv::Point>> & contours, double & contourArea, std::vector<cv::Point> & approx, int & approxSize, int & idx)
{
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(edge, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
	std::sort(contours.begin(), contours.end(), funcOpenCV::contourSorter);
	
	for (int i = 0; i < (int)contours.size(); i += 1)
	{
		double area = cv::contourArea(contours[i]);
		if (area <= scanMinArea)
			break;

		cv::approxPolyDP(contours[i], approx, cv::arcLength(contours[i], true) * 0.01, true);
		
		int sizeApprox = (int)approx.size();
		if (sizeApprox >= 4 || sizeApprox <= 6)
		{
			contourArea = area;
			approxSize = sizeApprox;
			idx = i;
			break;
		}
	}
}

void zukiScanner::scannerDrawerRect(cv::Mat & matOutput, std::vector<std::vector<cv::Point>>& contour, double & contourArea, std::vector<cv::Point>& approx, int & idx, rs2::depth_frame & depth, rs2_intrinsics & intrinsics)
{
	if (funcGeometry2D::checkAspectRatio2D(approx[1], approx[0], approx[2], 4))
	{
		cv::Mat warped;
		funcOpenCV::fourPointTransform(matOutput, warped, approx);
		scannerCorrector(warped, approx, depth, intrinsics);

		cv::Size size = matOutput.size();
		cv::Size sizeW = warped.size();
		float heightMod = ((float)size.width / 4) * ((float)sizeW.height / (float)sizeW.width);
		if (heightMod > size.height - 100)
			heightMod = (float)(size.height - 100);
		cv::Size sizeMap = cv::Size((int)(size.width / 4), (int)heightMod);

		cv::Mat overlay;
		funcStream::streamMapperRD(matOutput, warped, overlay, sizeMap, scanMapSize, scanMapColor);
		matOutput = overlay.clone();
		cv::drawContours(overlay, contour, idx, scanRectColor, scanRectSize);
		cv::addWeighted(overlay, scanTrans, matOutput, scanTransO, 0, matOutput);
	}
}

void zukiScanner::scannerDrawerPoly(cv::Mat & matOutput, std::vector<std::vector<cv::Point>>& contour, double & contourArea, std::vector<cv::Point>& approx, int & idx, rs2::depth_frame & depth, rs2_intrinsics & intrinsics)
{
	cv::RotatedRect boundingBox = cv::minAreaRect(contour[idx]);
	cv::Point2f corners[4];
	boundingBox.points(corners);

	if (funcGeometry2D::checkAspectRatio2D(corners[1], corners[0], corners[2], 4))
	{
		std::vector<cv::Point> cornersV;
		for (int j = 0; j < 4; j += 1)
			cornersV.push_back(corners[j]);

		cv::Mat warped;
		funcOpenCV::fourPointTransform(matOutput, warped, cornersV);
		scannerCorrector(warped, cornersV, depth, intrinsics);

		cv::Size size = matOutput.size();
		cv::Size sizeW = warped.size();
		float heightMod = ((float)size.width / 4) * ((float)sizeW.height / (float)sizeW.width);
		if (heightMod > size.height - 100)
			heightMod = (float)(size.height - 100);
		cv::Size sizeMap = cv::Size((int)(size.width / 4), (int)heightMod);

		cv::Mat overlay;
		funcStream::streamMapperRD(matOutput, warped, overlay, sizeMap, scanMapSize, scanMapColor);
		matOutput = overlay.clone();
		cv::drawContours(overlay, contour, idx, scanRectColor, scanRectSize);
		
		cv::line(overlay, corners[0], corners[1], scanBoxColor, scanRectSize);
		cv::line(overlay, corners[1], corners[2], scanBoxColor, scanRectSize);
		cv::line(overlay, corners[2], corners[3], scanBoxColor, scanRectSize);
		cv::line(overlay, corners[3], corners[0], scanBoxColor, scanRectSize);
		
		cv::addWeighted(overlay, scanTrans, matOutput, scanTransO, 0, matOutput);
	}
}

void zukiScanner::scannerCorrector(cv::Mat & warped, std::vector<cv::Point>& corner, rs2::depth_frame & depth, rs2_intrinsics & intrinsics)
{
	float pixelA[2] = { (float)corner[1].x, (float)corner[1].y };
	float pixelB[2] = { (float)corner[0].x, (float)corner[0].y };
	float pixelC[2] = { (float)corner[2].x, (float)corner[2].y };

	float distA = funcGeometry3D::calcDist3D(pixelA, pixelB, &depth, &intrinsics);
	float distB = funcGeometry3D::calcDist3D(pixelA, pixelC, &depth, &intrinsics);

	cv::Size sizeW = warped.size();
	float mod;
	if (distA > distB)
	{
		if (sizeW.width > sizeW.height)
		{
			mod = (float)sizeW.height * distA / distB;
			cv::resize(warped, warped, cv::Size((int)mod, sizeW.height), 0, 0, CV_INTER_LINEAR);
		}
		else
		{
			mod = (float)sizeW.width * distA / distB;
			cv::resize(warped, warped, cv::Size(sizeW.height, (int)mod), 0, 0, CV_INTER_LINEAR);
		}
	}
	else
	{
		if (sizeW.width > sizeW.height)
		{
			mod = (float)sizeW.height * distB / distA;
			cv::resize(warped, warped, cv::Size((int)mod, sizeW.height), 0, 0, CV_INTER_LINEAR);
		}
		else
		{
			mod = (float)sizeW.width * distB / distA;
			cv::resize(warped, warped, cv::Size(sizeW.height, (int)mod), 0, 0, CV_INTER_LINEAR);
		}
	}
}

