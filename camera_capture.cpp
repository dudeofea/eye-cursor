#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>

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
#define EYE_GRADIENT_THRESH	25.0
#define EYE_BLUR_SIZE	5

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

Mat toHueScale(Mat img){
	double h = 0.0, s = 0.5, v = 0.5, ff, p, q, t;
	//make color array
	std::vector<cv::Mat> rgb_mat;
	rgb_mat.push_back(Mat::zeros(img.rows, img.cols, CV_64F));
	rgb_mat.push_back(Mat::zeros(img.rows, img.cols, CV_64F));
	rgb_mat.push_back(Mat::zeros(img.rows, img.cols, CV_64F));
	for (int i = 0; i < img.rows; ++i)
	{
		uchar* img_p = img.ptr<uchar>(i);
		uchar* r_mat = rgb_mat[0].ptr<uchar>(i);
		uchar* g_mat = rgb_mat[1].ptr<uchar>(i);
		uchar* b_mat = rgb_mat[2].ptr<uchar>(i);
		for (int j = 0; j < img.cols; ++j)
		{
			//Turn brightness into hue
			h = 360.0 * img_p[j]/255.0;
			//convert to RGB
			h /= 60.0;
			long k = (long)h;
			ff = h - k;
			p = v * (1.0 - s);
			q = v * (1.0 - (s * ff));
			t = v * (1.0 - (s * (1.0 - ff)));
			r_mat[j] = (double)img_p[j];
			// switch(i) {
			// case 0:
			// 	r_mat[j] = v;
			// 	g_mat[j] = t;
			// 	b_mat[j] = p;
			// break;
			// case 1:
			// 	r_mat[j] = q;
			// 	g_mat[j] = v;
			// 	b_mat[j] = p;
			// break;
			// case 2:
			// 	r_mat[j] = p;
			// 	g_mat[j] = v;
			// 	b_mat[j] = t;
			// break;
			// case 3:
			// 	r_mat[j] = p;
			// 	g_mat[j] = q;
			// 	b_mat[j] = v;
			// break;
			// case 4:
			// 	r_mat[j] = t;
			// 	g_mat[j] = p;
			// 	b_mat[j] = v;
			// break;
			// case 5:
			// default:
			// 	r_mat[j] = v;
			// 	g_mat[j] = p;
			// 	b_mat[j] = q;
			// break;
			// }
		}
	}
	Mat color;
	merge(rgb_mat, color);
	return color;
}

void getEyeVectors(Mat &frame_gray, Rect face);
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
		getEyeVectors(frame_gray, faces[0]);
	}

	//imshow("cam", frame);
}

void getEyeVectors(Mat &frame_gray, Rect face) {
	Mat face_frame = frame_gray(face);

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

	//-- Find Eye Centers
	Point left_pupil = getEyeCenter(face_frame, left_eye_box);
	// Point right_pupil = getEyeCenter(face_frame, right_eye_box);
	// right_pupil.x += right_eye_box.x;
	// right_pupil.y += right_eye_box.y;
	// left_pupil.x += left_eye_box.x;
	// left_pupil.y += left_eye_box.y;
	// // draw eye centers
	// circle(face_frame, right_pupil, 3, 1234);
	// circle(face_frame, left_pupil, 3, 1234);

	// //-- Find Eye Corners
	// if (kEnableEyeCorner) {
	// 	Point2f leftRightCorner = findEyeCorner(face_frame(leftRightCornerRegion), true, false);
	// 	leftRightCorner.x += leftRightCornerRegion.x;
	// 	leftRightCorner.y += leftRightCornerRegion.y;
	// 	Point2f leftLeftCorner = findEyeCorner(face_frame(leftLeftCornerRegion), true, true);
	// 	leftLeftCorner.x += leftLeftCornerRegion.x;
	// 	leftLeftCorner.y += leftLeftCornerRegion.y;
	// 	Point2f rightLeftCorner = findEyeCorner(face_frame(rightLeftCornerRegion), false, true);
	// 	rightLeftCorner.x += rightLeftCornerRegion.x;
	// 	rightLeftCorner.y += rightLeftCornerRegion.y;
	// 	Point2f rightRightCorner = findEyeCorner(face_frame(rightRightCornerRegion), false, false);
	// 	rightRightCorner.x += rightRightCornerRegion.x;
	// 	rightRightCorner.y += rightRightCornerRegion.y;
	// 	circle(face_frame, leftRightCorner, 3, 200);
	// 	circle(face_frame, leftLeftCorner, 3, 200);
	// 	circle(face_frame, rightLeftCorner, 3, 200);
	// 	circle(face_frame, rightRightCorner, 3, 200);
	// }

	//imshow("cam", face_frame);
	//  Rect roi( Point( 0, 0 ), face_frame.size());
	//  Mat destinationROI = debugImage( roi );
	//  face_frame.copyTo( destinationROI );
}

//returns a matrix with the gradient in the x direction
Mat computeGradient(const Mat &mat){
	Mat out(mat.rows,mat.cols,CV_64F);
	for (int y = 0; y < mat.rows; ++y) {
		const uchar *Mr = mat.ptr<uchar>(y);
		double *Or = out.ptr<double>(y);		
		Or[0] = Mr[1] - Mr[0];
		for (int x = 1; x < mat.cols - 1; ++x) {
			Or[x] = (Mr[x+1] - Mr[x-1]);
		}
		Or[mat.cols-1] = Mr[mat.cols-1] - Mr[mat.cols-2];
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
	double gradientThresh = EYE_GRADIENT_THRESH * stdDev + meanMagnGrad[0];
	
	//normalize and threshold
	for (int y = 0; y < eye_box.rows; ++y) {
		double *Xr = gradientX.ptr<double>(y), *Yr = gradientY.ptr<double>(y);
		double *Mr = mags.ptr<double>(y);
		for (int x = 0; x < eye_box.cols; ++x) {
			double gX = Xr[x], gY = Yr[x];
			double magnitude = Mr[x];
			if (magnitude > gradientThresh) {
				Xr[x] = gX/magnitude;
				Yr[x] = gY/magnitude;
			} else {
				Xr[x] = 0.0;
				Yr[x] = 0.0;
				Mr[x] = 0.0;
			}
		}
	}

	//-- Create a blurred and inverted image for weighting
	Mat weight;
	GaussianBlur(eye_box, weight, Size(EYE_BLUR_SIZE, EYE_BLUR_SIZE), 0, 0);
	for (int y = 0; y < weight.rows; ++y) {
		unsigned char *row = weight.ptr<unsigned char>(y);
		for (int x = 0; x < weight.cols; ++x) {
			row[x] = (255 - row[x]);
		}
	}
	imshow("cam", toHueScale(weight));
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
	// return maxP;
}