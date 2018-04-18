// AkizukiConsole.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "app.h"

void eventKeyboard(appState & state, std::string & windowTitle, cv::Mat & matOutput);

int main(int argc, char * argv[]) try
{
	std::string appTitle = "Teruzuki";
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
		eventKeyboard(akizuki.state, appTitle, akizuki.matOutput);
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

void eventKeyboard(appState & state, std::string & windowTitle, cv::Mat & matOutput)
{
	char key = cv::waitKey(10);

	if (key == 'q' || key == 'Q')
		state = APPSTATE_EXIT;
	else if (key == 'w' || key == 'W')
	{
		time_t t = std::time(nullptr);
#pragma warning( disable : 4996 )
		tm tm = *std::localtime(&t);

		std::ostringstream oss;
		oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
		std::string str = windowTitle + "_" + oss.str() + ".jpg";
		cv::imwrite(str, matOutput);
		std::cout << "file saved: " << str << std::endl;
	}
}


