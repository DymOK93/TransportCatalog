cmake_minimum_required (VERSION 3.8)
project ("Execution")

set(CMAKE_CXX_STANDARD_REQUIRED 17)

set(
	EXECUTION_HEADER_FILES
		execution.h
)
set(
	EXECUTION_SOURCE_FILES
		execution.cpp
)

add_library(
	Execution STATIC
		${EXECUTION_HEADER_FILES}
		${EXECUTION_SOURCE_FILES}
)
target_include_directories(
	Execution PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)