cmake_minimum_required(VERSION 3.4.0)

#=========================================================
# Get cmake file to add external *.cmake scripts
#=========================================================
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(CXLStressTester)


#=========================================================
# Add boost library to project
#=========================================================
#include(InstallBoost)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set (THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_BUILD_TYPE Debug)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set (ASSETS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets/")
set(CMAKE_CXX_FLAGS " -g -pthread")
set(CMAKE_LINK " -lstdc++fs -lpthread")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "8.0.0")
    link_libraries(-lsdc++fs)
  endif()
endif()

#=========================================================
# CPack
#=========================================================
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/deploy)
set(CPACK_GENERATOR "TGZ")
include (CPack)

find_package(Threads REQUIRED)

#=========================================================
# Get src files for cxl hammer
#=========================================================
add_subdirectory(src)
