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

//Globals
CascadeClassifier face_cascade;
float dpX[EYE_FRAME_SIZE*EYE_FRAME_SIZE], dpY[EYE_FRAME_SIZE*EYE_FRAME_SIZE];

void getGazePosition(Mat &frame);
Mat toHueScale(Mat img);

int main() {
	//get camera stream
	VideoCapture stream1(0);   //0 is the id of video device
	if (!stream1.isOpened()) {
		cout << "cannot open camera";
	}

	//load up face detection
	String face_cascade_name = "haarcascade_frontalface_alt.xml";
	if(!face_cascade.load(face_cascade_name)){
		cout << "Could not find face cascade file: " + face_cascade_name + "\n";
		return -1;
	}
	//load up vector lookup tables
	for (size_t y = 0; y < EYE_FRAME_SIZE; y++) {
		for (int x = 0; x < EYE_FRAME_SIZE; ++x) {
			float magnitude = sqrt((x * x) + (y * y));
			dpX[x+EYE_FRAME_SIZE*y] = (float)x / magnitude;
			dpY[x+EYE_FRAME_SIZE*y] = (float)y / magnitude;
		}
	}

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

void getEyeVectors(Mat &frame, Mat &frame_gray, Rect face);
CvPoint2D32f getEyeCenter(Mat &face_frame, Rect eye);
CvPoint2D32f getPupilCenter(Mat &face, Rect eye);

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
	int eye_region_side =face.width  * (EYE_SIDE);
	Rect left_eye_box(
		eye_region_side,
		eye_region_top,
		eye_region_width,
		eye_region_height
	);
	Rect right_eye_box(
		face.width - eye_region_width - eye_region_side,
		eye_region_top,
		eye_region_width,
		eye_region_height
	);

	//get left center / eye corner
	CvPoint2D32f left_pupil, left_center;
	left_pupil = getPupilCenter(face_frame, left_eye_box);
	left_center = getEyeCenter(face_frame_color, left_eye_box);
	//fix offset
	left_pupil.x  += eye_region_side;
	left_pupil.y  += eye_region_top;
	left_center.x += eye_region_side;
	left_center.y += eye_region_top;

	//get right center / eye corner
	CvPoint2D32f right_pupil, right_center;
	right_pupil = getPupilCenter(face_frame, right_eye_box);
	right_center = getEyeCenter(face_frame_color, right_eye_box);
	//fix offset
	right_pupil.x  += face.width - eye_region_width - eye_region_side;
	right_pupil.y  += eye_region_top;
	right_center.x += face.width - eye_region_width - eye_region_side;
	right_center.y += eye_region_top;

	circle(face_frame, left_pupil, 2, 200);
	circle(face_frame, right_pupil, 2, 200);
	//circle(face_frame, left_center, 3, 200);
	//circle(face_frame, right_center, 3, 200);
	//resize for debugging
	//resize(face_frame, face_frame, Size(500,500), 0, 0, INTER_NEAREST);
	//imshow("cam", face_frame);

	//Point left_vec = left_corner - left_pupil;
	//printf("x: %d, y: %d\n", left_vec.x, left_vec.y);
}

//returns a matrix with the gradient in the x direction
Mat computeGradient(const Mat &mat){
	Mat out(mat.rows,mat.cols, CV_32S);
	for (int y = 0; y < mat.rows; ++y) {
		const uchar *Mr = mat.ptr<uchar>(y);
		int *Or = out.ptr<int>(y);
		Or[0] = (Mr[1] - Mr[0])*2.0;
		for (int x = 1; x < mat.cols - 1; ++x) {
			Or[x] = (Mr[x+1] - Mr[x-1]);
		}
		Or[mat.cols-1] = (Mr[mat.cols-1] - Mr[mat.cols-2])*2.0;
	}
	return out;
}

//returns a matrix of magnitudes given a matrix of x and y values
Mat matrixMagnitude(Mat &matX, Mat &matY) {
	Mat mags(matX.rows,matX.cols,CV_32F);
	for (int y = 0; y < matX.rows; ++y) {
		const int *grad_x = matX.ptr<int>(y), *grad_y = matY.ptr<int>(y);
		float *Mr = mags.ptr<float>(y);
		for (int x = 0; x < matX.cols; ++x) {
			int gX = grad_x[x], gY = grad_y[x];
			float magnitude = sqrt((gX * gX) + (gY * gY));
			Mr[x] = magnitude;
		}
	}
	return mags;
}

