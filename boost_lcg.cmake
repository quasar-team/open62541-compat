message(STATUS "Using file [boost_lcg.cmake] file")

# Disabling the search for boost-cmake to use FindBoost instead. 
# That is a workaround for boost-1.7.0 cmake config modules.
set(Boost_NO_BOOST_CMAKE ON)

find_package(Boost REQUIRED program_options system filesystem chrono date_time thread)
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Failed to find boost installation")
else()
    message(STATUS "Found system boost, version [${Boost_VERSION}], include dir [${Boost_INCLUDE_DIRS}] library dir [${Boost_LIBRARY_DIRS}], libs [${Boost_LIBRARIES}]")
    include_directories( ${Boost_INCLUDE_DIRS} )
	set( BOOST_LIBS ${Boost_LIBRARIES} )
endif()
