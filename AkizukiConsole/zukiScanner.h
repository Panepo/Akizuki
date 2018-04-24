#pragma once
#ifndef ZUKI_SCANNER_H
#define ZUKI_SCANNER_H

#include <librealsense2/rs.hpp>
#include <opencv2\opencv.hpp>

#include <vector>
#include <omp.h>

#include "funcStream.h"
#include "funcGeometry2D.h"
#include "funcGeometry3D.h"
#include "funcOpenCV.h"

#define scanTrans		0.4
#define scanTransO		(1 - scanTrans)

#define scanMinArea		10000
#define scanRectColor	cv::Scalar(0, 255,0)		// green
#define scanRectSize	2
#define scanMapSize		3
#define scanMapColor	cv::Scalar(0, 255,0)		// green
#define scanBoxColor	cv::Scalar(0, 255,255)		// yellow

typedef enum scannerState
{
	SCANNERSTATE_BLUR,
	SCANNERSTATE_SHARP,
	SCANNERSTATE_MULTI,
} scannerState;

class configScanner
{
public:
	scannerState state = SCANNERSTATE_BLUR;
	cv::Point pixelMouse = cv::Point(0, 0);
	std::string infoText = "Blur state";
};

class zukiScanner
{
public:
	void scannerMain(cv::Mat & matOutput, rs2::depth_frame & depth, rs2_intrinsics & intrinsics, configZoomer & configZoomer);
  
	void scannerMouseHandler(int event, int x, int y, int flags, configZoomer & configZoomer);
	void scannerKeyboardHandler();

	configScanner config;
private:
	void scannerBlur(cv::Mat & matOutput, rs2::depth_frame & depth, rs2_intrinsics & intrinsics);
	void scannerSharp(cv::Mat & matOutput, rs2::depth_frame & depth, rs2_intrinsics & intrinsics);
	void scannerMulti(cv::Mat & matOutput, rs2::depth_frame & depth, rs2_intrinsics & intrinsics);

	void scannerProcess(cv::Mat & matOutput, cv::Mat & edge, rs2::depth_frame & depth, rs2_intrinsics & intrinsics);

	void scannerDetector(
		cv::Mat & edge, 
		std::vector<std::vector<cv::Point>> & contourOutput, 
		double & contourArea, 
		std::vector<cv::Point> & approxOutput, 
		int & approxSize,
		int & idx
	);
	
	void scannerDrawerRect(
		cv::Mat & matOutput, 
		std::vector<std::vector<cv::Point>> & contour,
		double & contourArea, 
		std::vector<cv::Point> & approx, 
		int & idx,
		rs2::depth_frame & depth, 
		rs2_intrinsics & intrinsics
	);

	void scannerDrawerPoly(
		cv::Mat & matOutput,
		std::vector<std::vector<cv::Point>> & contour,
		double & contourArea,
		std::vector<cv::Point> & approx,
		int & idx,
		rs2::depth_frame & depth,
		rs2_intrinsics & intrinsics
	);

	void scannerCorrector(cv::Mat & warped, std::vector<cv::Point> & corner, rs2::depth_frame & depth, rs2_intrinsics & intrinsics);
};

#endif
