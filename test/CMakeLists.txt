cmake_minimum_required(VERSION 3.10)
project(open62541-compat-Test)
find_package(Threads REQUIRED)

message( STATUS "building unit tests, a wise choice. Remember to execute the tests too!" )

function ( clone_googletest GOOGLETEST_VERSION)
  if(EXISTS "${PROJECT_BINARY_DIR}/googletest/CMakeLists.txt")
    message(STATUS "googletest already available in ${PROJECT_BINARY_DIR}/googletest")
    return()
  endif()
  message(STATUS "cloning googletest from github. *NOTE* cloning [${GOOGLETEST_VERSION}]")
  execute_process(
    COMMAND git clone -b ${GOOGLETEST_VERSION} https://github.com/google/googletest.git
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    RESULT_VARIABLE GOOGLETEST_CLONE_RESULT
  )
  if(NOT GOOGLETEST_CLONE_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to clone googletest release [${GOOGLETEST_VERSION}]")
  endif()
  message(STATUS "googletest cloned")
endfunction()

set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

file(GLOB OPEN62541_COMPAT_MODULE_TEST_SRCS src/*.cpp)
include_directories( include )

clone_googletest("release-1.10.0")
set( BUILD_GTEST ON CACHE BOOL "We do want to build gtest")
set( BUILD_GMOCK OFF CACHE BOOL "we do not want to build gmock")
set( gtest_force_shared_crt ON CACHE BOOL "use shared CRT" FORCE)
add_subdirectory( ${PROJECT_BINARY_DIR}/googletest ${PROJECT_BINARY_DIR}/googletest EXCLUDE_FROM_ALL)

add_executable( open62541-compat-Test ${OPEN62541_COMPAT_MODULE_TEST_SRCS} )

message(STATUS "file [${CMAKE_CURRENT_LIST_FILE}]: Using BOOST_LIBS [${BOOST_LIBS}]")
message(STATUS "file [${CMAKE_CURRENT_LIST_FILE}]: Using LogIt library: LOGIT_LIB [${LOGIT_LIB}]")

target_link_libraries (open62541-compat-Test
	open62541-compat
	${BOOST_LIBS}
	${CMAKE_THREAD_LIBS_INIT}
	${LOGIT_LIB}
	gtest
	-ldl
)

add_dependencies (open62541-compat-Test gtest)
