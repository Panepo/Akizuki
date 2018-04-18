#include "stdafx.h"
#include "funcStream.h"

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
		funcOpenCV::addMinimapLD(output, outMap, output, sizeMap, zoomerMapSize, zoomerMapColor);
	}
}
