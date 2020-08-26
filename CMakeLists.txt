cmake_minimum_required (VERSION 3.8)
#Часть K (Coursera C++ Black Belt)
project ("TransportCatalog")

set(CMAKE_CXX_STANDARD 20)

#Директивы препроцессора (#define), переключающие режимы выполнения
add_compile_definitions(
	MULTITHREADING
	RENDER
	#SVG_DEBUG
	#WINDOWS_DEBUG
)


add_executable (
	TransportCatalog
		utils.h
		utils.cpp
		main.cpp
)

#Параллельное исполнение (библиотека необходима в связи с отсутствием поддержки <execution> на Cousera)
add_subdirectory(execution)
target_link_libraries(TransportCatalog Execution)

#Географические представления и вычисления
add_subdirectory(geographic)
target_link_libraries(TransportCatalog Geographic)

#Обработка строк и сравнение чисел с плавающей точкой
add_subdirectory(algorithm)
target_link_libraries(TransportCatalog Algorithm)

#Работа с JSON-документами
add_subdirectory(json)
target_link_libraries(TransportCatalog Json)

#Отрисовка формате SVG
add_subdirectory(svg)
target_link_libraries(TransportCatalog Svg)

#Инструменты для отрисовки карты
add_subdirectory(render)
target_link_libraries(TransportCatalog Render)

#Система навигации на основе направленного взвешенного графа
add_subdirectory(navigator)
target_link_libraries(TransportCatalog Navigator)

#Движок справочника и система отрисовки
add_subdirectory(tr_catalog)
target_link_libraries(TransportCatalog TransportCatalogEngine)

#Система запросов
add_subdirectory(request)
target_link_libraries(TransportCatalog Request)






