#include "eyelib.hpp"
#include <time.h>
#include <iomanip>

void getGazePosition(Mat &frame);

int main(int argc, char** argv) {
	//check arguments
	if(argc != 2){
		cout << "Usage: camera_capture <video_stream>" << "\n";
		return -1;
	}
	cout << "OpenCV version : " << CV_VERSION << "\n";
	//cout << "Major version : " << CV_MAJOR_VERSION << "\n";
	//cout << "Minor version : " << CV_MINOR_VERSION << "\n";
	//cout << "Subminor version : " << CV_SUBMINOR_VERSION << "\n";
	cout << "Loading video source...";
	//get camera stream
	VideoCapture stream1(argv[1], CAP_V4L);   //id of video device
	if (!stream1.isOpened()) {
		stream1.release();
		cout << "\nCould not open video source " << argv[1] << "\n";
		return -1;
	}
	cout << "              done\n";

	//Init Gaze Detector
	GazeDetection gd;
	if(gd.initialize() < 0){
		return -1;
	}

	//cout formatting
	cout << std::fixed;
    cout << std::setprecision(2);
	float time_scale = 1000.0 / CLOCKS_PER_SEC;

	//loop forever
	while (true) {
		clock_t begin_frame = clock();
		//process the next frame
		Mat cameraFrame;
		stream1.read(cameraFrame);
		clock_t begin_processing = clock();
		gd.getGazePosition(cameraFrame);

		cout << "frame in " << time_scale * float( begin_processing - begin_frame ) << "ms, processed in " << time_scale * float(clock() - begin_processing) << "ms\n";

		if (waitKey(30) >= 0)
		break;
	}
	return 0;
}
