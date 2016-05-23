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
		int center_x, center_y;
		string image_path;
		//constructor
		PupilImage(string filename){
			//get center from filename
			size_t slash = filename.find_last_of("/");
			size_t dash1 = filename.find("-", slash+1);
			size_t dash2 = filename.find("-", dash1+1);
			string x_str = filename.substr(slash+1, dash1-slash-1);
			string y_str = filename.substr(dash1+1, dash2-dash1-1);
			center_x = stoi(x_str);
			center_y = stoi(y_str);
			//store the file location
			image_path = filename;
		}
		//load the image for use in testing
		Mat load(){
			Mat image = imread(image_path, CV_LOAD_IMAGE_COLOR);   		// Read the file
			if(! image.data ){
				cout <<  "Could not open or find the image" << std::endl ;
				return image;
			}

			namedWindow( "Display window", WINDOW_AUTOSIZE );		// Create a window for display.
			imshow( "Display window", image );                   	// Show our image inside it.

			waitKey(0);                                         	 // Wait for a keystroke in the window
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
		test_images[i].load();
	}
}
