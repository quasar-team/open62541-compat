# LICENSE:
# Copyright (c) 2016, Piotr Nikiel, CERN
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS 
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Authors:
# Piotr Nikiel <piotr@nikiel.info>
# Ben Farnham <firstNm.secondNm@cern.ch>
project(open62541-compat CXX)
cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0012 NEW)

option(STANDALONE_BUILD "Build it as a stand-alone library instead of for Quasar" OFF )
option(STANDALONE_BUILD_SHARED "When building in stand-alone, build shared library rather than static library" OFF)
message(STATUS "STANDALONE_BUILD [${STANDALONE_BUILD}] STANDALONE_BUILD_SHARED [${STANDALONE_BUILD_SHARED}]")

function ( clone_open62541 )
  SET (OPEN62541_VERSION "0.2")
  message(STATUS "cloning open62541 from github. *NOTE* cloning tag [${OPEN62541_VERSION}]")
  execute_process(COMMAND git clone -b ${OPEN62541_VERSION} https://github.com/open62541/open62541.git WORKING_DIRECTORY ${PROJECT_BINARY_DIR} )
  SET( UA_ENABLE_AMALGAMATION ON )
  SET( UA_ENABLE_METHODCALLS ON )
  message(STATUS "open62541 cloned")    
endfunction()

function ( build_open62541 )
  message(STATUS "generating platform specific build for open62541 library for [${CMAKE_GENERATOR}]")
  execute_process( COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" . WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/open62541 )
  message(STATUS "executing platform specific build for open62541 library")
  execute_process( COMMAND "${CMAKE_COMMAND}" --build . WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/open62541 )
endfunction()

function ( clone_LogIt )
  SET (LOGIT_VERSION "stand-alone-LogIt-with-strings-V3") #change to master post-merge
  message(STATUS "cloning LogIt from github. *NOTE* cloning version [${LOGIT_VERSION}]")
  execute_process(COMMAND git clone -b ${LOGIT_VERSION} https://github.com/quasar-team/LogIt.git WORKING_DIRECTORY ${PROJECT_BINARY_DIR}  )
  set( LOGIT_HAS_UATRACE FALSE )
  message(STATUS "LogIt cloned from LogIt repo")
endfunction()

SET( SRCS
  src/nodemanagerbase.cpp
  src/open62541_compat.cpp
  src/uabytestring.cpp
  src/uastring.cpp
  src/uavariant.cpp
  src/uadatavariablecache.cpp
  src/statuscode.cpp
  src/uanodeid.cpp
  src/uadatavalue.cpp
  src/uadatetime.cpp
  src/uaclient/uasession.cpp
)

clone_open62541()
build_open62541()
SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ".lib")
find_library(
  OPEN62541_SDK_LIB NAMES open62541 
  PATHS ${PROJECT_BINARY_DIR}/open62541 ${PROJECT_BINARY_DIR}/open62541/build ${PROJECT_BINARY_DIR}/open62541/Release/ ${PROJECT_BINARY_DIR}/open62541/Debug/ NO_DEFAULT_PATH
)
message(STATUS "OPEN62541_SDK_LIB [${OPEN62541_SDK_LIB}]")

if(NOT STANDALONE_BUILD)
  add_library ( open62541-compat OBJECT ${SRCS} )
else()
  #
  # Decide on open62541-compat build output (shared/static)
  #
  if (STANDALONE_BUILD_SHARED)
  	 set(OPEN62541_COMPAT_LIB_FORMAT SHARED)
  else()
     set(OPEN62541_COMPAT_LIB_FORMAT STATIC)
  endif()

  add_definitions( -std=c++0x )
  include_directories( include )
  include_directories( ${PROJECT_BINARY_DIR}/open62541 )

  #
  # Load stand-alone build toolchain file (used for defining build-specific boost (BOOST_LIBS), amongst other things)
  #
  if(DEFINED CMAKE_TOOLCHAIN_FILE)
    message("CMAKE_TOOLCHAIN_FILE is defined -- including [[${CMAKE_TOOLCHAIN_FILE}]]")
    include(${CMAKE_TOOLCHAIN_FILE})    
  endif()
  if( NOT DEFINED BOOST_LIBS)
    message(FATAL_ERROR "Required variable BOOST_LIBS has not been defined. You must define a toolchain file describing (as a minimum) how boost headers/libs are resolved in the target build environment")
  endif() 
  message(STATUS "Using boost libraries: BOOST_LIBS [${BOOST_LIBS}]")  
  
  #
  # How does the stand-alone open62541-compat build want to use LogIt? Possible options:
  # - Source
  # - External shared
  # - External static.
  #
  set(LOGIT_BUILD_OPTION "LOGIT_AS_INT_SRC" CACHE STRING "LogIt is a mandatory dependency of open62541-compat. Inclusion options LOGIT_AS_INT_SRC, LOGIT_AS_EXT_SHARED, LOGIT_AS_EXT_STATIC")
  set_property(CACHE LOGIT_BUILD_OPTION PROPERTY STRINGS LOGIT_AS_INT_SRC LOGIT_AS_EXT_SHARED LOGIT_AS_EXT_STATIC)
  message(STATUS "LogIt build option LOGIT_BUILD_OPTION [${LOGIT_BUILD_OPTION}]")

  #
  # Resolve LogIt library
  #
  set(LOGIT_EXT_LIB_DIR "UNINIALIZED" CACHE PATH "Path to the directory containing the LogIt shared/static library binary file. Use absolute path, or relative to [${PROJECT_SOURCE_DIR}/]")
  message(STATUS "Using LogIt build option [${LOGIT_BUILD_OPTION}]")
  if("${LOGIT_BUILD_OPTION}" STREQUAL "LOGIT_AS_INT_SRC")
  	clone_LogIt()
    add_subdirectory( ${PROJECT_SOURCE_DIR}/LogIt )
    message(STATUS "LogIt added as compiled object code from sub-directory LogIt")
    add_library( open62541-compat ${OPEN62541_COMPAT_LIB_FORMAT} ${SRCS} $<TARGET_OBJECTS:LogIt>)
  else()
  	add_library( open62541-compat ${OPEN62541_COMPAT_LIB_FORMAT} ${SRCS} )
    if("${LOGIT_BUILD_OPTION}" STREQUAL "LOGIT_AS_EXT_SHARED")
      SET(CMAKE_FIND_LIBRARY_SUFFIXES ".so" ".lib") # windows looks for lib file to link to target dll: lib contains dll exports, symbols etc (see LogIt.h SHARED_LIB_EXPORT_DEFN)
      find_library( LOGITLIB NAMES LogIt PATHS ${LOGIT_EXT_LIB_DIR} NO_DEFAULT_PATH )          
    elseif("${LOGIT_BUILD_OPTION}" STREQUAL "LOGIT_AS_EXT_STATIC")
      SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ".lib")
      find_library( LOGITLIB NAMES LogIt PATHS ${LOGIT_EXT_LIB_DIR} NO_DEFAULT_PATH )
    else()
      message(FATAL_ERROR "Invalid command given as to how to use LogIt in the open62541-compat library, see documentaton for property [LOGIT_BUILD_OPTION]")
    endif()
    target_link_libraries( open62541-compat ${LOGITLIB} )
    message(STATUS "LogIt added as external library dependency LOGITLIB [${LOGITLIB}]")
  endif()

  #
  # Resolve LogIt include
  #
  set(LOGIT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/LogIt/include CACHE PATH "Path to LogIt include directory. If building with LogIt as external shared/static library this must be specified using -DLOGIT_INCLUDE_DIR=path. Path can be absolute or relative to [${PROJECT_SOURCE_DIR}/]")
  if( NOT EXISTS ${LOGIT_INCLUDE_DIR} )
    message(FATAL_ERROR "Cannot build with LogIt. No LogIt include directory found at [${LOGIT_INCLUDE_DIR}]. If building with LogIt as external shared/static library this must be specified using -DLOGIT_INCLUDE_DIR=path. Path can be absolute or relative to [${PROJECT_SOURCE_DIR}/]")
  endif()
  message(STATUS "Using LogIt include directory [${LOGIT_INCLUDE_DIR}]")
  include_directories( ${LOGIT_INCLUDE_DIR} )

  #
  # Set required libs, note windows build requires winsock.
  #
  target_link_libraries( open62541-compat ${BOOST_LIBS} ${OPEN62541_SDK_LIB})
  if(WIN32)
    target_link_libraries( open62541-compat ws2_32 )
  endif()  

endif()
