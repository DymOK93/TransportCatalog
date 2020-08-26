cmake_minimum_required(VERSION 3.8)
project(Algorithm)

set(CMAKE_CXX_STANDARD_REQUIRED 17)

set (
	ALGORITHM_HEADER_FILES
		numbers_smart_comparison.h
		string_algorithms.h
)

#Header-only
add_library(Algorithm INTERFACE)
target_include_directories(
	Algorithm INTERFACE 
		${CMAKE_CURRENT_SOURCE_DIR}
)




