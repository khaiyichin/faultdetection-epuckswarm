cmake_minimum_required(VERSION 2.8.12)
project(faultdetection-epuckswarm)

###################################
##### 03/28/23 edit (Khai Yi) #####
# - You will need to have ARGoS compiled with QT to use this repo properly;
#   the `include(ARGoSCheckQTOpenGL)` check below doesn't work properly so I've commented it out
# - The code has some fatal bugs if you build it in Release mode, so I've set it to always build in Debug mode.
#   I managed to fix the first that I found (unsigned type function but returns nothing) but it 
#   wasn't worth my time trying to solve their issues.
# - I commented a bunch of their `find_package`s because they are ARGoS dependencies, so if you can find ARGoS,
#   you have them satisfied. You might ask, if they're ARGoS dependencies, then leaving them as is should be fine.
#   That's correct, but for some reason the CMake files they're looking for isn't readily available; you can
#   uncomment them and try. It might work for you.
###################################
set(ARGOS_COMPILE_QTOPENGL TRUE)
set(CMAKE_BUILD_TYPE "Debug")

if(APPLE)
  # Add address sanitizer support for CLang
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address -fno-optimize-sibling-calls -fno-omit-frame-pointer")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fno-optimize-sibling-calls -fno-omit-frame-pointer")
  set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} -fsanitize=address")
  set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} -fsanitize=address")
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} -fsanitize=address")
endif(APPLE)

# Deactivate RPATH for MacOSX
set(CMAKE_MACOSX_RPATH 0)

# Set the path where CMake will find additional scripts
set(CMAKE_MODULE_PATH $ENV{CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
message("CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

# Find the ARGoS package, make sure to save the ARGoS prefix
find_package(ARGoS REQUIRED)
include_directories(${CMAKE_SOURCE_DIR} ${ARGOS_INCLUDE_DIRS})
link_directories(${ARGOS_LIBRARY_DIR}) # include the */lib/argos3 directory to find the ARGoS libraries
link_libraries(${ARGOS_LDFLAGS})

# Check whether all the necessary libs have been installed to compile the
# code that depends on Qt and OpenGL
# include(ARGoSCheckQTOpenGL)

# Find Lua
#find_package(Lua52 REQUIRED)

# Find the GSL package
#find_package(GSL REQUIRED)

# Set ARGoS include dir
#include_directories(${CMAKE_SOURCE_DIR} ${ARGOS_INCLUDE_DIRS} ${GSL_INCLUDE_DIR} ${LUA_INCLUDE_DIR})

# You need to replace the libargos3plugin_simulator_epuck.so of argos simulator with the library of the same name of the argos-epuck plugin
  
# Set ARGoS link dir
#link_directories(${ARGOS_LIBRARY_DIRS})

# Descend into the controllers directory
add_subdirectory(controllers)

# Descend into the loop_functions directory
add_subdirectory(loop_functions)
