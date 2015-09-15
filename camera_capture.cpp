#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <math.h>

using namespace cv;
using namespace std;

//Globals
CascadeClassifier face_cascade;

//Constants
#define FACE_SMOOTH 	0.0005
#define EYE_TOP 		0.25	//eye measurements relative to face box
#define EYE_SIDE 		0.13
#define EYE_HEIGHT 		0.30
#define EYE_WIDTH 		0.35
#define EYE_RESIZE_WIDTH	50	//size to resize eye box for speed
#define EYE_GRADIENT_THRESH	0.3
#define EYE_SCELRA_THRESH	30
#define EYE_BLUR_SIZE	3

void getGazePosition(Mat &frame);
Mat toHueScale(Mat img);

int main() {
	//get camera stream
	VideoCapture stream1(0);   //0 is the id of video device
	if (!stream1.isOpened()) {
		cout << "cannot open camera";
	}

	//load up filters
	Mat high_pass_kernel = (Mat_<double>(3,3) << 0, 1, 0, 1, -4, 1, 0, 1, 0);
	String face_cascade_name = "haarcascade_frontalface_alt.xml";
	if(!face_cascade.load(face_cascade_name)){
		cout << "Could not find face cascade file: " + face_cascade_name + "\n";
		return -1;
	}

	//loop forever
	while (true) {
		Mat cameraFrame;
		stream1.read(cameraFrame);

		/// Applying Gaussian blur
		//GaussianBlur( cameraFrame, cameraFrame, Size( 0, 0 ), 10);

		// Apply highpass filter
		// filter2D(
		// 	cameraFrame, cameraFrame,	//src, dst
		// 	-1, 						//ddepth
		// 	high_pass_kernel,			//kernel
		// 	Point( -1, -1 ),			//anchor
		// 	0,							//delta
		// 	BORDER_DEFAULT
		// );

		getGazePosition(cameraFrame);

		if (waitKey(30) >= 0)
		break;
	}
	return 0;
}

void getEyeVectors(Mat &frame, Mat &frame_gray, Rect face);
void getEyeCorners(Mat &face_frame, Rect eye, Point *max_p, Point *min_p);
Point getEyeCenter(Mat &face, Rect eye);

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

//get positions of corners of eye
void getEyeCorners(Mat &face_frame, Rect eye, Point *max_p, Point *min_p){
	//crop eye to get rid of some noise
	int crop = 10;
	eye.y += crop;
	eye.height -= crop * 2;
	
	Mat eye_color = face_frame(eye);
	Mat eye_scelra;
	
	//convert to HSV and get saturation channel
	cvtColor(eye_color, eye_scelra, CV_RGB2HSV);
	std::vector<Mat> hsvChannels(3);
	split(eye_scelra, hsvChannels);
	eye_scelra = hsvChannels[1];

	//threshold
	threshold(eye_scelra, eye_scelra, 80, 255, THRESH_BINARY_INV);		//threshold type 3, thesh. to 0

	//erode to get rid of noise points
	Mat eroder = getStructuringElement( MORPH_CROSS,
		Size(3, 3),
		Point(1, 1)
	);
	erode(eye_scelra, eye_scelra, eroder);

	//calc min/max x and associated y's
	Point max = Point(0,0);
	Point min = Point(eye_scelra.cols, 0);
	for (int y = 0; y < eye_scelra.rows; ++y)
	{
		uchar* row = eye_scelra.ptr<uchar>(y);
		for (int x = 0; x < eye_scelra.cols; ++x)
		{
			if(row[x] > 0){
				if (x < min.x)
				{
					min = Point(x, y);
				}
				if (x > max.x)
				{
					max = Point(x, y);
				}
			}
		}
	}
	//adjust for crop
	min.y += crop;
	max.y += crop;
	if(max_p != NULL){
		*max_p = max;
	}
	if(min_p != NULL){
		*min_p = min;
	}
}

//get position of pupils
void getEyeVectors(Mat &frame, Mat &frame_gray, Rect face) {
	Mat face_frame = frame_gray(face);
	Mat face_frame_color = frame(face);

	//blur to remove some noise
	GaussianBlur(face_frame, face_frame, Size( 0, 0 ), FACE_SMOOTH * face.width);

	//get eye bounding boxes
	int eye_region_width = face.width * (EYE_WIDTH);
	int eye_region_height = face.width * (EYE_HEIGHT);
	int eye_region_top = face.height * (EYE_TOP);
	Rect left_eye_box(
		face.width*(EYE_SIDE),
		eye_region_top,
		eye_region_width,
		eye_region_height
	);
	Rect right_eye_box(
		face.width - eye_region_width - face.width*(EYE_SIDE),
		eye_region_top,
		eye_region_width,
		eye_region_height
	);

	//get center / eye corner
	Point left_pupil, left_corner;
	left_pupil = getEyeCenter(face_frame, left_eye_box);
	getEyeCorners(face_frame_color, left_eye_box, &left_corner, NULL);

	Point left_vec = left_corner - left_pupil;
	printf("x: %d, y: %d\n", left_vec.x, left_vec.y);
}

//returns a matrix with the gradient in the x direction
Mat computeGradient(const Mat &mat){
	Mat out(mat.rows,mat.cols,CV_64F);
	for (int y = 0; y < mat.rows; ++y) {
		const uchar *Mr = mat.ptr<uchar>(y);
		double *Or = out.ptr<double>(y);		
		Or[0] = (Mr[1] - Mr[0])*2.0;
		for (int x = 1; x < mat.cols - 1; ++x) {
			Or[x] = (Mr[x+1] - Mr[x-1]);
		}
		Or[mat.cols-1] = (Mr[mat.cols-1] - Mr[mat.cols-2])*2.0;
	}
	return out/2.0;
}

