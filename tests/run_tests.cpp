//
//	Test runner for my unit/acceptance tests.
//	Also measures latency of certain algorithms
//
//	Runs all the tests under test_classes/ folder
//
#include "test_classes/testcase.hpp"
#include "test_classes/pupil_tests.hpp"
#include "test_classes/center_tests.hpp"
#include <iostream>
using namespace std;

typedef int (*TestFunction) (TestCase tc);

int main(int argc, char const *argv[]) {
	//run all tests of all types
	vector<TestCase*> all_tests;
	all_tests.push_back(new PupilTests());
	all_tests.push_back(new CenterTests());
	//free the allocated memory
	for( vector<TestCase*>::iterator i = all_tests.begin(); i != all_tests.end(); ++i ){
		(*i)->run_tests();
		delete *i;
	}
	all_tests.clear();
	return 0;
}
