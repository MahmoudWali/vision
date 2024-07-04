# FindHalcon.cmake
# Copyright (c) 2022-2023 by MVTec Software GmbH
#
# Find the native HALCON headers and libraries
#
# Finding and using Halcon
# ^^^^^^^^^^^^^^^^^^^^^^^^
#
# This module can be used to find HALCON. The following variables can be set
# to help CMake find the right version/variant of HALCON:
#
# HALCON_DIR          - The location to look for HALCON at. Set this if HALCON
#                       cannot be found by cmake's normal search mechanism. Note
# that HALCON_DIR ignores CMAKE_SYSROOT and/or CMAKE_FIND_ROOT_PATH.
#
# HALCON_ARCHITECTURE - The (HALCON) architecture name. If this is not set,
#                       CMake will attempt to guess the architecture to use
# based on the CMAKE_SYSTEM_NAME and CMAKE_SYSTEM_PROCESSOR variables.
#
# HALCON_HOST_ARCHITECTURE - the (HALCON) architecture name for the host
#                            system (i.e. the system being built on). This
# variable is relevant when looking for the hcomp executable, which must run
# on the host system. If HALCON_HOST_ARCHITECTURE is not set, CMake will set it
# to the value of HALCON_ARCHITECTURE if CMAKE_CROSS_COMPILING is not true, or
# guess based on CMAKE_HOST_SYSTEM_NAME and CMAKE_HOST_SYSTEM_PROCESSOR
# otherwise.
#
# HALCON_LIBDIR       - The location to look for HALCON library files at. Set
#                       this if the libraries are not located in the place
# standard HALCON normally puts them.
#
# The following variables can be set to select the HALCON variant to use:
#
# HALCON_SEQUENTIAL    - If true, use sequential HALCON.
# HALCON_STATIC        - If true, use static HALCON libraries.
# HALCON_XL            - If true, use HALCON XL.
#
# If any of the above variant selector variables are not set, CMake will
# attempt to figure out which variant to use by itself and set the above
# variables accordingly. If multiple variants are available, shared libraries
# are preferred over static libraries, non-XL HALCON is preferred over HALCON
# XL, and parallel HALCON is preferred over sequential HALCON, in that order.
#
# For special use cases, additional variables can be used to select the HALCON
# variant to use. Unlike the variables above, these will never be set
# automatically, but must be explicitly set before calling find_package.
# The following special variables are available:
#
# HALCON_STATIC_CINT   - If true, use the static HALCON C language interface
#                        independently of the HALCON_STATIC setting.
# HALCON_STATIC_CPPINT - If true, use the static HALCON C++ language interface
#                        independently of the HALCON_STATIC setting.
#
#
# COMPONENTS
# ^^^^^^^^^^
#
# The following COMPONENTS can be specified to the find_package command:
#
# Halcon           - the main HALCON library
# CInt             - the HALCON C language interface
# CppInt           - the HALCON C++ language interface
# DotNetInt        - HALCON .NET interface
# HDevEngine       - HALCON HDevEngine
# HDevEngineDotNet - HALCON HDevEngine .NET interface
# hcomp            - hcomp executable (runable on the host system)
# HDevelop         - HDevelop executable (runable on the host system)
#
# If no COMPONENTS are specified, then the package will attempt to find
# components based on the enabled languages. The hcomp and HDevelop components
# are only searched for if they are explicitly mentioned in the COMPONENTS or
# OPTIONAL_COMPONENTS find_package options.
#
#
# IMPORTED Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines :prop_tgt:`IMPORTED` targets ``HALCON::Halcon``,
# ``HALCON::CInt``, ``HALCON::CppInt``, and ``HALCON::HDevEngine`` if the
# required libraries are found.
#
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module defines the following variables:
#
# ::
#
# HALCON_VERSION             - the full version of HALCON found
# HALCON_VERSION_MAJOR       - the major version of HALCON
# HALCON_VERSION_MINOR       - the minor version of HALCON
# HALCON_VERSION_PATCH       - the patchlevel of HALCON
# HALCON_VERSION_TWEAK       - the hotfix version of HALCON
# HALCON_INCLUDE_DIR         - where to find Halcon.h, etc.
# HALCON_LIBRARY             - Main HALCON library
# HALCON_CInt_LIBRARY        - HALCON C language interface library
# HALCON_CppInt_LIBRARY      - HALCON C++ language interface library
# HALCON_HDevEngine_LIBRARY  - HALCON HDevEngine library
# HALCON_DotNetInt_ASSEMBLY  - HALCON .NET interface assembly. Since CMake
#                              only supports C# projects when using the
#                              Visual Studio 2010 generator or later, the
#                              interface for .NET 3.5 and later is always used.
# HALCON_hcomp_EXECUTABLE    - path to hcomp executable, runable on the host
#                              system
# HALCON_HDevelop_EXECUTABLE - path to HDevelop, runable on the host system
# HALCON_BUILD_SUFFIX        - Empty for a release version of HALCON, 'd' for
#                              a debug build, and 't' for a tuning build. Since
#                              the HALCON build type must be inferred from the
#                              name of directory containing the library, this
#                              will only work correctly if the HALCON
#                              installation follows the standard MVTec layout.
#
# NOTES
# ^^^^^
#
# Keep in mind that starting with HALCON 17.12, the HALCON 'major' and 'minor'
# version numbers combined are what would be the 'major' version for other
# libraries.
#
# When using HALCON frameworks on macOS, the headers from the frameworks are
# used by passing '-F <framework directory>' to the compiler. However, when
# linking the absolute paths to the frameworks are used.
#
# Do not use the variables HALCON_INCLUDE_DIR or HALCON_LIBRARY; instead use
# the IMPORTED targets. Otherwise, you are likely to run into problem e.g on
# macOS where the header files are stored within their respective frameworks.
#
include(FindPackageMessage)
include(FindPackageHandleStandardArgs)