CvPoint2D32f getPupilCenter(Mat &face, Rect eye){
	Mat eye_box = face(eye);

	//find x and y gradients
	Mat gradientX = computeGradient(eye_box);
	Mat gradientY = computeGradient(eye_box.t()).t();

	//normalize and threshold the gradient
	Mat mags = matrixMagnitude(gradientX, gradientY);

	//create a blurred and inverted image for weighting
	Mat weight;
	bitwise_not(eye_box, weight);
	blur(weight, weight, Size(2,2));

	//weight the magnitudes, convert to 8-bit for thresholding
	weight.convertTo(weight, CV_32F);
	mags = mags.mul(weight);
	normalize(mags, mags, 0, 1, NORM_MINMAX, CV_32F);
	mags.convertTo(mags, CV_8UC1, 255);

	//threshold using Otsu's method
	threshold(mags, mags, 0, 255, THRESH_BINARY | THRESH_OTSU);

	//convert to CV_32S and filter gradients
	mags.convertTo(mags, CV_32S);
	gradientY = gradientY.mul(mags);
	gradientX = gradientX.mul(mags);

	//resize arrays to same size
	resize(gradientX, gradientX, Size(EYE_FRAME_SIZE, EYE_FRAME_SIZE), 0, 0, INTER_NEAREST);
	resize(gradientY, gradientY, Size(EYE_FRAME_SIZE, EYE_FRAME_SIZE), 0, 0, INTER_NEAREST);
	resize(weight, weight, Size(EYE_FRAME_SIZE, EYE_FRAME_SIZE), 0, 0, INTER_NEAREST);

	imshow("gradY", gradientY * 255);
	imshow("weight", weight / 255);

	//run the algorithm:
	//	for each possible gradient location
	//	Note: these loops are reversed from the way the paper does them
	//	it evaluates every possible center for each gradient location instead of
	//	every possible gradient location for every center.
	Mat out = Mat::zeros(weight.rows,weight.cols, CV_32F);
	float max_val = 0;
	//for all pixels in the image
	for (int y = 0; y < EYE_FRAME_SIZE; ++y) {
		const int *grad_x = gradientX.ptr<int>(y), *grad_y = gradientY.ptr<int>(y);
		for (int x = 0; x < EYE_FRAME_SIZE; ++x) {
			int gX = grad_x[x], gY = grad_y[x];
			if (gX == 0 && gY == 0) {
				continue;
			}
			//for all possible centers
			for (int cy = 0; cy < EYE_FRAME_SIZE; ++cy) {
				float *Or = out.ptr<float>(cy);
				const float *Wr = weight.ptr<float>(cy);
				for (int cx = 0; cx < EYE_FRAME_SIZE; ++cx) {
					//ignore center of box
					if (x == cx && y == cy) {
						continue;
					}
					//create a vector from the possible center to the gradient origin
					int dx = x - cx;
					int dy = y - cy;

					//compute dot product using lookup table
					float dotProduct;
					if(dx > 0 && dy > 0){
						dotProduct = dpX[dx+EYE_FRAME_SIZE*dy]*gX + dpY[dx+EYE_FRAME_SIZE*dy]*gY;
					}else if(dx > 0){
						dotProduct = dpX[dx-EYE_FRAME_SIZE*dy]*gX - dpY[dx-EYE_FRAME_SIZE*dy]*gY;
					}else if(dy > 0){
						dotProduct = -dpX[-dx+EYE_FRAME_SIZE*dy]*gX - dpY[-dx+EYE_FRAME_SIZE*dy]*gY;
					}else{
						dotProduct = -dpX[-dx-EYE_FRAME_SIZE*dy]*gX - dpY[-dx-EYE_FRAME_SIZE*dy]*gY;
					}

					//ignore negative dot products as they point away from eye
					if(dotProduct <= 0.0){
						continue;
					}

					//square and multiply by the weight
					Or[cx] += dotProduct * Wr[cx];

					//compare with max
					if(Or[cx] > max_val){
						max_val = Or[cx];
					}
				}
			}
		}
	}

	//resize for debugging
	resize(out, out, Size(500,500), 0, 0, INTER_NEAREST);

	imshow("calc", out / max_val);

	//TODO: calc histogram and threshold based on
	//number of pixels in top end. get at least 10 pixels
	//or so in the top end

	//threshold to get just the pupil
	threshold(out, out, 0.92 * max_val, max_val, THRESH_TOZERO);

	//calc center of mass
	float sum = 0;
	float sum_x = 0;
	float sum_y = 0;
	for (int y = 0; y < out.rows; ++y)
	{
		float* row = out.ptr<float>(y);
		for (int x = 0; x < out.cols; ++x)
		{
			float val = row[x]*row[x];
			if(val > 0){
				sum += val;
				sum_x += val*x;
				sum_y += val*y;
			}
		}
	}
	CvPoint2D32f max = cvPoint2D32f(sum_x/sum, sum_y/sum);
	circle(out, max, 3, 0);
	imshow("thresh", out / max_val);
	return max;
}

//get position of center of eye
CvPoint2D32f getEyeCenter(Mat &face_frame, Rect eye){
	//crop eye to get rid of some noise
	int crop_y = 10;
	eye.y += crop_y;
	eye.height -= crop_y * 2;
	//eye.width -= 20;

	Mat eye_color = face_frame(eye);
	Mat eye_scelra;

	//convert to HSV and get saturation channel
	cvtColor(eye_color, eye_scelra, CV_RGB2HSV);
	std::vector<Mat> hsvChannels(3);
	split(eye_scelra, hsvChannels);
	eye_scelra = hsvChannels[1].mul(hsvChannels[2]) / 30;

	//invert
	bitwise_not(eye_scelra, eye_scelra);

	//blur
	blur(eye_scelra, eye_scelra, Size(4,4));

	//apply histogram equalization
	equalizeHist(eye_scelra, eye_scelra);

	//threshold
	//threshold(eye_scelra, eye_scelra, 10, 255, THRESH_BINARY_INV);		//threshold type 3, thesh. to 0

	//calc center of mass
	float sum = 0;
	float sum_x = 0;
	float sum_y = 0;
	for (int y = 0; y < eye_scelra.rows; ++y)
	{
		uchar* row = eye_scelra.ptr<uchar>(y);
		for (int x = 0; x < eye_scelra.cols; ++x)
		{
			sum += row[x];
			sum_x += row[x]*x;
			sum_y += row[x]*y;
		}
	}
	CvPoint2D32f max = cvPoint2D32f(sum_x/sum, sum_y/sum);
	//circle(eye_scelra, max, 3, 0);
	//imshow("eye", eye_scelra);
	//adjust for crop
	max.y += crop_y;
	return max;
}
