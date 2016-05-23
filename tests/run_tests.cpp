//
//	Test runner for my unit/acceptance tests.
//	Also measures latency of certain algorithms
//
//	Runs all the tests under test_classes/ folder
//
#include "test_classes/testcase.hpp"
#include "test_classes/pupil_tests.hpp"
#include <iostream>
using namespace std;

typedef int (*TestFunction) (TestCase tc);

int main(int argc, char const *argv[]) {
	//run all tests
	TestCase *t;
	PupilTests p;
	t = &p;
	t->run_tests();
	return 0;
}
