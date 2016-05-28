all:
	g++ -std=c++11 -c ../eyelib.cpp
	g++ -std=c++11 -c test_classes/testcase.cpp
	g++ -std=c++11 -c test_classes/pupil_tests.cpp
	g++ -std=c++11 -c test_classes/center_tests.cpp
	g++ -std=c++11 -c run_tests.cpp
	g++ -std=c++11 -o run_tests run_tests.o testcase.o pupil_tests.o center_tests.o eyelib.o -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_objdetect -ljsoncpp
	rm *.o
