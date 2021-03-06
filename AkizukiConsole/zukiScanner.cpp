#include "stdafx.h"
#include "zukiScanner.h"

// =================================================================================
// Plugin main process
// =================================================================================

void zukiScanner::scannerMain(cv::Mat & matOutput, rs2::depth_frame & depth, rs2_intrinsics & intrinsics, configZoomer & configZoomer)
{
	switch (config.state)
	{
	case SCANNERSTATE_BLUR:
		scannerBlur(matOutput, depth, intrinsics, configZoomer);
		break;
	case SCANNERSTATE_SHARP:
		scannerSharp(matOutput, depth, intrinsics, configZoomer);
		break;
	case SCANNERSTATE_MULTI:
		scannerMulti(matOutput, depth, intrinsics, configZoomer);
		break;
	default:
		break;
	}
}

// =================================================================================
// Plugin events
// =================================================================================

void zukiScanner::scannerMouseHandler(int event, int x, int y, int flags, configZoomer & configZoomer)
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

void zukiScanner::scannerKeyboardHandler()
{
	//switch (config.stream)
	//{
	//case STREAM_COLOR:
	//	config.stream = STREAM_INFRARED;
	//	break;
	//case STREAM_INFRARED:
	//	config.stream = STREAM_DEPTH;
	//	break;
	//case STREAM_DEPTH:
	//	config.stream = STREAM_COLOR;
	//	break;
	//default:
	//	break;
	//}

	switch (config.state)
	{
	case SCANNERSTATE_BLUR:
		config.state = SCANNERSTATE_SHARP;
		config.infoText = "Sharp state";
		break;
	case SCANNERSTATE_SHARP:
		config.state = SCANNERSTATE_MULTI;
		config.infoText = "Multi state";
		break;
	case SCANNERSTATE_MULTI:
		config.state = SCANNERSTATE_BLUR;
		config.infoText = "Blur state";
	default:
		break;
	}
}

// =================================================================================
// Plugin sub states
// =================================================================================

void zukiScanner::scannerBlur(cv::Mat & matOutput, rs2::depth_frame & depth, rs2_intrinsics & intrinsics, configZoomer & configZoomer)
{
	cv::Mat edge;
	funcOpenCV::cannyBlur(matOutput, edge, 50, 150);
	scannerProcess(matOutput, edge, depth, intrinsics, configZoomer);
}

void zukiScanner::scannerSharp(cv::Mat & matOutput, rs2::depth_frame & depth, rs2_intrinsics & intrinsics, configZoomer & configZoomer)
{
	cv::Mat edge;
	funcOpenCV::cannySharp(matOutput, edge, 50, 150);
	scannerProcess(matOutput, edge, depth, intrinsics, configZoomer);
}

void zukiScanner::scannerMulti(cv::Mat & matOutput, rs2::depth_frame & depth, rs2_intrinsics & intrinsics, configZoomer & configZoomer)
{
	cv::Mat edgeBlur;
	std::vector<std::vector<cv::Point>> contourBlur;
	double contourAreaBlur;
	std::vector<cv::Point> approxBlur;
	int approxSizeBlur;
	int idxBlur;

	cv::Mat edgeSharp;
	std::vector<std::vector<cv::Point>> contourSharp;
	double contourAreaSharp;
	std::vector<cv::Point> approxSharp;
	int approxSizeSharp;
	int idxSharp;

	#pragma omp parallel sections
	{
		#pragma omp section
		{
			funcOpenCV::cannySharp(matOutput, edgeBlur, 50, 150);
			scannerDetector(edgeBlur, contourBlur, contourAreaBlur, approxBlur, approxSizeBlur, idxBlur);
		}
		#pragma omp section
		{
			funcOpenCV::cannySharp(matOutput, edgeSharp, 50, 150);
			scannerDetector(edgeSharp, contourSharp, contourAreaSharp, approxSharp, approxSizeSharp, idxSharp);
		}
	}

	if (contourAreaBlur >= contourAreaSharp)
	{
		switch (approxSizeBlur)
		{
		case 4:
			scannerDrawerRect(matOutput, contourBlur, contourAreaBlur, approxBlur, idxBlur, depth, intrinsics, configZoomer);
			break;
		case 5:
		case 6:
			scannerDrawerPoly(matOutput, contourBlur, contourAreaBlur, approxBlur, idxBlur, depth, intrinsics, configZoomer);
			break;
		default:
			break;
		}
	}
	else
	{
		switch (approxSizeSharp)
		{
		case 4:
			scannerDrawerRect(matOutput, contourSharp, contourAreaSharp, approxSharp, idxSharp, depth, intrinsics, configZoomer);
			break;
		case 5:
		case 6:
			scannerDrawerPoly(matOutput, contourSharp, contourAreaSharp, approxSharp, idxSharp, depth, intrinsics, configZoomer);
			break;
		default:
			break;
		}
	}
}

