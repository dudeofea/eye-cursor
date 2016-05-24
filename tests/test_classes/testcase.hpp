#pragma once
#include <iostream>
#include <vector>
#include <dirent.h>
#include "../../eyelib.hpp"
using namespace std;

class ImageTest {
	public:
		float time_ms;
		float error_rate;
		string image_path;
		Mat load_test_image();
};

class TestCase {
	public:
		virtual void run_tests();
};

vector<string> get_images(const char *images_dir);