# If an IMPORTED_LOCATION cannot be found, report an error during configuration,
# not during build (required CMake 3.19 or later)
if(POLICY CMP0111)
  cmake_policy(SET CMP0111 NEW)
endif()

# Internal function to determine the HALCON architecture name (e.g. 'x64-linux')
# based on the operating system SYSTEM and the processor architecture PROCESSOR.
# This is broken out into its own function so it can be used to determine both
# the HALCON_ARCHITECTURE and the HALCON_HOST_ARCHITECTURE
function(HALCON_guess_architecture VAR SYSTEM PROCESSOR)
  set(_result "-NOTFOUND")
  if(SYSTEM STREQUAL "Android")
    set(_result "android${CMAKE_SYSTEM_VERSION}/${CMAKE_ANDROID_ARCH_ABI}")
  elseif(SYSTEM STREQUAL "Darwin")
    if(PROCESSOR STREQUAL "x86_64")
      set(_result "x64-macosx")
    elseif(PROCESSOR STREQUAL "arm64")
      set(_result "arm64-macosx")
    endif()
  elseif(SYSTEM STREQUAL "Linux")
    if(PROCESSOR STREQUAL "aarch64")
      set(_result "aarch64-linux")
    elseif(PROCESSOR STREQUAL "arm" OR PROCESSOR STREQUAL "armv7l")
      set(_result "armv7a-linux")
    elseif(PROCESSOR STREQUAL "x86_64")
      set(_result "x64-linux")
    endif()
  elseif(SYSTEM STREQUAL "Windows")
    if(PROCESSOR STREQUAL "x86")
      set(_result "x86sse2-win32")
    elseif(PROCESSOR STREQUAL "x86_64" OR PROCESSOR STREQUAL "AMD64")
      set(_result "x64-win64")
    endif()
  endif()
  set(${VAR} "${_result}" PARENT_SCOPE)
endfunction()

# Deal with HALCON_FIND_COMPONENTS.
if(NOT HALCON_FIND_COMPONENTS)
  # If the user didn't specify any components, select a default set based on
  # the current enabled languages.
  get_property(_languages GLOBAL PROPERTY ENABLED_LANGUAGES)
  if("C" IN_LIST _languages)
    list(APPEND HALCON_FIND_COMPONENTS "CInt")
    if(HALCON_FIND_REQUIRED)
      set(HALCON_FIND_REQUIRED_CInt 1)
    endif()
  endif()
  if("CXX" IN_LIST _languages)
    list(APPEND HALCON_FIND_COMPONENTS "CppInt;HDevEngine")
    if(HALCON_FIND_REQUIRED)
      set(HALCON_FIND_REQUIRED_CppInt 1)
      set(HALCON_FIND_REQUIRED_HDevEngine 1)
    endif()
  endif()
  if("CSharp" IN_LIST _languages)
    list(APPEND HALCON_FIND_COMPONENTS "DotNetInt;HDevEngineDotNet")
    if(HALCON_FIND_REQUIRED)
      set(HALCON_FIND_REQUIRED_DotNetInt 1)
      set(HALCON_FIND_REQUIRED_HDevEngineDotNet 1)
    endif()
  endif()
endif()

# Now deal with dependencies between the different components. First, check
# if any component that requires HALCON was selected.
foreach(_comp CInt CppInt HDevEngine)
  if(${_comp} IN_LIST HALCON_FIND_COMPONENTS)
    if(NOT "Halcon" IN_LIST HALCON_FIND_COMPONENTS)
      list(APPEND HALCON_FIND_COMPONENTS "Halcon")
    endif()
    if(HALCON_FIND_REQUIRED_${_comp})
      set(HALCON_FIND_REQUIRED_Halcon 1)
    endif()
  endif()
endforeach()

# The HDevEngine component requires the C and C++ language interfaces
if("HDevEngine" IN_LIST HALCON_FIND_COMPONENTS)
  foreach(_comp CInt CppInt)
    if(NOT ${_comp} IN_LIST HALCON_FIND_COMPONENTS)
      list(APPEND HALCON_FIND_COMPONENTS "${_comp}")
    endif()
    if(HALCON_FIND_REQUIRED_HDevEngine)
      set(HALCON_FIND_REQUIRED_${_comp} 1)
    endif()
  endforeach()
endif()

# If HALCON_ARCHITECTURE is not defined, attempt to guess the correct value.
if(NOT HALCON_ARCHITECTURE)
  HALCON_guess_architecture(HALCON_ARCHITECTURE
    ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR})
  if(NOT HALCON_ARCHITECTURE AND NOT HALCON_FIND_QUIETLY)
    message(WARNING "Cannot determine HALCON_ARCHITECTURE for package HALCON"
      " from CMAKE_SYSTEM_NAME/CMAKE_SYSTEM_PROCESSOR"
      " ${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}")
  endif()
endif()

# If HALCON_HOST_ARCHITECTURE is not defined, attempt to guess the correct
# value.
if(NOT HALCON_HOST_ARCHITECTURE)
  if(NOT CMAKE_CROSSCOMPILING)
    # If we're not cross compiling, assume the host architecture is identical
    # to the target architecture.
    set(HALCON_HOST_ARCHITECTURE ${HALCON_ARCHITECTURE})
  endif()
  if(NOT HALCON_HOST_ARCHITECTURE)
    HALCON_guess_architecture(HALCON_HOST_ARCHITECTURE
      ${CMAKE_HOST_SYSTEM_NAME} ${CMAKE_HOST_SYSTEM_PROCESSOR})
    if(NOT HALCON_HOST_ARCHITECTURE AND NOT HALCON_FIND_QUIETLY)
      message(WARNING "Cannot determine HALCON_HOST_ARCHITECTURE for package"
        " HALCON from CMAKE_HOST_SYSTEM_NAME/CMAKE_HOST_SYSTEM_PROCESSOR"
        " ${CMAKE_HOST_SYSTEM_NAME}/${CMAKE_HOST_SYSTEM_PROCESSOR}")
    endif()
  endif()
