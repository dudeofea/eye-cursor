LIBS = -L/usr/local/lib -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_objdetect -lopencv_videoio

all:
	g++ -c eyelib.cpp camera_capture.cpp
	g++ -o camera_capture eyelib.o camera_capture.o ${LIBS}
