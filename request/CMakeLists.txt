cmake_minimum_required(VERSION 3.8)
project(Request)

set(CMAKE_CXX_STANDARD_REQUIRED 17)

set (
	REQUEST_HEADER_FILES
		request.h
		synchronized.h
)
set (
	REQUEST_SOURCE_FILES
		request.cpp
)

add_library(
	Request STATIC
		${REQUEST_HEADER_FILES}
		${REQUEST_SOURCE_FILES}
)
target_include_directories(
	Request PUBLIC 
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Request Json)
target_link_libraries(Request TransportCatalogEngine)




