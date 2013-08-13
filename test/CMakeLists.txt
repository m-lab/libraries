# For log.h
set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL TRUE)
include_directories(${PROJECT_SOURCE_DIR}/src)

link_directories(
  ${PROJECT_SOURCE_DIR}/lib
  ${JSONCPP_ROOT}/lib)
set(JSONCPP_LIB json-cpp)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(PTHREADS_LIB pthread)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(PTHREADS_LIB pthread)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  link_directories(
      ${PTHREADS-W32_ROOT}/lib/x86)
  set(PTHREADS_LIB pthreadVCE2)
  set(WINSOCK_LIB Ws2_32 wsock32)
endif()

file(GLOB_RECURSE TEST_SRC_FILES *_test.cc)
add_executable(mlab_test ${TEST_SRC_FILES})

target_link_libraries(mlab_test
	gtest_main
	mlab
	${JSONCPP_LIB}
	${PTHREADS_LIB}
	${WINSOCK_LIB})

add_executable(mlab_c_test mlab_c_test.c)
target_link_libraries(mlab_c_test mlabc mlab ${JSONCPP_LIB})
add_executable(raw_socket_test test_raw_socket.cc test_raw_socket_send_recv.cc)
target_link_libraries(raw_socket_test mlab gtest_main) 
