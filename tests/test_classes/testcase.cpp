#include "testcase.hpp"

void TestCase::run_tests(){
	cout << "I'm a test case";
}

Mat ImageTest::load_test_image(){
	Mat image = imread(image_path, CV_LOAD_IMAGE_COLOR);   		// Read the file
	if(! image.data ){
		cout <<  "Could not open or find the image" << std::endl ;
		return image;
	}

	//namedWindow( "Display window", WINDOW_AUTOSIZE );		// Create a window for display.
	//imshow( "Display window", image );                   	// Show our image inside it.
	return image;
}

vector<string> get_images(const char *images_dir){
	//get all images to test
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
