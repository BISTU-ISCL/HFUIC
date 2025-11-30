cmake_minimum_required(VERSION 3.16)
project(HFUIC LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets Charts REQUIRED)
# Fallback for environments that still ship Qt5
if(NOT Qt6_FOUND)
  find_package(Qt5 COMPONENTS Widgets Charts REQUIRED)
  set(QT_PREFIX Qt5)
else()
  set(QT_PREFIX Qt6)
endif()

set(SOURCES
    src/main.cpp
    src/RealTimeDisplayWindow.cpp
    src/PostMatchAnalysisWindow.cpp
)

add_executable(HFUIC ${SOURCES})

if(QT_PREFIX STREQUAL "Qt6")
  target_link_libraries(HFUIC PRIVATE Qt6::Widgets Qt6::Charts)
else()
  target_link_libraries(HFUIC PRIVATE Qt5::Widgets Qt5::Charts)
endif()

target_include_directories(HFUIC PRIVATE src)
