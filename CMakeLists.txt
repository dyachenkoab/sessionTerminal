cmake_minimum_required(VERSION 3.5)

project(sessionTerminal LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE "Release")

#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined")

find_package(Qt5 COMPONENTS Core DBus Widgets REQUIRED)

set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        src/edit.h
        src/edit.cpp
        src/dbushandler.cpp
        src/dbushandler.h
        src/Structs.h
        res/Resources.qrc
)


add_executable( ${PROJECT_NAME} ${PROJECT_SOURCES} )

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  Qt5::Core
  Qt5::DBus
  Qt5::Widgets
  )

install( TARGETS ${PROJECT_NAME} DESTINATION "/usr/bin" )

SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Diachenko Andrey")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "qtbase5-dev (>=5.9), qtbase5-dev-tools (>=5.9)")
INCLUDE(CPack)

