//
//	Run tests for detecting center of pupil
//
//	Basically, iterates through test images
//	looking for images sets of type "pupil_test"
//	and compares the output of the algorithm
//	to human-picked pupil centers for accuracy.
//
#include "../../eyelib.hpp"
#include "center_tests.hpp"

class EyeCenterImage: public ImageTest {
	public:
		float center_x, center_y;
		string image_path;
		//constructor
		EyeCenterImage(string filename){
		}
		//test the image using the pupil center function
		void test(){
		}
};

void CenterTests::run_tests(){
	//load the test data
	vector<string> images_paths = get_images("../tests/test_images/eye_images");
	vector<EyeCenterImage> test_images;
	for(vector<int>::size_type i = 0; i != images_paths.size(); i++) {
		test_images.push_back(EyeCenterImage(images_paths[i]));
	}
	//run the tests
	vector<float> times;
	vector<float> error;
	for(vector<int>::size_type i = 0; i != test_images.size(); i++) {
		test_images[i].test();
	}
}
