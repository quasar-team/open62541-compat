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