endif()

# If HALCON_LIBRARY wasn't set, look for it now. If HALCON_DIR is set, look
# *only* in that directory without taking CMAKE_FIND_ROOT_PATH into account
# (this lets us use cross compilers that expect to have CMAKE_SYSROOT set
# without forcing us to put HALCON into that sysroot). Otherwise, use the
# normal CMake search algorithm.
#
# Note that we need to look for the main HALCON library before we look for the
# headers, as the location of the latter depends on the location of the former
# on macOS.
if(NOT HALCON_LIBRARY)
  # Build the list of HALCON library names to look for. We start with a list of
  # all possible combinations and then remove any names that do not fit. Note
  # that for macOS we don't have any sequential HALCON.
  if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(_HALCON_NAMES "HALCON" "HALCONxl" "halcon_static" "halconxl_static")
  else()
    set(_HALCON_NAMES "halcon" "halconxl" "seqhalcon" "seqhalconxl"
      "halcon_static" "halconxl_static" "seqhalcon_static" "seqhalconxl_static")
  endif()

  if(HALCON_SEQUENTIAL)
    list(APPEND _remove "^(halcon|HALCON)(|xl)(|_static)$")
  elseif(DEFINED HALCON_SEQUENTIAL)
    list(APPEND _remove "^seq(halcon|HALCON)(|xl)(|_static$")
  endif()

  if(HALCON_XL)
    list(APPEND _remove "^(|seq)(halcon|HALCON)(|_static)$")
  elseif(DEFINED HALCON_XL)
    list(APPEND _remove "^(|seq)(halcon|HALCON)xl(|_static)$")
  endif()

  if(HALCON_STATIC)
    list(APPEND _remove "^(|seq)(halcon|HALCON)(|xl)$")
  elseif(DEFINED HALCON_STATIC)
    list(APPEND _remove "^(|seq)(halcon|HALCON)(|xl)_static$")
  endif()

  foreach(_name IN LISTS _HALCON_NAMES)
    foreach(_pattern IN LISTS _remove)
      string(REGEX MATCH "${_pattern}" _t "${_name}")
      if(_t)
        list(REMOVE_ITEM _HALCON_NAMES "${_t}")
      endif()
    endforeach()
  endforeach()

  if(NOT HALCON_DIR)
    # The directory structure of a normal HALCON installation on Linux and
    # Windows does not fit well with the default locations CMake searches for
    # libraries, so simply adding the HALCON library subdirectory via
    # PATH_SUFFIXES is not going to be able to find anything. Instead, we need
    # to explicitly add CMAKE_FIND_ROOT_PATH (and CMAKE_SYSROOT) to the list
    # of PATHS to search.
    set(_PATHS)
    if(DEFINED CMAKE_FIND_ROOT_PATH)
      list(APPEND _PATHS ${CMAKE_FIND_ROOT_PATH})
    endif()
    if(DEFINED CMAKE_SYSROOT)
      list(APPEND _PATHS ${CMAKE_SYSROOT})
    endif()

    if(DEFINED HALCON_LIBDIR)
      set(_HALCON_LIBDIR ${HALCON_LIBDIR})
    elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
      set(_HALCON_LIBDIR
        "libd/${HALCON_ARCHITECTURE}"
        "lib/${HALCON_ARCHITECTURE}"
      )
    elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
      set(_HALCON_LIBDIR
        "libt/${HALCON_ARCHITECTURE}"
        "lib/${HALCON_ARCHITECTURE}"
      )
    else()
      set(_HALCON_LIBDIR "lib/${HALCON_ARCHITECTURE}")
    endif()

    find_library(HALCON_LIBRARY
      NAMES         ${_HALCON_NAMES}
      PATHS         ${_PATHS}
      PATH_SUFFIXES ${_HALCON_LIBDIR}
    )
  else()
    # If the build type is not RELEASE, add the path to the debug or tuning
    # build of HALCON first to the search path.
    if(DEFINED HALCON_LIBDIR)
      set(_HALCON_LIBDIR ${HALCON_LIBDIR})
    elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
      set(_HALCON_LIBDIR
        "${HALCON_DIR}/libd/${HALCON_ARCHITECTURE}"
        "${HALCON_DIR}/lib/${HALCON_ARCHITECTURE}")
    elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
      set(_HALCON_LIBDIR
        "${HALCON_DIR}/libt/${HALCON_ARCHITECTURE}"
        "${HALCON_DIR}/lib/${HALCON_ARCHITECTURE}")
    else()
      set(_HALCON_LIBDIR "${HALCON_DIR}/lib/${HALCON_ARCHITECTURE}")
    endif()

    # On Darwin, also add HALCON_DIR without any subdirectories in case the
    # HALCON frameworks are located in a directory that does not follow the
    # normal HALCON directory structure.
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
      list(APPEND _HALCON_LIBDIR "${HALCON_DIR}")
    endif()

    find_library(HALCON_LIBRARY
      NAMES ${_HALCON_NAMES}
      PATHS ${_HALCON_LIBDIR}
      NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  endif()
endif()

# If we actually found a HALCON library, check if the name tells us anything
# about the variant we don't already know.
if(HALCON_LIBRARY)
  set(HALCON_Halcon_FOUND TRUE)
  get_filename_component(_HALCON_LIBRARY "${HALCON_LIBRARY}" NAME_WE)
  find_package_message(HALCON "Found HALCON library: ${HALCON_LIBRARY}"
    "[${HALCON_LIBRARY}][${HALCON_INCLUDE_DIR}]")
  # The directory the main HALCON library is in is used to find the other
  # libraries.
  get_filename_component(_HALCON_LIBDIR ${HALCON_LIBRARY} DIRECTORY)
  if(NOT DEFINED HALCON_SEQUENTIAL)
    string(REGEX MATCH "seq(halcon|HALCON)(|xl)(|_static)" _T ${_HALCON_LIBRARY})
    if(_T)
      message(STATUS "Using sequential HALCON")
      set(HALCON_SEQUENTIAL ON)
    else()
      set(HALCON_SEQUENTIAL OFF)
    endif()
  endif()
  if(NOT DEFINED HALCON_XL)
    string(REGEX MATCH "(|seq)(halcon|HALCON)xl(|_static)" _T ${_HALCON_LIBRARY})
    if(_T)
      message(STATUS "Using HALCON XL")
      set(HALCON_XL ON)
    else()
      set(HALCON_XL OFF)
    endif()
  endif()
  if(NOT DEFINED HALCON_STATIC)
    string(REGEX MATCH "(|seq)(halcon|HALCON)(|xl)_static" _T ${_HALCON_LIBRARY})
    if(_T)
      message(STATUS "Using static HALCON")
      set(HALCON_STATIC ON)
    else()
      set(HALCON_STATIC OFF)
    endif()
  endif()

  # Infer the suffix used for 'bin' and 'lib' subdirectories from the location
  # of the HALCON library found.
  set(HALCON_BUILD_SUFFIX "")
  if(_HALCON_LIBDIR MATCHES "/libt/${HALCON_ARCHITECTURE}$")
    set(HALCON_BUILD_SUFFIX "t")
  elseif(_HALCON_LIBDIR MATCHES "/libd/${HALCON_ARCHITECTURE}$")
    set(HALCON_BUILD_SUFFIX "d")
  endif()
endif()

# Now set _HALCON_PREFIX and _HALCON_SUFFIX based on the variant to find the
# HALCON language interfaces. Also set any other special properties based
# on the variant. Note that for the IMPORTED_NO_SONAME property to work
# correctly, CMake must know what type of library is being imported. We assume
# that HALCON_STATIC has been set correctly at this point.
set(_HALCON_PREFIX)
set(_HALCON_SUFFIX)
if(HALCON_SEQUENTIAL)
  string(CONCAT _HALCON_PREFIX "seq" ${_HALCON_PREFIX})
endif()
if(HALCON_XL)
  string(APPEND _HALCON_SUFFIX "xl")
  list(APPEND _HALCON_COMPILE_DEFINITIONS "HC_LARGE_IMAGES")
endif()
if(HALCON_STATIC)
  string(APPEND _HALCON_SUFFIX "_static")
  list(APPEND _HALCON_COMPILE_DEFINITIONS "HC_STATIC_VARIANT")
  set(_LIBTYPE "STATIC")
else()
  set(_LIBTYPE "SHARED")
endif()

# If HALCON_INCLUDE_DIR is not set, look for it now. Where to look depends on
# where we found the HALCON library.
if(NOT HALCON_INCLUDE_DIR)
  if(APPLE AND HALCON_LIBRARY MATCHES "\\.framework$")
    find_path(HALCON_INCLUDE_DIR
      NAMES Halcon.h
      PATHS ${HALCON_LIBRARY}/Headers
      NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  elseif(NOT HALCON_DIR)
    find_path(HALCON_INCLUDE_DIR NAMES Halcon.h)
  else()
    find_path(HALCON_INCLUDE_DIR
      NAMES Halcon.h
      PATHS ${HALCON_DIR}/include
      NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  endif()
endif()

# If we found the HALCON headers, get the HALCON version by parsing the
# HVersNum.h header file. The version information is cached.
if(NOT HALCON_VERSION AND HALCON_INCLUDE_DIR)
  # Get the HALCON version by parsing HVersNum.h
  file(STRINGS ${HALCON_INCLUDE_DIR}/HVersNum.h _HALCON_HVERSNUM
    REGEX "^#[ ]*define[ ]+HLIB_")
  string(REGEX MATCH "HLIB_MAJOR_NUM[ \t]+([0-9]+)" _HALCON_DUMMY
    ${_HALCON_HVERSNUM})
  set(HALCON_VERSION_MAJOR ${CMAKE_MATCH_1} CACHE INTERNAL "HALCON major version")
  string(REGEX MATCH "HLIB_MINOR_NUM[ \t]+([0-9]+)" _HALCON_DUMMY
    ${_HALCON_HVERSNUM})
  set(HALCON_VERSION_MINOR ${CMAKE_MATCH_1} CACHE INTERNAL "HALCON minor version")
  string(REGEX MATCH "HLIB_REVISION_NUM[ \t]+([0-9]+)" _HALCON_DUMMY
    ${_HALCON_HVERSNUM})
  set(HALCON_VERSION_PATCH ${CMAKE_MATCH_1} CACHE INTERNAL "HALCON patch version")
  string(REGEX MATCH "HLIB_BUILD_NUM[ \t]+([0-9]+)" _HALCON_DUMMY
    ${_HALCON_HVERSNUM})
  set(HALCON_VERSION_TWEAK ${CMAKE_MATCH_1} CACHE INTERNAL "HALCON hotfix version")
  set(HALCON_VERSION
    ${HALCON_VERSION_MAJOR}.${HALCON_VERSION_MINOR}.${HALCON_VERSION_PATCH}.${HALCON_VERSION_TWEAK}
    CACHE INTERNAL "HALCON version")
endif()

# For Linux Intel versions of HALCON prior to 18.05, the SONAME was not set,
# and so the linker will record the HALCON libraries with absolute path names
# in the ELF binary, which is definitely not what we want. If the
# IMPORTED_NO_SONAME property is set, cmake will use a -L/-l combination for
# the library instead of passing the absolute path to the linker, avoiding
# the problem. Note that while many HALCON Embedded ports prior to 18.05 did
# set the SONAME, we can still set IMPORTED_NO_SONAME without issues because
# this property only tells CMake to not use absolute paths for the shared
# library, but the linker itself will still use the SONAME if it finds it in
# the binary. As a result, we can make this check depend only on the HALCON
# version and do not need to figure out a way to determine if a shared object
# contains an SONAME or not.
if(HALCON_VERSION VERSION_LESS "18.05")
  set(_NO_SONAME 1)
else()
  set(_NO_SONAME 0)
endif()

# If we where able to find the main HALCON library and its headers, look for
# other library components in the same location.
if(HALCON_Halcon_FOUND AND HALCON_INCLUDE_DIR)
  if("CInt" IN_LIST HALCON_FIND_COMPONENTS)
    # If HALCON_STATIC_CINT is true, the HALCON C language interfaces should
    # be static, even if HALCON itself is a dynamic library.
    if (HALCON_STATIC_CINT AND NOT HALCON_STATIC)
      set(_CINT_SUFFIX ${_HALCON_SUFFIX}_static)
      set(_CINT_LIBTYPE "STATIC")
    else()
      set(_CINT_SUFFIX ${_HALCON_SUFFIX})
      set(_CINT_LIBTYPE ${_LIBTYPE})
    endif()
    set(_CINT_COMPILE_DEFINITIONS ${_HALCON_COMPILE_DEFINITIONS})
    find_library(HALCON_CInt_LIBRARY
      NAMES ${_HALCON_PREFIX}halconc${_CINT_SUFFIX} HALCONC${_CINT_SUFFIX}
      PATHS ${_HALCON_LIBDIR}
      NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
    if(HALCON_CInt_LIBRARY)
      set(HALCON_CInt_FOUND TRUE)
      find_package_message(HALCON
        "Found HALCON C language interface: ${HALCON_CInt_LIBRARY}"
        "[${HALCON_CInt_LIBRARY}][${HALCON_INCLUDE_DIR}]")
    endif()
  endif()

  if("CppInt" IN_LIST HALCON_FIND_COMPONENTS)
    # If HALCON_STATIC_CPPINT is true, the HALCON C++ language interfaces should
    # be static, even if HALCON itself is a dynamic library.
    if (HALCON_STATIC_CPPINT AND NOT HALCON_STATIC)
      set(_CPPINT_SUFFIX ${_HALCON_SUFFIX}_static)
      set(_CPPINT_LIBTYPE "STATIC")
    else()
      set(_CPPINT_SUFFIX ${_HALCON_SUFFIX})
      set(_CPPINT_LIBTYPE ${_LIBTYPE})
    endif()
    set(_CPPINT_COMPILE_DEFINITIONS ${_HALCON_COMPILE_DEFINITIONS})
    find_library(HALCON_CppInt_LIBRARY
      NAMES ${_HALCON_PREFIX}halconcpp${_CPPINT_SUFFIX} HALCONCpp${_CPPINT_SUFFIX}
      PATHS ${_HALCON_LIBDIR}
      NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
    if(HALCON_CppInt_LIBRARY)
      set(HALCON_CppInt_FOUND TRUE)
      find_package_message(HALCON
        "Found HALCON C++ language interface: ${HALCON_CppInt_LIBRARY}"
        "[${HALCON_CppInt_LIBRARY}][${HALCON_INCLUDE_DIR}]")
    endif()
  endif()

  if("HDevEngine" IN_LIST HALCON_FIND_COMPONENTS)
    find_library(HALCON_HDevEngine_LIBRARY
      NAMES ${_HALCON_PREFIX}hdevenginecpp${_HALCON_SUFFIX}
            HDevEngineCpp${_HALCON_SUFFIX}
      PATHS ${_HALCON_LIBDIR}
      NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
    if(HALCON_HDevEngine_LIBRARY)
      set(HALCON_HDevEngine_FOUND TRUE)
      find_package_message(HALCON
        "Found HALCON HDevEngine: ${HALCON_HDevEngine_LIBRARY}"
        "[${HALCON_CHALCON_HDevEngine_LIBRARY}][${HALCON_INCLUDE_DIR}]")
    endif()
  endif()

  # Set the search path for .NET assemblies. Since CMake supports C# only for
  # Visual Studio 2010 or later, we always use the 3.5 assemblies.
  set(_HALCON_NET_DIR)
  if(HALCON_DIR)
    list(APPEND _HALCON_NET_DIR "${HALCON_DIR}/bin/dotnet35")
  else()
    list(APPEND _HALCON_NET_DIR "${_HALCON_LIBDIR}/../../bin/dotnet35")
  endif()

  if("DotNetInt" IN_LIST HALCON_FIND_COMPONENTS)
    find_file(HALCON_DotNetInt_ASSEMBLY
      NAMES ${_HALCON_PREFIX}halcondotnet${_HALCON_SUFFIX}.dll
      PATHS ${_HALCON_NET_DIR}
      NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
    if(HALCON_DotNetInt_ASSEMBLY)
      set(HALCON_DotNetInt_FOUND TRUE)
      find_package_message(HALCON
        "Found HALCON .NET interface: ${HALCON_DotNetInt_ASSEMBLY}"
        "[${HALCON_DotNetInt_ASSEMBLY}]")
    endif()
  endif()

  if("HDevEngineDotNet" IN_LIST HALCON_FIND_COMPONENTS)
    find_file(HALCON_HDevEngineDotNet_ASSEMBLY
      NAMES ${_HALCON_PREFIX}hdevenginedotnet${_HALCON_SUFFIX}.dll
      PATHS ${_HALCON_NET_DIR}
      NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
    if(HALCON_HDevEngineDotNet_ASSEMBLY)
      set(HALCON_HDevEngineDotNet_FOUND TRUE)
      find_package_message(HALCON
        "Found HALCON HDevEngine .NET interface: ${HALCON_HDevEngineDotNet_ASSEMBLY}"
        "[${HALCON_HDevEngineDotNet_ASSEMBLY}]")
    endif()
  endif()
endif()

# Now create import targets for use.
if(HALCON_LIBRARY AND NOT TARGET HALCON::Halcon)
  add_library(HALCON::Halcon ${_LIBTYPE} IMPORTED)
  if(APPLE AND ${HALCON_LIBRARY} MATCHES "\\.framework$")
    set_target_properties(HALCON::Halcon PROPERTIES
      IMPORTED_LOCATION "${HALCON_LIBRARY}/HALCON${_HALCON_SUFFIX}"
      INTERFACE_COMPILE_DEFINITIONS "${_HALCON_COMPILE_DEFINITIONS}"
      INTERFACE_COMPILE_OPTIONS     "-F${_HALCON_LIBDIR}")
  elseif(WIN32 AND NOT HALCON_STATIC)
    set_target_properties(HALCON::Halcon PROPERTIES
      IMPORTED_IMPLIB               "${HALCON_LIBRARY}"
      INTERFACE_COMPILE_DEFINITIONS "${_HALCON_COMPILE_DEFINITIONS}"
      INTERFACE_INCLUDE_DIRECTORIES "${HALCON_INCLUDE_DIR}")
  else()
    set_target_properties(HALCON::Halcon PROPERTIES
      IMPORTED_LOCATION             "${HALCON_LIBRARY}"
      IMPORTED_NO_SONAME            "${_NO_SONAME}"
      INTERFACE_COMPILE_DEFINITIONS "${_HALCON_COMPILE_DEFINITIONS}"
      INTERFACE_INCLUDE_DIRECTORIES "${HALCON_INCLUDE_DIR}")
  endif()

  # If we're using a static HALCON, we should define _HLibStatic. Note
  # that early versions of CMake don't allow using target_compile_definitions
  # on IMPORTED targets, so we set the property directly instead.
  if(HALCON_STATIC)
    set_property(TARGET HALCON::Halcon APPEND PROPERTY
      INTERFACE_COMPILE_DEFINITIONS _HLibStatic
    )
  endif()

  # Starting with 20.05, the HALCON library may contain C++ code internally.
  # This means that for a static HALCON library, we must set the
  # IMPORTED_LINK_INTERFACE_LANGUAGES property correctly so that CMake knows
  # the C++ linker must be used. Unfortunately, this is not enough - the C++
  # language must also be enabled for the project, which we try to ensure by
  # calling enable_language().
  if(HALCON_VERSION GREATER_EQUAL 20 AND HALCON_STATIC)
    enable_language(CXX)
    set_target_properties(HALCON::Halcon PROPERTIES
      IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
    )
  endif()

  if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    # On Android HALCON 20.11 and later use the Android logging support.
    if(HALCON_VERSION GREATER_EQUAL 20.11)
      set_property(TARGET HALCON::Halcon APPEND PROPERTY
        INTERFACE_LINK_LIBRARIES "-llog")
    endif()
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND HALCON_STATIC)
    # On macOS when using a static HALCON library, we will need to add a
    # number of additional system libraries and frameworks to link against.
    set_property(TARGET HALCON::Halcon APPEND PROPERTY INTERFACE_LINK_LIBRARIES
      "-liconv"
      "-lldap"
      "-framework AppKit"
      "-framework CoreFoundation"
      "-framework Cocoa"
      "-framework DiskArbitration"
      "-framework Foundation"
      "-framework IOKit"
      "-framework OpenCL"
      "-framework OpenGL"
      "-framework Security"
      "-framework SecurityFoundation"
    )
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # On Linux, we may possibly need to link against additional system
    # libraries when linking against Halcon.
    if(NOT HALCON_SEQUENTIAL)
      set_property(TARGET HALCON::Halcon APPEND PROPERTY
        INTERFACE_LINK_LIBRARIES "-lpthread")
    endif()
    set_property(TARGET HALCON::Halcon APPEND PROPERTY
      INTERFACE_LINK_LIBRARIES "-ldl;-lrt")
    if(HALCON_STATIC)
      set_property(TARGET HALCON::Halcon APPEND PROPERTY
        INTERFACE_LINK_LIBRARIES "-lm")
    endif()
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows" AND HALCON_STATIC)
    # When linking HALCON statically on Windows, we need to link against some
    # Windows system libraries explicitly.
    set_property(TARGET HALCON::Halcon APPEND PROPERTY INTERFACE_LINK_LIBRARIES
      "Comctl32.lib;Iphlpapi.lib;Setupapi.lib;Winmm.lib;Ws2_32.lib"
    )
  endif()
endif()

if(HALCON_CInt_LIBRARY AND NOT TARGET HALCON::CInt)
  add_library(HALCON::CInt ${_CINT_LIBTYPE} IMPORTED)
  if (APPLE AND ${HALCON_CInt_LIBRARY} MATCHES "\\.framework$")
    set_target_properties(HALCON::CInt PROPERTIES
      IMPORTED_LOCATION
        "${HALCON_CInt_LIBRARY}/HALCONC${_HALCON_SUFFIX}"
      INTERFACE_COMPILE_DEFINITIONS "${_HALCON_COMPILE_DEFINITIONS}"
      INTERFACE_COMPILE_OPTIONS     "-F${_HALCON_LIBDIR}"
      INTERFACE_LINK_LIBRARIES      HALCON::Halcon)
  elseif(WIN32 AND NOT HALCON_STATIC AND NOT HALCON_STATIC_CINT)
    set_target_properties(HALCON::CInt PROPERTIES
      IMPORTED_IMPLIB               "${HALCON_CInt_LIBRARY}"
      INTERFACE_COMPILE_DEFINITIONS "${_HALCON_COMPILE_DEFINITIONS}"
      INTERFACE_INCLUDE_DIRECTORIES "${HALCON_INCLUDE_DIR}/halconc"
      INTERFACE_LINK_LIBRARIES      HALCON::Halcon)
  else()
    set_target_properties(HALCON::CInt PROPERTIES
      IMPORTED_LOCATION             "${HALCON_CInt_LIBRARY}"
      IMPORTED_NO_SONAME            "${_NO_SONAME}"
      INTERFACE_COMPILE_DEFINITIONS "${_HALCON_COMPILE_DEFINITIONS}"
      INTERFACE_INCLUDE_DIRECTORIES "${HALCON_INCLUDE_DIR}/halconc"
      INTERFACE_LINK_LIBRARIES      HALCON::Halcon)
  endif()
  if(HALCON_STATIC OR HALCON_STATIC_CINT)
    set_property(TARGET HALCON::CInt APPEND PROPERTY
      INTERFACE_COMPILE_DEFINITIONS _LIntStatic
    )
  endif()
endif()

if(HALCON_CppInt_LIBRARY AND NOT TARGET HALCON::CppInt)
  add_library(HALCON::CppInt ${_CPPINT_LIBTYPE} IMPORTED)
  if (APPLE AND ${HALCON_CppInt_LIBRARY} MATCHES "\\.framework$")
    set_target_properties(HALCON::CppInt PROPERTIES
      IMPORTED_LOCATION
        "${HALCON_CppInt_LIBRARY}/HALCONCpp${_HALCON_SUFFIX}"
      INTERFACE_COMPILE_DEFINITIONS "${_HALCON_COMPILE_DEFINITIONS}"
      INTERFACE_COMPILE_OPTIONS     "-F${_HALCON_LIBDIR}"
      INTERFACE_LINK_LIBRARIES      HALCON::Halcon
    )
  elseif(WIN32 AND NOT HALCON_STATIC AND NOT HALCON_STATIC_CPPINT)
    set_target_properties(HALCON::CppInt PROPERTIES
      IMPORTED_IMPLIB               "${HALCON_CppInt_LIBRARY}"
      INTERFACE_COMPILE_DEFINITIONS "${_HALCON_COMPILE_DEFINITIONS}"
      INTERFACE_INCLUDE_DIRECTORIES "${HALCON_INCLUDE_DIR}/halconcpp"
      INTERFACE_LINK_LIBRARIES      HALCON::Halcon
    )
  else()
    set_target_properties(HALCON::CppInt PROPERTIES
      IMPORTED_LOCATION             "${HALCON_CppInt_LIBRARY}"
      IMPORTED_NO_SONAME            "${_NO_SONAME}"
      INTERFACE_COMPILE_DEFINITIONS "${_HALCON_COMPILE_DEFINITIONS}"
      INTERFACE_INCLUDE_DIRECTORIES "${HALCON_INCLUDE_DIR}/halconcpp"
      INTERFACE_LINK_LIBRARIES      HALCON::Halcon
    )
  endif()
  if(HALCON_STATIC OR HALCON_STATIC_CPPINT)
    set_property(TARGET HALCON::CppInt APPEND PROPERTY
      INTERFACE_COMPILE_DEFINITIONS _LIntStatic
    )
  endif()
endif()

if(HALCON_HDevEngine_LIBRARY AND NOT TARGET HALCON::HDevEngine)
  add_library(HALCON::HDevEngine ${_LIBTYPE} IMPORTED)
  if (APPLE AND ${HALCON_HDevEngine_LIBRARY} MATCHES "\\.framework$")
    set_target_properties(HALCON::HDevEngine PROPERTIES
      IMPORTED_LOCATION
        "${HALCON_HDevEngine_LIBRARY}/HDevEngineCpp${_HALCON_SUFFIX}"
      INTERFACE_COMPILE_DEFINITIONS "${_HALCON_COMPILE_DEFINITIONS}"
      INTERFACE_COMPILE_OPTIONS     "-F${_HALCON_LIBDIR}"
      INTERFACE_LINK_LIBRARIES      "HALCON::CInt;HALCON::CppInt"
    )
  elseif(WIN32 AND NOT HALCON_STATIC)
    set_target_properties(HALCON::HDevEngine PROPERTIES
      IMPORTED_IMPLIB               "${HALCON_HDevEngine_LIBRARY}"
      INTERFACE_COMPILE_DEFINITIONS "${_HALCON_COMPILE_DEFINITIONS}"
      INTERFACE_INCLUDE_DIRECTORIES "${HALCON_INCLUDE_DIR}/hdevengine"
      INTERFACE_LINK_LIBRARIES      "HALCON::CInt;HALCON::CppInt"
    )
  else()
    set_target_properties(HALCON::HDevEngine PROPERTIES
      IMPORTED_LOCATION             "${HALCON_HDevEngine_LIBRARY}"
      IMPORTED_NO_SONAME            "${_NO_SONAME}"
      INTERFACE_COMPILE_DEFINITIONS "${_HALCON_COMPILE_DEFINITIONS}"
      INTERFACE_INCLUDE_DIRECTORIES "${HALCON_INCLUDE_DIR}/hdevengine"
      INTERFACE_LINK_LIBRARIES      "HALCON::CInt;HALCON::CppInt"
    )
  endif()
  if(HALCON_STATIC)
    set_property(TARGET HALCON::HDevEngine APPEND PROPERTY
      INTERFACE_COMPILE_DEFINITIONS _LIntStatic
    )
  endif()
endif()

# For finding the hcomp or HDevelop executables, we must find the HALCON
# binary directories for the host architecture.
set(_HALCON_BIN)
if(HALCON_DIR AND HALCON_HOST_ARCHITECTURE)
  if(NOT HALCON_BUILD_SUFFIX STREQUAL "")
    list(APPEND _HALCON_BIN
      "${HALCON_DIR}/bin${HALCON_BUILD_SUFFIX}/${HALCON_HOST_ARCHITECTURE}"
    )
  endif()
  list(APPEND _HALCON_BIN "${HALCON_DIR}/bin/${HALCON_HOST_ARCHITECTURE}")
elseif(_HALCON_LIBDIR AND HALCON_HOST_ARCHITECTURE)
  if(NOT HALCON_BUILD_SUFFIX STREQUAL "")
    list(APPEND _HALCON_BIN
      "${_HALCON_LIBDIR}/../../bin${HALCON_BUILD_SUFFIX}/${HALCON_HOST_ARCHITECTURE}"
    )
  endif()
  list(APPEND _HALCON_BIN "${_HALCON_LIBDIR}/../../bin/${HALCON_HOST_ARCHITECTURE}")
endif()
# On macOS, an installed version of HALCON will place the bin directory in
# /Library/Application Support/HALCON-<version>/bin
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
  if(HALCON_VERSION_MAJOR VERSION_LESS 17 OR HALCON_VERSION_PATCH VERSION_GREATER 0)
    list(APPEND _HALCON_BIN
      "/Library/Application Support/HALCON-${HALCON_VERSION_MAJOR}.${HALCON_VERSION_MINOR}/bin")
  else()
    list(APPEND _HALCON_BIN
      "/Library/Application Support/HALCON-${HALCON_VERSION_MAJOR}.${HALCON_VERSION_MINOR}-Progress/bin")
  endif()
endif()

if("hcomp" IN_LIST HALCON_FIND_COMPONENTS)
  if(NOT _HALCON_BIN)
    find_program(HALCON_hcomp_EXECUTABLE NAMES hcomp)
  else()
    find_program(HALCON_hcomp_EXECUTABLE
      NAMES hcomp
      PATHS ${_HALCON_BIN}
      NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  endif()
  if(HALCON_hcomp_EXECUTABLE)
    set(HALCON_hcomp_FOUND TRUE)
    find_package_message(HALCON
      "Found HALCON hcomp executable: ${HALCON_hcomp_EXECUTABLE}"
      "[${HALCON_hcomp_EXECUTABLE}]")
  endif()
endif()

if("HDevelop" IN_LIST HALCON_FIND_COMPONENTS)
  if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    set(_HDEVELOP_PATHS)
    foreach(_bin IN LISTS _HALCON_BIN)
      list(APPEND _HDEVELOP_PATHS "${_bin}/hdevelop.app/Contents/MacOS")
    endforeach()
    set(_HDEVELOP_APP "HDevelop")
    if(HALCON_XL)
      string(APPEND _HDEVELOP_APP " XL")
    endif()
    if(HALCON_VERSION_MAJOR VERSION_LESS 17)
      string(APPEND _HDEVELOP_APP " ${HALCON_VERSION_MAJOR}.app")
    elseif(HALCON_VERSION_PATCH VERSION_LESS 1)
      string(APPEND _HDEVELOP_APP
        " ${HALCON_VERSION_MAJOR}.${HALCON_VERSION_MINOR} Progress.app")
    else()
      string(APPEND _HDEVELOP_APP
        " ${HALCON_VERSION_MAJOR}.${HALCON_VERSION_MINOR}.app")
    endif()
    list(APPEND _HDEVELOP_PATHS "/Applications/${_HDEVELOP_APP}/Contents/MacOS")
    find_program(HALCON_HDevelop_EXECUTABLE
      NAMES hdevelop
      PATHS ${_HDEVELOP_PATHS}
      NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
  else()
    if(NOT _HALCON_BIN)
      find_program(HALCON_HDevelop_EXECUTABLE NAMES hdevelop${_HALCON_SUFFIX})
    else()
      find_program(HALCON_HDevelop_EXECUTABLE
        NAMES hdevelop${_HALCON_SUFFIX}
        PATHS ${_HALCON_BIN}
        NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
    endif()
  endif()
  if(HALCON_HDevelop_EXECUTABLE)
    set(HALCON_HDevelop_FOUND TRUE)
    find_package_message(HALCON
      "Found HALCON hdevelop executable: ${HALCON_HDevelop_EXECUTABLE}"
      "[${HALCON_HDevelop_EXECUTABLE}]")
  endif()
endif()

# Handle standard parameters of find_package
find_package_handle_standard_args(HALCON
  FOUND_VAR HALCON_FOUND
  REQUIRED_VARS HALCON_INCLUDE_DIR HALCON_LIBRARY
  VERSION_VAR HALCON_VERSION
  HANDLE_COMPONENTS)

# For some reason CMake 3.19.5 requires IMPORTED_LOCATION to be set if HALCON
# is used together with Qt5, even though for imported Windows DLLs,
# IMPORTED_IMPLIB is what is required for linking. It does not seem necessary
# to set IMPORTED_LOCATION to the actual DLL - setting it to the .lib file
# seems to work. It is also only necessary to set this property for libraries
# that are linked directly via target_link_libraries, not for libraries linked
# via a transitive INTERFACE_LINK_LIBRARIES property. Note that CMake 3.16 did
# not have this bizarre issue.
if(WIN32 AND CMAKE_VERSION VERSION_GREATER_EQUAL 3.19)
  if(TARGET HALCON::Halcon)
    set_target_properties(HALCON::Halcon PROPERTIES
      IMPORTED_LOCATION "${HALCON_LIBRARY}"
    )
  endif()
  if(TARGET HALCON::CInt)
    set_target_properties(HALCON::CInt PROPERTIES
      IMPORTED_LOCATION "${HALCON_CInt_LIBRARY}"
    )
  endif()
  if(TARGET HALCON::CppInt)
    set_target_properties(HALCON::CppInt PROPERTIES
      IMPORTED_LOCATION "${HALCON_CppInt_LIBRARY}"
    )
  endif()
  if(TARGET HALCON::HDevEngine)
    set_target_properties(HALCON::HDevEngine PROPERTIES
      IMPORTED_LOCATION "${HALCON_HDevEngine_LIBRARY}"
    )
  endif()
endif()
