# LICENSE:
# Copyright (c) 2016, CERN
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
#
# Authors:
# Ben Farnham <firstNm.secondNm@cern.ch>
# Piotr Nikiel <piotr@nikiel.info>
# 
# Note: the following code was mostly contributed by Ben, in the master CMakeLists of open62541-compat
# Then moved by Piotr to this file for easier maintenance, further fixes by Piotr


function ( fetch_LogIt )
  SET (LOGIT_VERSION "v0.1.1") #change to master post-merge
  message(STATUS "fetching LogIt from github. *NOTE* fetching version [${LOGIT_VERSION}]")
  Open62541CompatFetchContent_Declare(
    LogIt
    GIT_REPOSITORY    https://github.com/quasar-team/LogIt.git
    GIT_TAG           ${LOGIT_VERSION}
    GIT_SHALLOW       "1"
    SOURCE_DIR	      ${PROJECT_BINARY_DIR}/LogIt
    BINARY_DIR	      ${PROJECT_BINARY_DIR}/LogIt
  )
  Open62541CompatFetchContent_Populate( LogIt )
  message(STATUS "LogIt fetched from LogIt repo")
endfunction()

function ( process_LogIt )
  # Parse LOGIT_BUILD_OPTION first as it determines almost any other behaviour that follows
  # Possible options:
  # - Source          --> fetches LogIt into ${PROJECT_BINARY_DIR}/LogIt
  # - External shared --> uses existing LogIt, its headers should be in ${LOGIT_INCLUDE_DIR}
  # - External static --> uses existing LogIt, its headers should be in ${LOGIT_INCLUDE_DIR}
  # If nothing coming from outside, make "LOGIT_AS_INT_SRC" the default
  set(LOGIT_BUILD_OPTION "LOGIT_AS_INT_SRC" CACHE STRING "LogIt is a mandatory dependency of open62541-compat. Inclusion options LOGIT_AS_INT_SRC, LOGIT_AS_EXT_SHARED, LOGIT_AS_EXT_STATIC")
  set(LOGIT_BUILD_OPTIONS_SUPPORTED "LOGIT_AS_INT_SRC" "LOGIT_AS_EXT_SHARED" "LOGIT_AS_EXT_STATIC")
  
  if (NOT "${LOGIT_BUILD_OPTION}" IN_LIST LOGIT_BUILD_OPTIONS_SUPPORTED)
    message(FATAL_ERROR "Given LOGIT_BUILD_OPTION is not supported. It must be one of: ${LOGIT_BUILD_OPTIONS_SUPPORTED}")
  endif()
  
  
  set_property(CACHE LOGIT_BUILD_OPTION PROPERTY STRINGS LOGIT_AS_INT_SRC LOGIT_AS_EXT_SHARED LOGIT_AS_EXT_STATIC)
  message(STATUS "LogIt build option LOGIT_BUILD_OPTION [${LOGIT_BUILD_OPTION}]")

  #
  # Fetch LogIt if needed
  #
  if("${LOGIT_BUILD_OPTION}" STREQUAL "LOGIT_AS_INT_SRC")
  	fetch_LogIt()
  endif()
  
  #
  # Resolve LogIt include
  #
  set(LOGIT_INCLUDE_DIR ${PROJECT_BINARY_DIR}/LogIt/include CACHE PATH "Path to LogIt include directory. If building with LogIt as external shared/static library this must be specified using -DLOGIT_INCLUDE_DIR=path. Path can be absolute or relative to [${PROJECT_BINARY_DIR}/]")
  if( NOT EXISTS ${LOGIT_INCLUDE_DIR} )
    message(FATAL_ERROR "Cannot build with LogIt. No LogIt include directory found at [${LOGIT_INCLUDE_DIR}]. If building with LogIt as external shared/static library this must be specified using -DLOGIT_INCLUDE_DIR=path. Path can be absolute or relative to [${PROJECT_BINARY_DIR}/]")
  endif()
  message(STATUS "Using LogIt include directory [${LOGIT_INCLUDE_DIR}]")
  include_directories( ${LOGIT_INCLUDE_DIR} )

  #
  # Resolve LogIt library
  #
  set(LOGIT_EXT_LIB_DIR "UNINIALIZED" CACHE PATH "Path to the directory containing the LogIt shared/static library binary file. Use absolute path, or relative to [${PROJECT_BINARY_DIR}/]")
  message(STATUS "Using LogIt build option [${LOGIT_BUILD_OPTION}]")
  if("${LOGIT_BUILD_OPTION}" STREQUAL "LOGIT_AS_INT_SRC")
    add_subdirectory( ${PROJECT_BINARY_DIR}/LogIt ${PROJECT_BINARY_DIR}/LogIt )
    set(LOGIT_TARGET_OBJECTS $<TARGET_OBJECTS:LogIt>)
    message(STATUS "LogIt added as compiled object code from sub-directory LogIt")
  else()
    if("${LOGIT_BUILD_OPTION}" STREQUAL "LOGIT_AS_EXT_SHARED")
      SET(CMAKE_FIND_LIBRARY_SUFFIXES ".so" ".lib") # windows looks for lib file to link to target dll: lib contains dll exports, symbols etc (see LogIt.h SHARED_LIB_EXPORT_DEFN)
      find_library( LOGITLIB NAMES LogIt PATHS ${LOGIT_EXT_LIB_DIR} NO_DEFAULT_PATH )
    elseif("${LOGIT_BUILD_OPTION}" STREQUAL "LOGIT_AS_EXT_STATIC")
      SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ".lib")
      find_library( LOGITLIB NAMES LogIt PATHS ${LOGIT_EXT_LIB_DIR} NO_DEFAULT_PATH )
    else()
      message(FATAL_ERROR "Invalid command given as to how to use LogIt in the open62541-compat library, see documentaton for property [LOGIT_BUILD_OPTION]")
    endif()
    if ( LOGITLIB STREQUAL "LOGITLIB-NOTFOUND")
      message(FATAL_ERROR "LogIt library (.a/.so/.dll) hasn't been found. Please set the variable LOGIT_EXT_LIB_DIR accordingly.")
    endif () 
    message(STATUS "LogIt added as external library dependency LOGITLIB [${LOGITLIB}]")
  endif()


endfunction()

