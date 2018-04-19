#include "stdafx.h"
#include "funcStream.h"

void funcStream::streamInfoer(cv::Mat * input, std::string text)
{
	cv::Size size = input->size();

	cv::putText(*input, text, cv::Point(10, size.height - 10), inforerFontA, 1, inforerColorFA, 1, cv::LINE_AA);
	cv::putText(*input, text, cv::Point(10, size.height - 10), inforerFontB, 1, inforerColorFB, 1, cv::LINE_AA);
}

void funcStream::streamInfoerB(cv::Mat * input, std::string text)
{
	cv::Size size = input->size();
	cv::copyMakeBorder(*input, *input, 0, 40, 0, 0, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
	cv::putText(*input, text, cv::Point(10, size.height + 30), inforerFont, 1, inforerColor, 1, cv::LINE_AA);
}

void funcStream::streamMapperRD(cv::Mat & input, cv::Mat & miniInput, cv::Mat & output, cv::Size & sizeMap, int border, cv::Scalar color)
{
	output = input.clone();
	cv::Size size = output.size();
	cv::Mat minimap = miniInput.clone();
	cv::resize(minimap, minimap, sizeMap, 0, 0, CV_INTER_LINEAR);
	cv::copyMakeBorder(minimap, minimap, border, border, border, border, cv::BORDER_CONSTANT, color);
	minimap.copyTo(output(cv::Rect(size.width - minimap.cols - 10, size.height - minimap.rows - 10,
		minimap.cols, minimap.rows)));
}

void funcStream::streamMapperLD(cv::Mat & input, cv::Mat & miniInput, cv::Mat & output, cv::Size & sizeMap, int border, cv::Scalar color)
{
	output = input.clone();
	cv::Size size = output.size();
	cv::Mat minimap = miniInput.clone();
	cv::resize(minimap, minimap, sizeMap, 0, 0, CV_INTER_LINEAR);
	cv::copyMakeBorder(minimap, minimap, border, border, border, border, cv::BORDER_CONSTANT, color);
	minimap.copyTo(input(cv::Rect(10, size.height - minimap.rows - 10,
		minimap.cols, minimap.rows)));
}

void funcStream::streamZoomer(cv::Mat & input, cv::Mat & output, cv::Point & pixelZoom, cv::Point & pixelRoiZoom, float & scaleZoom)
{
	if (scaleZoom == 1)
	{
		output = input.clone();
	}
	else
	{
		pixelZoom.x = (int)(pixelZoom.x * scaleZoom) + pixelRoiZoom.x;
		pixelZoom.y = (int)(pixelZoom.y * scaleZoom) + pixelRoiZoom.y;

		cv::Size size = input.size();
		int scaledWidth = (int)(size.width * scaleZoom);
		int scaledHeight = (int)(size.height * scaleZoom);
		pixelRoiZoom.x = pixelZoom.x - (scaledWidth / 2);
		pixelRoiZoom.y = pixelZoom.y - (scaledHeight / 2);

		if ((pixelZoom.x + (scaledWidth / 2)) > size.width)
			pixelRoiZoom.x = size.width - scaledWidth;

		if ((pixelZoom.y + (scaledHeight / 2)) > size.height)
			pixelRoiZoom.y = size.height - scaledHeight;

		if ((pixelZoom.x - (scaledWidth / 2)) < 0)
			pixelRoiZoom.x = 0;

		if ((pixelZoom.y - (scaledHeight / 2)) < 0)
			pixelRoiZoom.y = 0;

		cv::Rect roi = cv::Rect(pixelRoiZoom.x, pixelRoiZoom.y, scaledWidth, scaledHeight);
		output = input(roi);
		cv::resize(output, output, size, 0, 0, CV_INTER_AREA);

		// generate mini-map
		cv::Mat outMap = input.clone();
		cv::rectangle(outMap, roi, zoomerLineColor, zoomerLineSize);
		cv::Size sizeMap = cv::Size((int)(size.width / 8), (int)(size.height / 8));
		streamMapperLD(output, outMap, output, sizeMap, zoomerMapSize, zoomerMapColor);
	}
}
