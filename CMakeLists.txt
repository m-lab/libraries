cmake_minimum_required (VERSION 2.6)

if(DEFINED CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Choose the build type.")
else()
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type.")
endif()

project (mlab)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

# Set up directories
set(JSONCPP_ROOT ${PROJECT_SOURCE_DIR}/third_party/json-cpp)
set(GTEST_ROOT ${PROJECT_SOURCE_DIR}/third_party/gtest-1.6.0)
set(PTHREADS-W32_ROOT ${PROJECT_SOURCE_DIR}/third_party/pthreads-w32-2-9-1-release/Pre-built.2)

# Set OS
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(CMAKE_CXX_FLAGS "-Wall -Werror -fPIC -fno-exceptions")
	set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -fno-strict-aliasing")

	add_definitions(-DOS_MACOSX)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set(CMAKE_CXX_FLAGS "-Wall -Werror -fPIC -fno-exceptions -fno-rtti")
	set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -fno-strict-aliasing")

	add_definitions(-DOS_LINUX)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	# TODO(dominich): Find a way to really disable exceptions.
	set(CMAKE_CXX_FLAGS "/W2 /WX /EHsc")
	set(CMAKE_CXX_FLAGS_DEBUG "/Od -DDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "/O2 -DNDEBUG")

	add_definitions(-DOS_WINDOWS)

	include_directories(${PTHREADS-W32_ROOT}/include)
endif()

# Set CPU
if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86")
	add_definitions(-DARCH_X86)
endif()

include_directories(
	${PROJECT_SOURCE_DIR}/include
	${JSONCPP_ROOT}/include
	${GTEST_ROOT}/include)

file(GLOB_RECURSE SRC_FILES src/*.cc)

# Build the libraries
add_library(mlab STATIC ${SRC_FILES})
add_library(mlabc STATIC src/mlab.cc)

# Build supplemental targets
add_subdirectory(third_party/gtest-1.6.0)
add_subdirectory(third_party/json-cpp)
add_subdirectory(swig)
add_subdirectory(test)

add_custom_target(
	print_build_type
	ALL
	COMMAND	echo Building ${CMAKE_BUILD_TYPE})

add_dependencies(mlab print_build_type)
add_dependencies(mlabc print_build_type)