// =================================================================================
// Plugin sub functions
// =================================================================================

void zukiScanner::scannerProcess(cv::Mat & matOutput, cv::Mat & edge, rs2::depth_frame & depth, rs2_intrinsics & intrinsics, configZoomer & configZoomer)
{
	std::vector<std::vector<cv::Point>> contour;
	double contourArea;
	std::vector<cv::Point> approx;
	int approxSize;
	int idx;
	scannerDetector(edge, contour, contourArea, approx, approxSize, idx);

	switch (approxSize)
	{
	case 4:
		scannerDrawerRect(matOutput, contour, contourArea, approx, idx, depth, intrinsics, configZoomer);
		break;
	case 5:
	case 6:
		scannerDrawerPoly(matOutput, contour, contourArea, approx, idx, depth, intrinsics, configZoomer);
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

void zukiScanner::scannerDrawerRect(cv::Mat & matOutput, std::vector<std::vector<cv::Point>>& contour, double & contourArea, std::vector<cv::Point>& approx, int & idx, rs2::depth_frame & depth, rs2_intrinsics & intrinsics, configZoomer & configZoomer)
{
	if (funcGeometry2D::checkAspectRatio2D(approx[1], approx[0], approx[2], 4))
	{
		cv::Mat warped;
		funcOpenCV::fourPointTransform(matOutput, warped, approx);
		scannerCorrector(warped, approx, depth, intrinsics, configZoomer);

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

void zukiScanner::scannerDrawerPoly(cv::Mat & matOutput, std::vector<std::vector<cv::Point>>& contour, double & contourArea, std::vector<cv::Point>& approx, int & idx, rs2::depth_frame & depth, rs2_intrinsics & intrinsics, configZoomer & configZoomer)
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
		scannerCorrector(warped, cornersV, depth, intrinsics, configZoomer);

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

void zukiScanner::scannerCorrector(cv::Mat & warped, std::vector<cv::Point>& corner, rs2::depth_frame & depth, rs2_intrinsics & intrinsics, configZoomer & configZoomer)
{
	cv::Point pixelA, pixelB, pixelC;
	funcStream::streamZoomPixelTrans(corner[1], pixelA, configZoomer);
	funcStream::streamZoomPixelTrans(corner[0], pixelB, configZoomer);
	funcStream::streamZoomPixelTrans(corner[2], pixelC, configZoomer);

	float pixelAA[2] = { (float)pixelA.x, (float)pixelA.y };
	float pixelBA[2] = { (float)pixelB.x, (float)pixelB.y };
	float pixelCA[2] = { (float)pixelC.x, (float)pixelC.y };

	float distA = funcGeometry3D::calcDist3D(pixelAA, pixelBA, &depth, &intrinsics);
	float distB = funcGeometry3D::calcDist3D(pixelAA, pixelCA, &depth, &intrinsics);

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

