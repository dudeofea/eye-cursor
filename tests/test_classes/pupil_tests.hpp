#pragma once
#include <iostream>
#include <dirent.h>
#include <vector>
#include <algorithm>
#include <string> 
#include "testcase.hpp"
using namespace std;

class PupilTests: public TestCase {
	public:
	void run_tests();
};
