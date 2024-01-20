cmake_minimum_required(VERSION 3.5)
project(json)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -Werror")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(NO_EXCEPTIONS "Disable exceptions" OFF)
if(NO_EXCEPTIONS)
  add_definitions(-DNO_EXCEPTIONS=1)
endif()

option(BUILD_SHARED_LIBS "Build shared library" OFF)
if(BUILD_SHARED_LIBS)
  set(LIBRARY SHARED)
else()
  set(LIBRARY STATIC)
endif()

include_directories(src)

set(SOURCE_FILES
  src/json.cpp
)

add_library(${PROJECT_NAME} ${LIBRARY} ${SOURCE_FILES})

add_subdirectory(test)
