cmake_minimum_required (VERSION 3.0.0)
project(DisplayImage)

# INCLUDES
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
include_directories(${JSONCPP_INCLUDE_DIRS})

# EYE LIBRARY
add_library(eyelib eyelib.cpp eyelib.hpp)

# MAIN EXECUTABLE
add_executable(camera_capture camera_capture.cpp)
target_link_libraries(camera_capture
                      eyelib
                      ${OpenCV_LIBS})

# UNIT TESTS
add_executable(run_tests tests/run_tests.cpp)
target_sources(run_tests PRIVATE
               tests/test_classes/testcase.cpp
               tests/test_classes/center_tests.cpp
               tests/test_classes/pupil_tests.cpp)
target_link_libraries(run_tests
                      eyelib
                      ${JSONCPP_LIBRARIES}
                      ${OpenCV_LIBS})
