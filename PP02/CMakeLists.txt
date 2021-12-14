cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
set(PROJECT_NAME "PP02")
project(${PROJECT_NAME})

# force static runtime libraries for msvc builds
if(MSVC)
  set(variables CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_MINSIZEREL)
  foreach(variable ${variables})
  if(${variable} MATCHES "/MD")
    string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
  endif()
  endforeach()
endif()

# set c++17
set (CMAKE_CXX_STANDARD 17)
set( CMAKE_CXX_STANDARD_REQUIRED ON )

# compile everything position independent (even static libraries)
set( CMAKE_POSITION_INDEPENDENT_CODE TRUE )

# set verbose compile options
#set( CMAKE_VERBOSE_MAKEFILE ON )

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /PROFILE")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

# assuming x86-64 Microarchitecture Feature Level >= x86-64-v2
if((CMAKE_C++_COMPILER_ID EQUAL "GNU") AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "11.0"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=x86-64-v2")
elseif(CMAKE_C++_COMPILER_ID EQUAL "Clang" AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "12.0"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=x86-64-v2")
elseif(NOT MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=nehalem")
endif()

# add OpenMP
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

# add threads lib
find_package (Threads REQUIRED)

set(PROJECT_SOURCES  
  CommonDef.h
  ppCommon.h
  xFile.h
  xPSNR.h  xPSNR.cpp
  xPic.h   xPic.cpp
  xPixelOps.h
  xPixelOpsSTD.h  xPixelOpsSTD.cpp
  xSeq.h  xSeq.cpp
  xVec4.h
  PP02.cpp)

source_group("Source Files" FILES ${PROJECT_SOURCES})

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_link_libraries (${PROJECT_NAME} PRIVATE Threads::Threads)