//returns a matrix of magnitudes given a matrix of x and y values
Mat matrixMagnitude(Mat &matX, Mat &matY) {
	Mat mags(matX.rows,matX.cols,CV_64F);
	for (int y = 0; y < matX.rows; ++y) {
		const double *Xr = matX.ptr<double>(y), *Yr = matY.ptr<double>(y);
		double *Mr = mags.ptr<double>(y);
		for (int x = 0; x < matX.cols; ++x) {
			double gX = Xr[x], gY = Yr[x];
			double magnitude = sqrt((gX * gX) + (gY * gY));
			Mr[x] = magnitude;
		}
	}
	return mags;
}

Point getEyeCenter(Mat &face, Rect eye){
	Mat eye_box = face(eye);
	
	//scale down for speed
	/*resize(
		eye_box, eye_box, 
		Size(EYE_RESIZE_WIDTH, EYE_RESIZE_WIDTH/eye_box.cols) * eye_box.rows
	);*/
	
	// draw eye region
	rectangle(face, eye, 1234);
	
	//find x and y gradients
	Mat gradientX = computeGradient(eye_box);
	Mat gradientY = computeGradient(eye_box.t()).t();
	
	//normalize and threshold the gradient
	Mat mags = matrixMagnitude(gradientX, gradientY);

	//compute the threshold
	Scalar stdMagnGrad, meanMagnGrad;
	meanStdDev(mags, meanMagnGrad, stdMagnGrad);
	double stdDev = stdMagnGrad[0] / sqrt(mags.rows*mags.cols);
	double gradientThresh = -8.0 * stdDev + meanMagnGrad[0];

	//normalize and threshold
	Point maxStart = Point(0,0), maxEnd = Point(0,0);		//points indicating start/end of thresholded zone
	for (int y = 0; y < eye_box.rows; ++y) {
		double *Xr = gradientX.ptr<double>(y), *Yr = gradientY.ptr<double>(y);
		double *Mr = mags.ptr<double>(y);
		for (int x = 0; x < eye_box.cols; ++x) {
			double gX = Xr[x], gY = Yr[x];
			double magnitude = Mr[x];
			if (magnitude > gradientThresh) {
				Xr[x] = gX/magnitude;
				Yr[x] = gY/magnitude;
				if(x > 10 && maxStart.x == 0 && maxStart.y == 0){
					maxStart.x = x;
					maxStart.y = y;
				}
				if(x < eye_box.rows - 10){
					maxEnd.x = x;
					maxEnd.y = y;
				}
			} else {
				Xr[x] = 0.0;
				Yr[x] = 0.0;
				Mr[x] = 0.0;
			}
		}
	}

	//printf("start: %d, %d, end: %d, %d\n", maxStart.x, maxStart.y, maxEnd.x, maxEnd.y);
	//imshow("cam", gradientX);

	//-- Create a blurred and inverted image for weighting
	Mat weight;
	GaussianBlur(eye_box, weight, Size(EYE_BLUR_SIZE, EYE_BLUR_SIZE), 0, 0);
	for (int y = 0; y < weight.rows; ++y) {
		unsigned char *row = weight.ptr<unsigned char>(y);
		for (int x = 0; x < weight.cols; ++x) {
			row[x] = (255 - row[x]);
		}
	}
	//imshow("cam", weight);
	//-- Run the algorithm!
	Mat outSum = Mat::zeros(eye_box.rows,eye_box.cols,CV_64F);
	// for each possible gradient location
	// Note: these loops are reversed from the way the paper does them
	// it evaluates every possible center for each gradient location instead of
	// every possible gradient location for every center.
	for (int y = 0; y < weight.rows; ++y) {
		const double *Xr = gradientX.ptr<double>(y), *Yr = gradientY.ptr<double>(y);
		for (int x = 0; x < weight.cols; ++x) {
			double gX = Xr[x], gY = Yr[x];
			if (gX == 0.0 && gY == 0.0) {
				continue;
			}
			// for all possible centers
			for (int cy = 0; cy < outSum.rows; ++cy) {
				double *Or = outSum.ptr<double>(cy);
				const unsigned char *Wr = weight.ptr<unsigned char>(cy);
				for (int cx = 0; cx < outSum.cols; ++cx) {
					if (x == cx && y == cy) {
						continue;
					}
					// create a vector from the possible center to the gradient origin
					double dx = x - cx;
					double dy = y - cy;
					// normalize d
					double magnitude = sqrt((dx * dx) + (dy * dy));
					dx = dx / magnitude;
					dy = dy / magnitude;
					double dotProduct = dx*gY + dy*gY;
					dotProduct = std::max(0.0,dotProduct);
					// square and multiply by the weight
					Or[cx] += dotProduct * dotProduct * (Wr[cx]/1.0);
				}
			}
		}
	}
	// scale all the values down, basically averaging them
	double numGradients = (weight.rows*weight.cols);
	Mat out;
	outSum.convertTo(out, CV_32F,1.0/numGradients);
	//-- Find the maximum point
	Point maxP;
	double maxVal;
	minMaxLoc(out, NULL,&maxVal,NULL,&maxP);
	circle(eye_box, maxP, 3, 200);
	//imshow("cam", eye_box);
	return maxP;
}