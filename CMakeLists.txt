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
project(open62541-compat LANGUAGES C CXX)
cmake_minimum_required(VERSION 3.0)
cmake_policy(SET CMP0012 NEW)

include (cmake/Open62541CompatFetchContent.cmake)
option(PULL_OPEN62541 "Pull open62541 from their github instead of using the bundled one (not recommended!)" OFF)

option(STANDALONE_BUILD "Build it as a stand-alone library instead of for Quasar" OFF )
option(STANDALONE_BUILD_SHARED "When building in stand-alone, build shared library rather than static library" OFF)
option(SKIP_TESTS "do not build the tests (not advised)" OFF)
message(STATUS "STANDALONE_BUILD [${STANDALONE_BUILD}] STANDALONE_BUILD_SHARED [${STANDALONE_BUILD_SHARED}]")
# "OPEN62541_VERSION" will be used only if you use PULL_OPEN62541
SET (OPEN62541_VERSION "v0.3.0" CACHE STRING "Which open62541 commit/tag/branch to take")
option (UA_ENABLE_AMALGAMATION "Whether open62541 should amalgamate" ON )
option (UA_ENABLE_METHODCALLS  "Whether open62541 should have methods enabled" ON )


if(${PULL_OPEN62541})
        function ( fetch_open62541 )
          message(STATUS "fetching open62541 from github. *NOTE* fetching tag [${OPEN62541_VERSION}]")
          Open62541CompatFetchContent_Declare(
            open62541
            GIT_REPOSITORY    https://github.com/open62541/open62541.git
            GIT_TAG           ${OPEN62541_VERSION}
            GIT_SHALLOW       "1"
            SOURCE_DIR	      ${PROJECT_BINARY_DIR}/open62541
            BINARY_DIR	      ${PROJECT_BINARY_DIR}/open62541
          )
          Open62541CompatFetchContent_Populate( open62541 )
          message(STATUS "open62541 fetched")
        endfunction()

        function ( build_open62541 )
          message(STATUS "generating platform specific build for open62541 library for [${CMAKE_GENERATOR}]")
          add_subdirectory( ${PROJECT_BINARY_DIR}/open62541
                            ${PROJECT_BINARY_DIR}/open62541
          		    EXCLUDE_FROM_ALL  # EXCLUDE_FROM_ALL will not add install() targets from open62541 which we clearly don't want/need
        		    )
          set_target_properties( open62541 PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/open62541 )
        endfunction()

endif(${PULL_OPEN62541})

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
  src/uabytearray.cpp
  src/opcua_basedatavariabletype.cpp
  src/uaserver.cpp
  src/uaclient/uasession.cpp
)

if(NOT ${PULL_OPEN62541})  # this is the default behaviour.
	include_directories(extern/open62541/include)
	list(APPEND SRCS extern/open62541/src/open62541.c)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
endif(NOT ${PULL_OPEN62541})

if(${PULL_OPEN62541})  # this is not default
        fetch_open62541()
        build_open62541()
        set(OPEN62541_SDK_LIB -lopen62541)
endif(${PULL_OPEN62541})

add_definitions(-DNOMINMAX)  # doesn't include windows' min() and max() which break std c++ code. Should be neutral to non-windows code.

if(NOT STANDALONE_BUILD)
  add_library ( open62541-compat OBJECT ${SRCS} )
  add_custom_target( quasar_opcua_backend_is_ready DEPENDS open62541-compat )
else()

  # We need some C++11
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wno-literal-suffix" )

  #
  # Decide on open62541-compat build output (shared/static)
  #
  if (STANDALONE_BUILD_SHARED)
  	 set(OPEN62541_COMPAT_LIB_FORMAT SHARED)
         add_definitions(-fPIC)
  else()
     set(OPEN62541_COMPAT_LIB_FORMAT STATIC)
  endif()


  include_directories( include )
  include_directories( ${PROJECT_BINARY_DIR}/open62541 )

  #
  # Load stand-alone build toolchain file (used for defining build-specific boost (BOOST_LIBS), amongst other things)
  #
  if(DEFINED OPEN62541-COMPAT_BUILD_CONFIG_FILE)
    message("OPEN62541-COMPAT_BUILD_CONFIG_FILE is defined -- including [[${OPEN62541-COMPAT_BUILD_CONFIG_FILE}]]")
    include(${OPEN62541-COMPAT_BUILD_CONFIG_FILE})
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
    add_subdirectory( ${PROJECT_BINARY_DIR}/LogIt )
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
    target_link_libraries( open62541-compat ${LOGITLIB} )
    message(STATUS "LogIt added as external library dependency LOGITLIB [${LOGITLIB}]")
  endif()

  link_directories(
     ${PROJECT_BINARY_DIR}/open62541
     ${PROJECT_BINARY_DIR}/open62541/build
     ${PROJECT_BINARY_DIR}/open62541/Release/
     ${PROJECT_BINARY_DIR}/open62541/Debug/ )

  add_library( open62541-compat ${OPEN62541_COMPAT_LIB_FORMAT} ${SRCS} $<TARGET_OBJECTS:LogIt> )

  #
  # Set required libs, note windows build requires winsock.
  #

  target_link_libraries( open62541-compat ${BOOST_LIBS} ${OPEN62541_SDK_LIB} )

  if(WIN32)
    target_link_libraries( open62541-compat ws2_32 )
  endif()

  #
  # Build unit tests (googletest). For the moment only build unit tests in stand-alone mode
  # until such time as the quasar framework has a comprehensive strategy for testing optional
  # modules.
  #
  if( NOT SKIP_TESTS )
    add_subdirectory( ${PROJECT_SOURCE_DIR}/test )
  else()
    message( STATUS "explicitly requested not to build unit tests!" )
  endif()

endif()

if(${PULL_OPEN62541})  # this is not default
  add_dependencies( open62541-compat open62541 )
endif(${PULL_OPEN62541})
