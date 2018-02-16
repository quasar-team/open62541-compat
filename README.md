# open62541-compat
Adapts open62541 API to UA Toolkit API for usage in (1)Quasar-based projects, (2) independent UASDK-based projects

For bugs or suggestions please file a GitHub ticket.

Quick-start guide in Quasar
---------------------------
The procedure is documented in Quasar project.
In Quasar repo, read:

Documentation/AlternativeBackends.html

Quick-start guide to get an stand-alone(independent) library
-----------------------------------------------------------
1. clone the repo

2. generate the makefiles / VS solution using cmake. A few options here, depending on

   (a) do you want to build open62541-compat as a shared or static library?

   (b) open62541-compat requires the boost library, do you want to use the system boost library or do you have a custom boost build?

   (c) open62541-compat depends on the quasar module LogIt, should the compat module clone, build and link LogIt source directly into 
       the compat library or should the compat library consume LogIt from some external build.

   Time for an example...

### Linux examples
   - Build open62541-compat as a static library on linux. Use whatever the system installation of boost-devel is and build LogIt
     directly into the compat library
     ```
     cmake -DCMAKE_TOOLCHAIN_FILE=boost_standard_install_cc7.cmake -DSTANDALONE_BUILD=ON
     ```

   - Build open62541-compat as a **shared** library on linux (and boost/LogIt treated as above) - just add **STANDALONE_BUILD_SHARED**
     ```
     cmake -DCMAKE_TOOLCHAIN_FILE=boost_standard_install_cc7.cmake -DSTANDALONE_BUILD=ON -DSTANDALONE_BUILD_SHARED=ON
     ```
     
   - Build open62541-compat as a shared library on linux but with the LogIt library pulled in from some external build location (here LogIt is consumed as a shared library from /tmp/LogIt/). Note the **-DLOGIT** options define how LogIt is built into open62541-compat.
     ```
     cmake -DCMAKE_TOOLCHAIN_FILE=boost_standard_install_cc7.cmake -DSTANDALONE_BUILD=ON -DSTANDALONE_BUILD_SHARED=ON -DLOGIT_BUILD_OPTION=LOGIT_AS_EXT_SHARED -DLOGIT_EXT_LIB_DIR=/tmp/LogIt/ -DLOGIT_INCLUDE_DIR=/tmp/LogIt/include/
     ```
### Windows examples (using a mingw 'git bash' command prompt)
   - Build open62541-compat as a static library on windows (visual studio 2017). Use a custom boost build (perhaps you built boost 
     yourself, or have several boost installations available to choose from). Build LogIt directly into the compat library.
     ```
     # (comment) - set environment variables to point to the custom boost headers/libs directories (required by the toolchain file)
     export BOOST_PATH_HEADERS=/c/3rdPartySoftware/boost_mapped_namespace_builder/work/MAPPED_NAMESPACE_INSTALL/include/
     export BOOST_PATH_LIBS=/c/3rdPartySoftware/boost_mapped_namespace_builder/work/MAPPED_NAMESPACE_INSTALL/lib/
     
     cmake -DCMAKE_TOOLCHAIN_FILE=boost_custom_win_VS2017.cmake -DSTANDALONE_BUILD=ON -G "Visual Studio 15 2017 Win64" -DCMAKE_BUILD_TYPE=Release
     ```
     Note! Your boost build/installation may be more exotic than ours, take a look at the sample custom toolchain file (__boost_custom_win_VS2017.cmake__)
     for inspiration, write your own and use it in your build via the build option **CMAKE_TOOLCHAIN_FILE**
     
   - Build open62541-compat as a shared library on windows (visual studio 2017). Use a custom boost build. Use the LogIt library (here as a shared library) from some external build (as with the linux build above, the **-DLOGIT** options define how LogIt is built into open62541-compat).
     ```
     # (comment) - set environment variables to point to the custom boost headers/libs directories (required by the toolchain file)
     export BOOST_PATH_HEADERS=/c/3rdPartySoftware/boost_mapped_namespace_builder/work/MAPPED_NAMESPACE_INSTALL/include/
     export BOOST_PATH_LIBS=/c/3rdPartySoftware/boost_mapped_namespace_builder/work/MAPPED_NAMESPACE_INSTALL/lib/
     
     cmake -DCMAKE_TOOLCHAIN_FILE=boost_custom_win_VS2017.cmake -DSTANDALONE_BUILD=ON -DSTANDALONE_BUILD_SHARED=ON -G "Visual Studio 15 2017 Win64" -DCMAKE_BUILD_TYPE=Release -DLOGIT_BUILD_OPTION=LOGIT_AS_EXT_SHARED -DLOGIT_EXT_LIB_DIR=/c/workspace/OPC-UA/LogIt/Release/ -DLOGIT_INCLUDE_DIR=/c/workspace/OPC-UA/LogIt/include/
     ```



###### How do you use it in your independent UASDK server or client? ######

The include directory contains UASDK-(partially)-compatible headers, and you should link with the following libs:
* build/libopen62541-compat.a
* open62541/build/libopen62541.a

For questions/issues: file a ticket or write to piotr.nikiel@cern.ch

