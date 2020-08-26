cmake_minimum_required (VERSION 3.8)
project ("Json")

set(CMAKE_CXX_STANDARD_REQUIRED 17)

set(
	JSON_HEADER_FILES
		json.h
		json_number.h
)
set(
	JSON_SOURCE_FILES
		json.cpp
		json_number.cpp
)

add_library(
	Json STATIC
		${JSON_HEADER_FILES}
		${JSON_SOURCE_FILES}
)
target_include_directories(
	Json PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

#Обработка строк и сравнение чисел с плавающей точкой
target_link_libraries(Json Algorithm)