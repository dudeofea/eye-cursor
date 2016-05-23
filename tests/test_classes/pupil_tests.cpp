//
//	Run tests for detecting center of pupil
//
//	Basically, iterates through test images
//	looking for images sets of type "pupil_test"
//	and compares the output of the algorithm
//	to human-picked pupil centers for accuracy.
//
#include "../../eyelib.hpp"
#include "pupil_tests.hpp"

class PupilImage {
	public:
		float center_x, center_y;
		float time_ms;
		float error_rate;
		string image_path;
		//constructor
		PupilImage(string filename){
			//get center from filename
			size_t slash = filename.find_last_of("/");
			size_t dash1 = filename.find("-", slash+1);
			size_t dash2 = filename.find("-", dash1+1);
			string x_str = filename.substr(slash+1, dash1-slash-1);
			string y_str = filename.substr(dash1+1, dash2-dash1-1);
			center_x = stof(x_str);
			center_y = stof(y_str);
			//store the file location
			image_path = filename;
		}
		//test the image using the pupil center function
		void test(){
			//load the image
			Mat test_image = this->load();
			//get blue channel, less noisy
			std::vector<Mat> test_image_rgb(3);
			split(test_image, test_image_rgb);
			Mat test_image_gray = test_image_rgb[2];
			//start the timer
			int timer_iterations = 10;	//how many times to run
			CvPoint2D32f result;
			std::clock_t start = std::clock();
			//run the algorithm
			for (int i = 0; i < timer_iterations; i++) {
				result = getPupilCenter(test_image_gray);
			}
			//print time
			int slash = image_path.find_last_of('/');
			std::cout << "pupil: " + image_path.substr(slash+1) << "\t\t" << (std::clock() - start) / (double)(timer_iterations * CLOCKS_PER_SEC / 1000) << "ms\t";
			waitKey(0);                                         	 // Wait for a keystroke in the window
			//calc error
			//cout << "answer: (" + to_string(center_x)+","+to_string(center_y)+")\n";
			//cout << "result: (" + to_string(result.x)+","+to_string(result.y)+")\n";
			float error_x = 100*abs(center_x-result.x)/result.x;
			float error_y = 100*abs(center_y-result.y)/result.y;
			float error = sqrt(error_x*error_x + error_y*error_y);
			cout << "error: "+to_string(error)+"%\n";
		}
		//load the image for use in testing
		Mat load(){
			Mat image = imread(image_path, CV_LOAD_IMAGE_COLOR);   		// Read the file
			if(! image.data ){
				cout <<  "Could not open or find the image" << std::endl ;
				return image;
			}

			//namedWindow( "Display window", WINDOW_AUTOSIZE );		// Create a window for display.
			//imshow( "Display window", image );                   	// Show our image inside it.
			return image;
		}
};

vector<string> get_images(){
	//get all images to test
	const char *images_dir = "test_images/pupil_images";
	vector<string> images_paths;
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(images_dir)) != NULL) {
		// print all files / folders in directory
		while ((ent = readdir (dir)) != NULL) {
			string filename = string(ent->d_name);
			//if a jpg file
			if(filename.substr(std::max(4, int(filename.size()))-4) == string(".jpg")){
				images_paths.push_back(string(images_dir) + "/" + filename);
			}
		}
		closedir (dir);
	} else {
		cout << "Could not open directory";
	}
	return images_paths;
}

void PupilTests::run_tests(){
	//load the test data
	vector<string> images_paths = get_images();
	vector<PupilImage> test_images;
	for(vector<int>::size_type i = 0; i != images_paths.size(); i++) {
		test_images.push_back(PupilImage(images_paths[i]));
	}
	//load gradient lookup tables
	calcGradientLookup();
	//run the tests
	vector<float> times;
	vector<float> error;
	for(vector<int>::size_type i = 0; i != test_images.size(); i++) {
		test_images[i].test();
	}
}
