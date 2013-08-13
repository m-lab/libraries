set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL TRUE)
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

find_package(PythonLibs)
include_directories(${PYTHON_INCLUDE_PATH})

set(CMAKE_SWIG_FLAGS "-c++")
set(CMAKE_SWIG_FLAGS "-DOS_LINUX")

link_directories(
    ${PROJECT_SOURCE_DIR}/lib
    ${JSONCPP_ROOT}/lib)
set(JSONCPP_LIB json-cpp)

set_source_files_properties(mlab.i PROPERTIES CPLUSPLUS ON)
#TODO: figure out why the variable isn't set correctly
#set_source_files_properties(${swig_generated_file_fullname} PROPERTIES COMPILE_FLAGS "-Wno-error")
set_source_files_properties(mlabPYTHON_wrap.cxx PROPERTIES COMPILE_FLAGS "-Wno-unused-but-set-variable")

swig_add_module(mlabpy python mlab.i ${SRC_FILES})
swig_link_libraries(mlabpy ${PYTHON_LIBRARIES} ${JSONCPP_LIB})
