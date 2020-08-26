cmake_minimum_required (VERSION 3.8)
project ("Geographic")

set(CMAKE_CXX_STANDARD_REQUIRED 17)

set(
	GEOGRAPHIC_HEADER_FILES
		range.h
		geographic.h
		earth.h
)
set(
	GEOGRAPHIC_SOURCE_FILES
		earth.cpp
)

add_library(
	Geographic STATIC
		${GEOGRAPHIC_HEADER_FILES}
		${GEOGRAPHIC_SOURCE_FILES}
)
target_include_directories(
	Geographic PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)