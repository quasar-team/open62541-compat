# open62541-compat
Adapts open62541 API to UA Toolkit API for usage in (1)Quasar-based projects, (2) independent UASDK-based projects

For bugs or suggestions please file a GitHub ticket.

Versions
--------

| Branch/Version | open62541 version included | LogIt version | Years active | Description |
| -------------- | ----------------- | ------------- | --- | ----------- |
| 1.5.x          | 1.5.x             | v1.0.0        | 2026- | Deferred (asynchronous) read/write/call: source variables and async methods execute on the quasar thread pool, UASDK-style. |
| 1.4.x          | 1.2.x             | v0.1.1        | 2021-2026 | Stable, widely used. |
| 1.3.x          | 1.1.x             | v0.1.1        | 2020-2021 |Old-stable.          |
| 1.2.x          | 1.0.x             | v0.1.1        | 2019-2020 | Historic. Uses open62541 1.0. |
| 1.1            | 0.3(-rc2)         | v0.1.1        | 2018-2019 | Historic. Uses open62541 0.3. |
| 1.0            | 0.2 rc2           | "master"      | 2018      | Prehistoric. Significantly improved CMake scripting etc. Usable in Yocto and PetaLinux. |
| 0.9            | 0.2 rc2           | "master"      | 2015-2018 | Prehistoric. Will stay cmake2.8 compatible. OK for native Linux and Win. Not useful for Yocto or PetaLinux.  |




Quick-start guide in Quasar
---------------------------
The procedure is documented in Quasar project.
In Quasar repo, read:

Documentation/AlternativeBackends.html

Quick-start guide to get a stand-alone (independent) library
-----------------------------------------------------------
1. clone the repo

2. configure with CMake. A few choices, depending on

   (a) do you want to build open62541-compat as a shared or static library?

   (b) open62541-compat requires the boost library: system boost or a custom build?

   (c) open62541-compat depends on the quasar module LogIt: built-in from source (default) or consumed from an external build.

### Linux examples
   - Static library, system boost (e.g. boost-devel), LogIt built in:
     ```
     cmake -B build -DSTANDALONE_BUILD=ON
     cmake --build build -j$(nproc)
     ```
     If `find_package(Boost)` needs help on your platform, add a build config file:
     ```
     cmake -B build -DSTANDALONE_BUILD=ON -DOPEN62541-COMPAT_BUILD_CONFIG_FILE=boost_standard_install.cmake
     ```
     Other sample config files: `boost_lcg.cmake` (LCG releases), `boost_custom.cmake` (self-built boost via the `BOOST_PATH_HEADERS`/`BOOST_PATH_LIBS` environment variables).

   - **Shared** library: add `-DSTANDALONE_BUILD_SHARED=ON`.

   - LogIt from an external build (here: shared, from /tmp/LogIt):
     ```
     cmake -B build -DSTANDALONE_BUILD=ON -DSTANDALONE_BUILD_SHARED=ON -DLOGIT_BUILD_OPTION=LOGIT_AS_EXT_SHARED -DLOGIT_EXT_LIB_DIR=/tmp/LogIt/ -DLOGIT_INCLUDE_DIR=/tmp/LogIt/include/
     cmake --build build -j$(nproc)
     ```

### Windows examples (MSVC)
   The CI-proven recipe lives in `.github/workflows/windows.yml`: prebuilt boost from nuget plus the `boost_windows_ci.cmake` config file.

   For your own boost build, point the environment at it and use `boost_custom_win.cmake` (version/toolset agnostic):
   ```
   set BOOST_PATH_HEADERS=C:\path\to\boost\include
   set BOOST_PATH_LIBS=C:\path\to\boost\lib64

   cmake -B build -DSTANDALONE_BUILD=ON -DOPEN62541-COMPAT_BUILD_CONFIG_FILE=boost_custom_win.cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
   cmake --build build --config Release
   ```

### Unit tests
Writing unit tests (and running unit tests) is a good habit; developers - if you're adding new features we recommend adding tests to really lock that work in place. Passing tests signify that features work now and provide an invaluable tool for future developers; to help them avoid inadvertently broken existing functionality. So, be a conscientious developer, write tests and run tests. By default (for stand-alone open62541-compat builds) unit tests are part of the build, if you need to skip them you can (but why would you?), more on skipping tests later. The open62541-compat module unit tests are based on the [googletest](https://github.com/google/googletest) framework. Note that the unit tests build to a stand-alone executable, this executable links to the stand-alone open62541-compat library as used in end-user applications - building the unit tests has ZERO effect on the actual end-user library binary that the build outputs.
* Unit tests and their requirements (basically googletest) are in the test subdirectory.
* New unit tests should be added to test/src (for help on writing tests see the googletest documentation, you might find inspiration in the existing tests too).
* From a clean start, the googletest framework is built from source (cloned from [here](https://github.com/google/googletest)), so it should execute on your platform.
* On running make for the open62541-compat module, the unit tests build to this executable: __build/test/open62541-compat-Test__. Note that executable is built after the open62541-compat library is built.
* Run those tests by simply invoking that executable:
```
./build/test/open62541-compat-Test
```

#### Skipping tests
If, for some reason, you feel you must omit unit tests from the build then there is a way, just add __-DSKIP_TESTS=ON__ to your cmake invocation command line. Taking the example of the stand-alone static library build on linux above the command line would be
```
cmake -DOPEN62541-COMPAT_BUILD_CONFIG_FILE=boost_standard_install.cmake -DSTANDALONE_BUILD=ON -DSKIP_TESTS=ON
```
(and feel guilty.)

###### How do you use it in your independent UASDK server or client? ######

The include directory contains UASDK-(partially)-compatible headers, and you link with `build/libopen62541-compat.a` (the bundled open62541 stack is compiled in).

Maintainer: Paris Moschovakos <paris.moschovakos@cern.ch>. For questions/issues please file a GitHub ticket.
Original author: Piotr Nikiel.

CI build info | CI build status
------------ | -------------
linux (GitHub Actions) | [![CI](https://github.com/quasar-team/open62541-compat/actions/workflows/ci.yml/badge.svg)](https://github.com/quasar-team/open62541-compat/actions/workflows/ci.yml)
windows (GitHub Actions) | [![Windows CI](https://github.com/quasar-team/open62541-compat/actions/workflows/windows.yml/badge.svg)](https://github.com/quasar-team/open62541-compat/actions/workflows/windows.yml)
