cmake_minimum_required(VERSION 3.8)
project(TransportCatalogEngine)

set(CMAKE_CXX_STANDARD_REQUIRED 17)

set (
	TRANSPORT_CATALOG_ENGINE_HEADER_FILES
		transport_catalog.h
)
set (
	TRANSPORT_CATALOG_ENGINE_SOURCE_FILES
		tr_catalog_engine.cpp
		tr_catalog_render.cpp
		tr_catalog_layer_renderers.cpp
)

add_library(
	TransportCatalogEngine STATIC
		${TRANSPORT_CATALOG_ENGINE_HEADER_FILES}
		${TRANSPORT_CATALOG_ENGINE_SOURCE_FILES}
)
target_include_directories(
	TransportCatalogEngine PUBLIC 
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(TransportCatalogEngine Algorithm)
target_link_libraries(TransportCatalogEngine Geographic)
target_link_libraries(TransportCatalogEngine Navigator)
target_link_libraries(TransportCatalogEngine Svg)
target_link_libraries(TransportCatalogEngine Render)



