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

   - Build open62541-compat as a static library on linux. Use whatever the system installation of boost-devel is and build LogIt
     directly into the compat library
     ```
     cmake -DCMAKE_TOOLCHAIN_FILE=boost_standard_install_cc7.cmake -DSTANDALONE_BUILD=ON
     ```

   - Build open62541-compat as a **shared** library on linux (and boost/LogIt treated as above) - just add **STANDALONE_BUILD_SHARED**
     ```
     cmake -DCMAKE_TOOLCHAIN_FILE=boost_standard_install_cc7.cmake -DSTANDALONE_BUILD=ON -DSTANDALONE_BUILD_SHARED=ON
     ```

   - Build Build open62541-compat as a static library on windows (visual studio 2017). Use a custom boost build (perhaps you built boost 
     yourself, or have several boost installations available to choose from). Build LogIt directly into the compat library.
     ```
     cmake -DCMAKE_TOOLCHAIN_FILE=boost_custom_win_VS2017.cmake -DSTANDALONE_BUILD=ON -G "Visual Studio 15 2017 Win64" -DCMAKE_BUILD_TYPE=Release
     ```
     Note! We don't know how your boost build/installation looks, but take a look at the sample custom toolchain file (__boost_custom_win_VS2017.cmake__)
     for inspiration and write your own.
     Note! Our custom toolchain file (__boost_custom_win_VS2017.cmake__) requires that you specify where your boost headers and libraries are via an
     environment variables (dumped below).
     ```
     $ env | grep BOOST
	 BOOST_PATH_HEADERS=/c/3rdPartySoftware/boost_mapped_namespace_builder/work/MAPPED_NAMESPACE_INSTALL/include/
     BOOST_PATH_LIBS=/c/3rdPartySoftware/boost_mapped_namespace_builder/work/MAPPED_NAMESPACE_INSTALL/lib/
     ```

   - Build open62541-compat as a shared library on windows (visual studio 2017). Use a custom boost build (perhaps you built boost yourself, or have 
     several boost installations available). Build LogIt directly into the compat shared library.
     ```
     cmake -DCMAKE_TOOLCHAIN_FILE=boost_custom_win_VS2017.cmake -DSTANDALONE_BUILD=ON -DSTANDALONE_BUILD_SHARED=ON -G "Visual Studio 15 2017 Win64" -DCMAKE_BUILD_TYPE=Release
     ```



###### How do you use it in your independent UASDK server or client? ######

The include directory contains UASDK-(partially)-compatible headers, and you should link with the following libs:
* build/libopen62541-compat.a
* open62541/build/libopen62541.a

For questions/issues: file a ticket or write to piotr.nikiel@cern.ch

