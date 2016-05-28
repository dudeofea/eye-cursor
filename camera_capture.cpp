#include "eyelib.hpp"

void getGazePosition(Mat &frame);
Mat toHueScale(Mat img);

//Get the number of cameras available
int countCameras(){
	VideoCapture temp_camera;
	int maxTested = 10;
	for (int i = 0; i < maxTested; i++){
		VideoCapture temp_camera(i);
		bool res = (!temp_camera.isOpened());
		temp_camera.release();
		if (res){
			return i;
		}
	}
	return maxTested;
}

int main() {
	//get camera stream
	VideoCapture stream1(countCameras());   //id of video device
	if (!stream1.isOpened()) {
		cout << "cannot open camera";
	}

	//load up face detection
	String face_cascade_name = "haarcascade_frontalface_alt.xml";
	if(!face_cascade.load(face_cascade_name)){
		cout << "Could not find face cascade file: " + face_cascade_name + "\n";
		return -1;
	}
	//load lookup tables
	calcGradientLookup();

	//loop forever
	while (true) {
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
