// AkizukiConsole.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "app.h"

int main(int argc, char * argv[]) try
{
	std::string appTitle = "Akizuki";
	app akizuki(appTitle);

	akizuki.setResolution(RS2_STREAM_COLOR, 1280, 720, 30);
	akizuki.setResolution(RS2_STREAM_DEPTH, 1280, 720, 30);
	akizuki.setVisualPreset("High Density");
	akizuki.cameraInitial();

	cv::namedWindow(appTitle, cv::WINDOW_AUTOSIZE);

	while (akizuki.state)
	{
		akizuki.cameraProcess();
		cv::imshow(appTitle, akizuki.matOutput);
	}

	return EXIT_SUCCESS;
}
catch (const rs2::error & e)
{
	std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;
	system("pause");
	return EXIT_FAILURE;
}
catch (const std::exception& e)
{
	std::cerr << e.what() << std::endl;
	system("pause");
	return EXIT_FAILURE;
}


