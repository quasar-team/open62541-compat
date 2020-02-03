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



endfunction()

