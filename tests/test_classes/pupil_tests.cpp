//
//	Run tests for detecting center of pupil
//
//	Basically, iterates through test images
//	looking for images sets of type "pupil_test"
//	and compares the output of the algorithm
//	to human-picked pupil centers for accuracy.
//
#include "pupil_tests.hpp"

class PupilImage: public ImageTest {
	public:
		float center_x, center_y;
		float time_ms;
		float error_rate;
		//constructor
		PupilImage(Json::Value json_data, string filename){
			Json::ValueIterator itr = json_data["pupil_positions"][filename].begin();
			center_x = stof(itr->asString()); itr++;
			center_y = stof(itr->asString());
			//store the file location
			image_path = "../tests/test_images/eye_images/" + filename + ".jpg";
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
			//waitKey(0);                                         	 // Wait for a keystroke in the window
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
			Mat image = imread(image_path, cv::IMREAD_COLOR);		// Read the file
			if(! image.data ){
				cout <<  "Could not open or find the image" << std::endl ;
				return image;
			}

			//namedWindow( "Display window", WINDOW_AUTOSIZE );		// Create a window for display.
			//imshow( "Display window", image );                   	// Show our image inside it.
			return image;
		}
};

void PupilTests::run_tests(){
	//load the test data
	ifstream json_file("../tests/test_images/eye_images/labels.json", ifstream::binary);
	Json::Value test_data;
	Json::Reader reader;
	std::ifstream test("testis.json", std::ifstream::binary);
	bool parsingSuccessful = reader.parse(json_file, test_data, false);
	if (!parsingSuccessful){
	    // report to the user the failure and their locations in the document.
	    cout << "Failed to parse configuration\n" << reader.getFormattedErrorMessages();
		return;
	}
	vector<PupilImage> test_images;
	for(Json::ValueIterator itr = test_data["pupil_positions"].begin() ; itr != test_data["pupil_positions"].end() ; itr++) {
		test_images.push_back(PupilImage(test_data, itr.key().asString()));
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
