#include "eyelib.hpp"

CascadeClassifier face_cascade;

void getGazePosition(Mat &frame);
Mat toHueScale(Mat img);

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
	cout << "Loading face cascade XML...";
	//load up face detection
	String face_cascade_name = "haarcascade_frontalface_alt.xml";
	if(!face_cascade.load(face_cascade_name)){
		cout << "\nCould not find face cascade file: " + face_cascade_name + "\n";
		return -1;
	}
	cout << "          done\n";
	cout << "Calculating gradient lookup tables...";
	//load lookup tables
	calcGradientLookup();
	cout << "done\n";

	//loop forever
	while (true) {
		cout << "Getting a frame\n";
		Mat cameraFrame;
		stream1.read(cameraFrame);

		getGazePosition(cameraFrame);

		if (waitKey(30) >= 0)
		break;
	}
	return 0;
}

//returns a 2D vector of where the eyes are pointing
void getGazePosition(Mat &frame){
	std::vector<Rect> faces;

	//get blue channel, less noisy
	std::vector<Mat> rgbChannels(3);
	split(frame, rgbChannels);
	Mat frame_gray = rgbChannels[2];

	//detect faces
	face_cascade.detectMultiScale( frame_gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE|CV_HAAR_FIND_BIGGEST_OBJECT, Size(150, 150) );

	//show rectangle around faces
	for(int i = 0; i < faces.size(); i++ )
	{
		rectangle(frame, faces[i], 1234);
	}

	//find eyes in the first (hopefully only) face
	if (faces.size() > 0) {
		getEyeVectors(frame, frame_gray, faces[0]);
	}

	//imshow("cam", frame);
}
