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

message(STATUS "Using file [boost_custom_win.cmake] toolchain file")
message(STATUS "Boost - include environment variable BOOST_PATH_HEADERS [$ENV{BOOST_PATH_HEADERS}] libs environment variable BOOST_PATH_LIBS [$ENV{BOOST_PATH_LIBS}]")

# Custom boost installation on Windows (MSVC), located through two environment
# variables. Version/toolset agnostic: libraries are matched by their tagged
# file names (e.g. libboost_chrono-vc143-mt-x64-1_87.lib). Requires
# -DCMAKE_BUILD_TYPE=Release|Debug on the configure line.
if( NOT DEFINED ENV{BOOST_PATH_HEADERS} OR NOT EXISTS $ENV{BOOST_PATH_HEADERS} )
	message(FATAL_ERROR "environment variable BOOST_PATH_HEADERS must be set to a valid path for boost header files. Current value [$ENV{BOOST_PATH_HEADERS}] rejected")
endif()
include_directories( $ENV{BOOST_PATH_HEADERS} )

if( NOT DEFINED ENV{BOOST_PATH_LIBS} OR NOT EXISTS $ENV{BOOST_PATH_LIBS} )
	message(FATAL_ERROR "environment variable BOOST_PATH_LIBS must be set to a valid path for boost compiled libraries. Current value [$ENV{BOOST_PATH_LIBS}] rejected")
endif()

function( find_msvc_boost_library OUT_VAR COMPONENT )
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		file(GLOB _candidates "$ENV{BOOST_PATH_LIBS}/*boost_${COMPONENT}-*-mt-gd-x64-*.lib")
	else()
		file(GLOB _candidates "$ENV{BOOST_PATH_LIBS}/*boost_${COMPONENT}-*-mt-x64-*.lib")
	endif()
	list(LENGTH _candidates _n)
	if(_n EQUAL 0)
		message(FATAL_ERROR "No MSVC boost library found for component [${COMPONENT}] under [$ENV{BOOST_PATH_LIBS}] (CMAKE_BUILD_TYPE [${CMAKE_BUILD_TYPE}])")
	endif()
	list(GET _candidates 0 _lib)
	set(${OUT_VAR} ${_lib} PARENT_SCOPE)
	message(STATUS "boost ${COMPONENT} -> [${_lib}]")
endfunction()

find_msvc_boost_library( libboostprogramoptions program_options )
find_msvc_boost_library( libboostchrono chrono )
find_msvc_boost_library( libboostdatetime date_time )
find_msvc_boost_library( libboostthread thread )

set( BOOST_LIBS ${libboostthread} ${libboostprogramoptions} ${libboostchrono} ${libboostdatetime})
