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
# Paris Moschovakos <paris.moschovakos@cern.ch>

message(STATUS "Using file [boost_custom.cmake] toolchain file")
message(STATUS "Boost - include environment variable BOOST_PATH_HEADERS [$ENV{BOOST_PATH_HEADERS}] libs environment variable BOOST_PATH_LIBS [$ENV{BOOST_PATH_LIBS}]")

# Custom (self-built) boost installation, located through two environment
# variables. Works with any boost version new enough for the components below.
if( NOT DEFINED ENV{BOOST_PATH_HEADERS} OR NOT EXISTS $ENV{BOOST_PATH_HEADERS} )
	message(FATAL_ERROR "environment variable BOOST_PATH_HEADERS must be set to a valid path for boost header files. Current value [$ENV{BOOST_PATH_HEADERS}] rejected")
endif()
include_directories( $ENV{BOOST_PATH_HEADERS} )

if( NOT DEFINED ENV{BOOST_PATH_LIBS} OR NOT EXISTS $ENV{BOOST_PATH_LIBS} )
	message(FATAL_ERROR "environment variable BOOST_PATH_LIBS must be set to a valid path for boost compiled libraries. Current value [$ENV{BOOST_PATH_LIBS}] rejected")
endif()

function( find_custom_boost_library LIBRARY_IDENTIFIER COMPONENT REQUIRED )
	SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ".so")
	find_library(${LIBRARY_IDENTIFIER} NAMES boost_${COMPONENT} PATHS $ENV{BOOST_PATH_LIBS} NO_DEFAULT_PATH)
	if(NOT ${LIBRARY_IDENTIFIER})
		if(${REQUIRED})
			message(FATAL_ERROR "Failed to find boost library [${COMPONENT}] under [$ENV{BOOST_PATH_LIBS}]")
		endif()
		set(${LIBRARY_IDENTIFIER} "" PARENT_SCOPE)
		return()
	endif()
	message(STATUS "boost ${COMPONENT} -> [${${LIBRARY_IDENTIFIER}}]")
endfunction()

find_custom_boost_library( libboostprogramoptions program_options TRUE )
find_custom_boost_library( libboostchrono chrono TRUE )
find_custom_boost_library( libboostdatetime date_time TRUE )
find_custom_boost_library( libboostthread thread TRUE )
find_custom_boost_library( libboostsystem system FALSE )

set( BOOST_LIBS ${libboostthread} ${libboostprogramoptions} ${libboostchrono} ${libboostdatetime} ${libboostsystem})
