add_executable(ex_simple_client_read src/ex_simple_client_read.cpp)
target_link_libraries (ex_simple_client_read
	${BOOST_LIBS}
	open62541-compat

)