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
cmake_minimum_required( VERSION 2.8 )
#project( FindSystemBoost CXX )
message(STATUS "Using file [boost_standard_install_cc7.cmake] toolchain file")

message(STATUS "CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES [${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES}]")

#
# Warning: *Distinctly* suspect that these should require be set but otherwise
# no boost libs are found. Possibly fixed in some later version, but not in
# cmake v2.8's FindBoost package
#
set(BOOST_INCLUDEDIR "/usr/include/")
set(BOOST_LIBRARYDIR "/usr/lib64/")

set(CMAKE_FIND_LIBRARY_SUFFIXES ".so")
set(CMAKE_FIND_LIBRARY_PREFIXES "lib")

find_package(Boost REQUIRED
	program_options system filesystem chrono date_time thread)

if(NOT Boost_FOUND)
	message(FATAL_ERROR "Failed to find system boost installation (is the boost-devel package installed on this machine?)")
else()
	message(STATUS "Found system boost, version [${Boost_VERSION}], include dir [${Boost_INCLUDE_DIRS}] library dir [${Boost_LIBRARY_DIRS}], libs [${Boost_LIBRARIES}]")
endif()	

include( ${Boost_INCLUDE_DIRS} )
set( BOOST_LIBS ${Boost_LIBRARIES} )
