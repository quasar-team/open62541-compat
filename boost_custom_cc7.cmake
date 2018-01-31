# LICENSE:
# Copyright (c) 2018, Ben Farnham, CERN
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
# Ben Farnham <firstNm.secondNm@cern.ch>
message(STATUS "Using file [boost_custom_cc7.cmake] toolchain file")

message(STATUS "environment vars: BOOST_HOME [$ENV{BOOST_HOME}] UNIFIED_AUTOMATION_HOME [$ENV{UNIFIED_AUTOMATION_HOME}]")
message(STATUS "Boost - include environment variable BOOST_PATH_HEADERS [$ENV{BOOST_PATH_HEADERS}] libs environment variable BOOST_PATH_LIBS [$ENV{BOOST_PATH_LIBS}]")

#-------
# Boost headers
#-------
if( NOT DEFINED ENV{BOOST_PATH_HEADERS} OR NOT EXISTS $ENV{BOOST_PATH_HEADERS} )
	message(FATAL_ERROR "environment variable BOOST_PATH_HEADERS must be set to a valid path for boost header files. Current value [$ENV{BOOST_PATH_HEADERS}] rejected")
endif()
message(STATUS "Boost - headers will be included from [$ENV{BOOST_PATH_HEADERS}]")
include_directories( $ENV{BOOST_PATH_HEADERS} )
 

#-------
# Boost compiled libs
#-------
if( NOT DEFINED ENV{BOOST_PATH_LIBS} OR NOT EXISTS $ENV{BOOST_PATH_LIBS} )
	message(FATAL_ERROR "environment variable BOOST_PATH_LIBS must be set to a valid path for boost compiled libraries. Current value [$ENV{BOOST_PATH_LIBS}] rejected")
endif()
message(STATUS "Boost - libraries will be linked from [$ENV{BOOST_PATH_LIBS}]")

function( find_boost_static_library LIBRARY_IDENTIFIER LIBRARY_FILE_NAME)
	SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
	SET(CMAKE_FIND_LIBRARY_PREFIXES "")

	find_library(${LIBRARY_IDENTIFIER} NAMES ${LIBRARY_FILE_NAME} PATHS $ENV{BOOST_PATH_LIBS}  NO_DEFAULT_PATH)
	message(STATUS "${LIBRARY_IDENTIFIER} has value [${${LIBRARY_IDENTIFIER}}]")
	if(NOT ${LIBRARY_IDENTIFIER})
		message(FATAL_ERROR "Failed to load boost static library ID [${LIBRARY_IDENTIFIER}] file [${LIBRARY_FILE_NAME}]")
	endif()
	message(STATUS "loaded boost static library [${LIBRARY_IDENTIFIER}] -> file [${${LIBRARY_IDENTIFIER}}]")
endfunction()

find_boost_static_library( libboostprogramoptions libboost_1_59_0_program_options )
find_boost_static_library( libboostsystem libboost_1_59_0_system )
find_boost_static_library( libboostfilesystem libboost_1_59_0_filesystem )
find_boost_static_library( libboostchrono libboost_1_59_0_chrono )
find_boost_static_library( libboostdatetime libboost_1_59_0_date_time) 
find_boost_static_library( libboostthread libboost_1_59_0_thread)
find_boost_static_library( libboostlog libboost_1_59_0_log)
find_boost_static_library( libboostlogsetup libboost_1_59_0_log_setup)

set( BOOST_LIBS ${libboostlogsetup} ${libboostlog} ${libboostsystem} ${libboostfilesystem} ${libboostthread} ${libboostprogramoptions} ${libboostchrono} ${libboostdatetime} -lrt)
