cmake_minimum_required( VERSION 3.1 )
set( CMAKE_CXX_STANDARD 11 )

# Create Project
project( Soccer )

# Add directory to search for modules
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} /usr/local/lib/cmake )

# Find libraries
find_package( OpenCV REQUIRED )
find_package( raspicam REQUIRED )

# Set executable
add_executable( soccer main.cpp area.cpp camera.cpp ball.cpp goal.cpp com.cpp )
add_executable( calibrate calibrate.cpp area.cpp )

# Linking libraries
target_link_libraries( soccer ${OpenCV_LIBS} )
target_link_libraries( soccer ${raspicam_CV_LIBS} )
target_link_libraries( soccer "bluetooth" )
target_link_libraries( soccer "wiringPi" )
target_link_libraries( soccer "ncurses" )
target_link_libraries( soccer "pthread" )

target_link_libraries( calibrate ${OpenCV_LIBS} )
target_link_libraries( calibrate ${raspicam_CV_LIBS} )
