cmake_minimum_required(VERSION 2.8)

project(RWHEC_Jul2019_Exp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -std=c++11")

find_package(Ceres REQUIRED)
include_directories(${CERES_INCLUDE_DIRS} /usr/local /usr/local/eigen3 /usr/local/include/opencv4)
link_directories( )


add_executable(RWHEC_Jul2019_Exp
CostFunctions.hpp
CostFunctions.cpp
Calibration2.hpp
Calibration2.cpp
StringFunctions.hpp
DirectoryFunctions.hpp
DirectoryFunctions.cpp
RWHEC_Jul2019_Exp.hpp
RWHEC_Jul2019_Exp.cpp
)

target_link_libraries(RWHEC_Jul2019_Exp ${CERES_LIBRARIES} opencv_core opencv_highgui opencv_imgproc opencv_calib3d opencv_imgcodecs)

