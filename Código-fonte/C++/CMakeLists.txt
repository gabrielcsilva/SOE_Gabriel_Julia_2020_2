cmake_minimum_required(VERSION 2.8)
project( carCounter )
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
add_executable( carCounter carCounter.cpp )
target_link_libraries( carCounter ${OpenCV_LIBS} )
 

find_package (CURL)
if (CURL_FOUND)
	include_directories(${CURL_INCLUDE_DIRS})
	target_link_libraries(carCounter ${CURL_LIBRARIES})
else()
	MESSAGE(FATAL_ERROR "LibCurl is not found in your system.")
endif (CURL_FOUND)

target_link_libraries(carCounter ${LIBS})
