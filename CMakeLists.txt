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

cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0012 NEW)

option(STANDALONE_BUILD "Build it as a stand-alone library instead of for Quasar" OFF )
option(STANDALONE_BUILD_SHARED "When building in stand-alone, build shared library rather than static library" OFF)

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


if(NOT STANDALONE_BUILD)
  add_library ( open62541-compat OBJECT ${SRCS} ) 
else()
  add_definitions( -std=c++0x )
  include_directories( include )
  include_directories( open62541 )
  
  #
  # BOOST BUILD HACKS! Soft code me somehow
  #
  include_directories( "C:/3rdPartySoftware/boost_mapped_namespace_builder/work/MAPPED_NAMESPACE_INSTALL/include" )
  SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ".lib")
  find_library(BOOST_LIB_DATE_TIME libboost_1_66_0_date_time-vc141-mt-x64-1_66 "C:/3rdPartySoftware/boost_mapped_namespace_builder/work/MAPPED_NAMESPACE_INSTALL/lib/" NO_DEFAULT_PATH)
  message(STATUS "BOOST_LIB_DATE_TIME [${BOOST_LIB_DATE_TIME}]")

  #
  # How does the stand-alone open62541-compat build want to use LogIt? Possible options:
  # - Source
  # - External shared
  # - External static.
  #
  set(LOGIT_BUILD_OPTION "LOGIT_AS_INT_SRC" CACHE STRING "\
    LogIt is a mandatory dependency of the open62541-compat, but, there are options as to how it's included:\
    \n LOGIT_AS_INT_SRC: The LogIt source code is retrieved and built straight into open62541-compat as part of the library\
    \n LOGIT_AS_EXT_SHARED: LogIt is an external *shared* library, obviously this only makes sense if you are building open62541-compat as a shared library\
    \n LOGIT_AS_EXT_STATIC: LogIt is an external *static* library")
  set_property(CACHE LOGIT_BUILD_OPTION PROPERTY STRINGS LOGIT_AS_INT_SRC LOGIT_AS_EXT_SHARED LOGIT_AS_EXT_STATIC)
  message(STATUS "LogIt build option LOGIT_BUILD_OPTION [${LOGIT_BUILD_OPTION}]")

  #
  # Resolve LogIt include
  #
  set(LOGIT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/LogIt/include CACHE PATH "Path to the LogIt include directory. Use absolute path, or relative to [${PROJECT_SOURCE_DIR}/]")
  if( NOT EXISTS ${LOGIT_INCLUDE_DIR} )
    message(FATAL_ERROR "Cannot build with LogIt. No LogIt include directory found at [${LOGIT_INCLUDE_DIR}]. If building with LogIt as an external shared/static library - please specify using [-DLOGIT_INCLUDE_DIR=path_to_external_include_dir]")
  endif()
  message(STATUS "Using LogIt include directory [${LOGIT_INCLUDE_DIR}]")
  include_directories( ${LOGIT_INCLUDE_DIR} )

  #
  # Resolve LogIt library
  #
  set(LOGIT_LIB_DIR "UNINIALIZED" CACHE PATH "Path to the directory containing the LogIt shared/static library binary file. Use absolute path, or relative to [${PROJECT_SOURCE_DIR}/]")
  if("${LOGIT_BUILD_OPTION}" STREQUAL "LOGIT_AS_INT_SRC")
    message(STATUS "Using LogIt from source code, the code will be retrieved and built into the open62541-compat library")
    execute_process(COMMAND git clone https://github.com/quasar-team/LogIt.git ${PROJECT_SOURCE_DIR}/LogIt )
    set( LOGIT_HAS_UATRACE FALSE )
    add_subdirectory( ${PROJECT_SOURCE_DIR}/LogIt )
    #add_library(libLogIt OBJECT $<TARGET_OBJECTS:LogIt>) !! doesn't work - cannot use object library in another object library
  elseif("${LOGIT_BUILD_OPTION}" STREQUAL "LOGIT_AS_EXT_SHARED")
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ".so" ".lib") # windows looks for lib file to link to target dll: lib contains dll exports, symbols etc (see LogIt.h SHARED_LIB_EXPORT_DEFN)
    find_library(LOGITLIB LogIt ${LOGIT_LIB_DIR} NO_DEFAULT_PATH)
    message(STATUS "LOGITLIB [${LOGITLIB}]")
  elseif("${LOGIT_BUILD_OPTION}" STREQUAL "LOGIT_AS_EXT_STATIC")
    SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ".lib")
    find_library(LOGITLIB LogIt ${LOGIT_LIB_DIR} NO_DEFAULT_PATH)
    message(STATUS "LOGITLIB [${LOGITLIB}]")
  else()
    message(FATAL_ERROR "No command given as to how to use LogIt in the open62541-compat library, see documentaton for property [LOGIT_BUILD_OPTION]")
  endif()

  #
  # Resolve the open62541 library
  #
  SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ".lib")
  find_library(OPEN62541_LIB open62541 "open62541/Release/" NO_DEFAULT_PATH)
  message(STATUS "OPEN62541_LIB [${OPEN62541_LIB}]")

  #
  # Decide on the output mode for open62541-compat build (shared/static)
  #
  if (STANDALONE_BUILD_SHARED)
     add_library( open62541-compat SHARED ${SRCS} )
  else()
     add_library( open62541-compat STATIC ${SRCS} )
  endif()

  set( REQUIRED_LIBS "${LOGITLIB}" "${BOOST_LIB_DATE_TIME}" "${OPEN62541_LIB}")
  if(WIN32)
    set (REQUIRED_LIBS ${REQUIRED_LIBS} ws2_32)
  endif()
  message(STATUS "REQUIRED_LIBS [${REQUIRED_LIBS}]")
  
  target_link_libraries( open62541-compat  ${REQUIRED_LIBS})

endif()