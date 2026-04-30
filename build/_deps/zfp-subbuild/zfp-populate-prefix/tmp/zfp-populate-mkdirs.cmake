# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/Users/demureronin/CLionProjects/data_channel/build/_deps/zfp-src")
  file(MAKE_DIRECTORY "/Users/demureronin/CLionProjects/data_channel/build/_deps/zfp-src")
endif()
file(MAKE_DIRECTORY
  "/Users/demureronin/CLionProjects/data_channel/build/_deps/zfp-build"
  "/Users/demureronin/CLionProjects/data_channel/build/_deps/zfp-subbuild/zfp-populate-prefix"
  "/Users/demureronin/CLionProjects/data_channel/build/_deps/zfp-subbuild/zfp-populate-prefix/tmp"
  "/Users/demureronin/CLionProjects/data_channel/build/_deps/zfp-subbuild/zfp-populate-prefix/src/zfp-populate-stamp"
  "/Users/demureronin/CLionProjects/data_channel/build/_deps/zfp-subbuild/zfp-populate-prefix/src"
  "/Users/demureronin/CLionProjects/data_channel/build/_deps/zfp-subbuild/zfp-populate-prefix/src/zfp-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/demureronin/CLionProjects/data_channel/build/_deps/zfp-subbuild/zfp-populate-prefix/src/zfp-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/demureronin/CLionProjects/data_channel/build/_deps/zfp-subbuild/zfp-populate-prefix/src/zfp-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
