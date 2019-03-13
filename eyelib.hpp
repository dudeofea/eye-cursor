#pragma once
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <math.h>

using namespace cv;
using namespace std;

//Constants
#define FACE_SMOOTH 	0.0005
#define EYE_TOP 		0.25	//eye measurements relative to face box
#define EYE_SIDE 		0.13
#define EYE_HEIGHT 		0.30
#define EYE_WIDTH 		0.35
#define EYE_FRAME_SIZE	60		//size to resize eye box for speed
#define EYE_GRADIENT_THRESH	0.3
#define EYE_SCELRA_THRESH	30
#define EYE_BLUR_SIZE	3
#define MAX_CAMERAS		20

class GazeDetection {
public:
	int initialize();
	void getGazePosition(Mat &frame);
private:
	CascadeClassifier face_cascade;
	void getGazeVectors(Mat &frame, Mat &frame_grey, Rect face);
	void getGazeVector(Mat &frame, Mat &frame_grey, Rect eye);
};

CvPoint2D32f getEyeCenter(Mat &face_frame, Rect eye);
CvPoint2D32f getPupilCenter(Mat &eye_box);
