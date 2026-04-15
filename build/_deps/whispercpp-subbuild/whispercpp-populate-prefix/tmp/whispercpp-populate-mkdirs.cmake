# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/Users/zhaopeng.charles/code/magechiu/aiplayer/build/_deps/whispercpp-src")
  file(MAKE_DIRECTORY "/Users/zhaopeng.charles/code/magechiu/aiplayer/build/_deps/whispercpp-src")
endif()
file(MAKE_DIRECTORY
  "/Users/zhaopeng.charles/code/magechiu/aiplayer/build/_deps/whispercpp-build"
  "/Users/zhaopeng.charles/code/magechiu/aiplayer/build/_deps/whispercpp-subbuild/whispercpp-populate-prefix"
  "/Users/zhaopeng.charles/code/magechiu/aiplayer/build/_deps/whispercpp-subbuild/whispercpp-populate-prefix/tmp"
  "/Users/zhaopeng.charles/code/magechiu/aiplayer/build/_deps/whispercpp-subbuild/whispercpp-populate-prefix/src/whispercpp-populate-stamp"
  "/Users/zhaopeng.charles/code/magechiu/aiplayer/build/_deps/whispercpp-subbuild/whispercpp-populate-prefix/src"
  "/Users/zhaopeng.charles/code/magechiu/aiplayer/build/_deps/whispercpp-subbuild/whispercpp-populate-prefix/src/whispercpp-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/zhaopeng.charles/code/magechiu/aiplayer/build/_deps/whispercpp-subbuild/whispercpp-populate-prefix/src/whispercpp-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/zhaopeng.charles/code/magechiu/aiplayer/build/_deps/whispercpp-subbuild/whispercpp-populate-prefix/src/whispercpp-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
