#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <cmath>
#include <ctime>
#include <fstream>
#include <json/json.h>
#include <json/value.h>
#include "testcase.hpp"

using namespace std;

class PupilTests: public TestCase {
	public:
	void run_tests();
};
