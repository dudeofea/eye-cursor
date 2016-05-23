#pragma once
#include <iostream>
#include <vector>
#include <dirent.h>
using namespace std;

class TestCase {
	public:
	virtual void run_tests();
};

vector<string> get_images(const char *images_dir);
