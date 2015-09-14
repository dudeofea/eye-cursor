all:
	g++ -o camera_capture camera_capture.cpp -lopencv_core -lopencv_videoio -lopencv_imgproc -lopencv_highgui -lopencv_objdetect