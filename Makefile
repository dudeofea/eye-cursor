all:
	g++ -o camera_capture camera_capture.cpp -lopencv_core -lopencv_highgui -lopencv_objdetect -lopencv_videoio -lopencv_imgproc
