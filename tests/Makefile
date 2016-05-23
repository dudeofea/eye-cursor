all:
	g++ -std=c++0x -c ../eyelib.cpp
	g++ -std=c++0x -c test_classes/testcase.cpp
	g++ -std=c++0x -c test_classes/pupil_tests.cpp
	g++ -std=c++0x -c run_tests.cpp
	g++ -std=c++0x -o run_tests run_tests.o testcase.o pupil_tests.o eyelib.o -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_objdetect
	rm *.o
