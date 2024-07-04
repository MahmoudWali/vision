# UseHalcon.cmake
# Copyright (c) 2022-2023 by MVTec Software GmbH
#
# Include this file to use HALCON in your projects. This file includes some
# extra settings that aren't required by HALCON per se, but are required when
# compiling code exported from HDevelop.

# Make sure this file is only included once
if(__USE_HALCON_INCLUDED)
  return()
endif()
set(__USE_HALCON_INCLUDED ${CMAKE_CURRENT_LIST_FILE})
message(STATUS "Including ${CMAKE_CURRENT_LIST_FILE}")

if(NOT DEFINED HALCON_DIR AND NOT $ENV{HALCONROOT} STREQUAL "")
  # If HALCON_DIR is not set but HALCONROOT is set in the environment, set
  # HALCON_DIR.
  file(TO_CMAKE_PATH "$ENV{HALCONROOT}" HALCON_DIR)
  message(STATUS "Setting HALCON_DIR to ${HALCON_DIR} from environment")
endif()

if(NOT DEFINED HALCON_ARCHITECTURE AND NOT $ENV{HALCONARCH} STREQUAL "")
  # If HALCON_ARCHITECTURE is not set but HALCONARCH is set in the environment,
  # set HALCON_ARCHITECTURE.
  set(HALCON_ARCHITECTURE "$ENV{HALCONARCH}")
  message(STATUS
    "Setting HALCON_ARCHITECTURE to ${HALCON_ARCHITECTURE} from environment"
  )
endif()

# Look for HALCON libraries. Since we don't know which components will be
# required, make them all OPTIONAL_COMPONENTS.
find_package(HALCON REQUIRED OPTIONAL_COMPONENTS
  CInt CppInt HDevEngine DotNetInt HDevEngineDotNet hcomp)

# If WITHOUT_CODE_EXPORT is false, look for a HDevelop executable to use to
# export code with.
option(WITHOUT_CODE_EXPORT "Do not run HDevelop to export code" OFF)
if(NOT WITHOUT_CODE_EXPORT)
  find_package(HALCON OPTIONAL_COMPONENTS HDevelop)
endif()

# Get the library the main HALCON library is located in, as we may need it
# for linking.
get_filename_component(HALCON_LIBRARY_DIR "${HALCON_LIBRARY}" DIRECTORY)

# Option to control how RPATH is set for Linux executables. If WITHOUT_RPATH is
# true, we don't want to set the RPATH. If it is false, we only want to set the
# RPATH for the HALCON libraries, not any other paths. This is necessary
# because we do *not* want the executable to use the system libraries out of
# the compiler toolchain's sysroot under any circumstances.
option(WITHOUT_RPATH "Do not set RPATH in Linux executables" ON)
if(WITHOUT_RPATH OR HALCON_STATIC)
  set(CMAKE_SKIP_RPATH TRUE)
  set(CMAKE_SKIP_INSTALL_RPATH TRUE)
else()
  set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
  set(CMAKE_INSTALL_RPATH ${HALCON_LIBRARY_DIR})
endif()

# On macOS, binaries using the hardened runtime will not use
# /Library/Frameworks as a default RPATH, which means such binaries will not
# find HALCON frameworks installed there (the HALCON frameworks use an install
# name based in @rpath, not an absolute path). We can avoid this problem by
# adding /Library/Frameworks manually. You can disable this feature with the
# WITHOUT_OSX_DEFAULT_RPATH option.
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  option(WITHOUT_OSX_DEFAULT_PATH
    "Do not set default RPATH to /Library/Frameworks" OFF
  )
  if(NOT WITHOUT_OSX_DEFAULT_RPATH AND WITHOUT_RPATH AND NOT HALCON_STATIC)
    set(CMAKE_SKIP_INSTALL_RPATH OFF)
    string(APPEND CMAKE_INSTALL_RPATH "/Library/Frameworks")
  endif()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  # On Darwin systems we must link against the CoreFoundation framework to
  # link code exported from HDevelop.
  set_property(TARGET HALCON::Halcon APPEND PROPERTY
    INTERFACE_LINK_LIBRARIES "-framework CoreFoundation")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  # On Linux systems, DT_RPATH overrides LD_LIBRARY_PATH, so we really want to
  # be using DT_RUNPATH, which does not. However, CMake doesn't know about
  # DT_RUNPATH, and the GNU linker doesn't have a command line option to set it
  # directly in any case. Instead, we use the --enable-new-dtags linker option
  # to tell the linker to create DT_RUNPATH instead of DT_RPATH tags.
  #
  # Also, to avoid linker warnings on some Linux systems, add the directory
  # the HALCON libraries are in with -rpath-link.
  string(APPEND CMAKE_EXE_LINKER_FLAGS
      " -Wl,--enable-new-dtags -Wl,-rpath-link ${HALCON_LIBRARY_DIR}")
endif()


# HALCON_ExportCode <exported> SOURCE <source>
#                              [FALLBACK <fallback source>]
#
# Add a target to export code from the HDevelop script <source>. If <source> is
# not an absolute path, it is assumed to be relative to
# CMAKE_CURRENT_SOURCE_DIR. The exported code is saved to the file <exported>.
# If <exported> is not an absolute path, it is assumed to be relative to
# CMAKE_CURRENT_BINARY_DIR. If FALLBACK is specified and no suitable HDevelop
# executable can be found, the <fallback source> file is simply copied to
# <exported>. If <fallback source> is not an absolute path, it is assumed to be
# relative to CMAKE_CURRENT_SOURCE_DIR.
#
# If the option WITHOUT_CODE_EXPORT is true, HALCON_ExportCode will not attempt
# to run HDevelop to export any code and always use the fallback (if specified)
# instead.
function(HALCON_ExportCode exported)
  cmake_parse_arguments(_ARG "VERBOSE" "SOURCE;FALLBACK" "" ${ARGN})
  if(DEFINED _ARG_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unparsed arguments: ${_ARG_UNPARSED_ARGUMENTS}")
  endif()
  if(NOT DEFINED _ARG_SOURCE)
    message(FATAL_ERROR "Missing SOURCE file")
  endif()

  get_filename_component(exported "${exported}"
    ABSOLUTE BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}"
  )
  get_filename_component(_exported_name "${exported}" NAME)
  get_filename_component(_ARG_SOURCE "${_ARG_SOURCE}"
    ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
  )

  if(DEFINED _ARG_FALLBACK)
    get_filename_component(_ARG_FALLBACK "${_ARG_FALLBACK}"
      ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
    )
  endif()

  if(NOT WITHOUT_CODE_EXPORT AND HALCON_HDevelop_FOUND)
    # HDevelop is not a console application, therefore it immediately returns
    # after being called. This seems to be a problem on some Windows machines,
    # where cmake/ninja hangs after starting the export below. Using start /wait
    # causes the caller to wait until the program finishes.
    set(_wait_for_exit)
    if(CMAKE_HOST_WIN32)
      set(_wait_for_exit start /wait)
    endif()
    # Use native paths to call HDevelop
    file(TO_NATIVE_PATH "${exported}" _exported_native)
    file(TO_NATIVE_PATH "${_ARG_SOURCE}" _source_native)
    add_custom_command(OUTPUT ${exported}
      MAIN_DEPENDENCY ${_ARG_SOURCE}
      DEPENDS         ${HALCON_HDevelop_EXECUTABLE}
      COMMAND         ${_wait_for_exit} ${HALCON_HDevelop_EXECUTABLE}
                        -convert ${_source_native} ${_exported_native}
      COMMENT         "Exporting ${_exported_name}"
    )
  elseif(DEFINED _ARG_FALLBACK)
    add_custom_command(OUTPUT ${exported}
      MAIN_DEPENDENCY ${_ARG_FALLBACK}
      COMMAND         ${CMAKE_COMMAND} -E copy_if_different
                        ${_ARG_FALLBACK} ${exported}
      COMMENT         "Copying ${_exported_name}"
    )
  else()
    message(FATAL_ERROR "No HDevelop executable for the build system found")
  endif()
endfunction()


# HALCON_AddExtensionPackage name DEF_FILES <def1> [ ... <defn> ]
#                                 SOURCES <src1> [ ... <srcn> ]
#                                 [INCLUDE_DIRECTORIES <path1> [ ... <pathn> ] ]
#                                 [INSTALL_FILES <path1> [ ... <pathn> ] ]
#                                 [LANGUAGES <lang1> [ ... <langn> ] ]
#                                 [CLASSES <class1> [ ... <classn> ] ]
#                                 [OPERATORS <operator1> [ ... <operatorn> ] ]
#                                 [CHAPTERS <chapter> [ ... <chaptern> ] ]
#
# Add a target to build a HALCON extension package <name> and the necessary
# install commands to install the extension in the format HALCON expects it.
#
# Specify the .def files to use with DEF_FILES, and the source files with
# SOURCES. Extra (private) include directories can be specified using the
# INCLUDE_DIRECTORIES argument. The main library target for the extension is
# called <name>; this can be used to add additional attributes after calling
# HALCON_AddExtensionPackage.
#
# Additional files that need to be installed for the extension package are
# specified with INSTALL_FILES. The files are installed keeping their relative
# location to CMAKE_CURRENT_SOURCE_DIR in the installation directory.
#
# The LANGUAGES argument is used to determine which language interfaces are
# built for the extension package. If not specified, the GLOBAL property
# ENABLED_LANGUAGES is used by default. Supported languages are C and CXX, and
# CSharp if the CMake version being used has the required language support. The
# targets for the language interface libraries are called <name>cint,
# <name>cppint, and <name>dotnetint, respectively.
#
# The CLASSES and OPERATORS arguments are used to specify the classes and
# operators that are defined by this extension package. This information is
# necessary to correctly build the HTML reference documention for the extension
# package, as CMake cannot easily determine this from the .def files itself.
# If CLASSES and/or OPERATORS are not specified, the dependency rules for the
# reference documentation will be incomplete and the reference documentation
# may not be built correctly. The value(s) of the CLASSES argument should be
# the names of the class definitions in your .def files. Note that
# HALCON_AddExtensionPackage has no way to check for missing or extraneous
# classes or operators, so it is important to take extra care to get these
# right.
#
# The CHAPTERS arguments are used to specify the chapters to add the package's
# operators to. Like CLASSES and OPERATORS this is necessary to correctly build
# the HTML reference documentation. Specify the names of all the chapters your
# def files reference with the chapter.english keyword.
#
# HALCON_AddExtensionPackage will set the CMake properties C_VISIBILITY_PRESET
# and CXX_VISIBILITY_PRESET to 'hidden', and VISIBILITY_INLINES_HIDDEN to 'ON'
# for the main library target and the C and C++ language interface library
# targets.
function(HALCON_AddExtensionPackage name)
  cmake_parse_arguments(_ARG
    "VERBOSE" ""
    "DEF_FILES;INCLUDE_DIRECTORIES;SOURCES;INSTALL_FILES;LANGUAGES;CLASSES;OPERATORS;CHAPTERS"
    ${ARGN}
  )
  if(DEFINED _ARG_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unparsed arguments: ${_ARG_UNPARSED_ARGUMENTS}")
  endif()

  if(NOT DEFINED _ARG_DEF_FILES)
    message(FATAL_ERROR "Missing argument: DEF_FILES")
  endif()
  if(NOT DEFINED _ARG_SOURCES)
    message(FATAL_ERROR "Missing argument: SOURCES")
  endif()

  # If the LANGUAGES are not defined, use the ENABLED_LANGUAGES global property
  if(NOT DEFINED _ARG_LANGUAGES)
    get_property(_ARG_LANGUAGES GLOBAL PROPERTY ENABLED_LANGUAGES)
  endif()

  # We need a hcomp executable to build an extension package
  if(NOT HALCON_hcomp_EXECUTABLE)
    message(FATAL_ERROR
      "Building extension packages requires the hcomp executable")
  endif()

  if(HALCON_XL)
    set(_suffix "xl")
  endif()

  # Get full native paths to the def files for hcomp
  foreach(_file IN LISTS _ARG_DEF_FILES)
    get_filename_component(_path ${_file} ABSOLUTE)
    file(TO_NATIVE_PATH ${_path} _npath)
    list(APPEND _pkg_defs "${_npath}")
  endforeach()

  # Generate operator help files for this extension package
  set(_pkg_help_dir "${CMAKE_CURRENT_BINARY_DIR}/help")
  set(_pkg_help
    "${_pkg_help_dir}/operators_en_US.idx"
    "${_pkg_help_dir}/operators_en_US.key"
    "${_pkg_help_dir}/operators_en_US.num"
    "${_pkg_help_dir}/operators_en_US.ref"
    "${_pkg_help_dir}/operators_en_US.sta"
  )
  add_custom_command(
    OUTPUT ${_pkg_help}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${_pkg_help_dir}
    COMMAND ${CMAKE_COMMAND} -E chdir ${_pkg_help_dir}
      ${HALCON_hcomp_EXECUTABLE} -u -M -p${name} ${_pkg_defs}
    DEPENDS ${_ARG_DEF_FILES} ${HALCON_hcomp_EXECUTABLE}
    COMMENT "Generating extension package help files for ${name}"
  )
  add_custom_target(${name}_help DEPENDS ${_pkg_help})

  # Add an install target for the help files.
  install(FILES ${_pkg_help} DESTINATION ${name}/help)

  # Generate the HTML operator reference documentation. We depend on
  # _ARG_OPERATORS and _ARG_CLASSES to be set correctly to get a list of all
  # the generated html files.
  set(_pkg_html_dir "${CMAKE_CURRENT_BINARY_DIR}/doc/html/reference")
  set(_pkg_html
    "${_pkg_html_dir}/index.html"
    "${_pkg_html_dir}/index_by_name.html"
    "${_pkg_html_dir}/index_classes.html"
  )
  foreach(_op IN LISTS _ARG_OPERATORS)
    list(APPEND _pkg_html "${_pkg_html_dir}/${_op}.html")
  endforeach()
  foreach(_class IN LISTS _ARG_CLASSES)
    # The class names as found in the .def file are converted into a filename
    # with the following algorithm:
    # 1. The first letter is transformed into upper case
    # 2. '_' is removed and the next character transformed into upper case
    # 3. A 'H' is prepended to the whole thing.
    set(_name "H")
    string(REPLACE "_" ";" _class "${_class}")
    foreach(_word IN LISTS _class)
      string(SUBSTRING "${_word}" 0 1 _first)
      string(TOUPPER "${_first}" _first)
      string(SUBSTRING "${_word}" 1 -1 _next)
      string(APPEND _name "${_first}${_next}")
    endforeach()
    list(APPEND _pkg_html "${_pkg_html_dir}/${_name}.html")
  endforeach()
  foreach(_chapter IN LISTS _ARG_CHAPTERS)
    string(TOLOWER "${_chapter}" _chapter)
    list(APPEND _pkg_html "${_pkg_html_dir}/toc_${_chapter}.html")
  endforeach()
  add_custom_command(
    OUTPUT ${_pkg_html}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${_pkg_html_dir}
    COMMAND ${CMAKE_COMMAND} -E chdir ${_pkg_html_dir}
      ${HALCON_hcomp_EXECUTABLE} -u -R -p${name} ${_pkg_defs}
    DEPENDS ${_ARG_DEF_FILES} ${HALCON_hcomp_EXECUTABLE}
    COMMENT "Generating extension package reference documentation for ${name}"
  )
  add_custom_target(${name}_html DEPENDS ${_pkg_html})

  # Add an install target for the HTML reference files.
  install(FILES ${_pkg_html} DESTINATION ${name}/doc/html/reference)

  # If INSTALL_FILES is specified, add install rules for the files mentioned
  # there.
  if(_ARG_INSTALL_FILES)
    foreach(_file IN LISTS _ARG_INSTALL_FILES)
      # Get the relative path of _file (relative to CMAKE_CURRENT_SOURCE_DIR).
      get_filename_component(_file "${_file}" REALPATH
        BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
      )
      file(RELATIVE_PATH _file ${CMAKE_CURRENT_SOURCE_DIR} ${_file})
      get_filename_component(_dir "${_file}" DIRECTORY)
      install(FILES ${_file} DESTINATION ${name}/${_dir})
      if(_ARG_VERBOSE)
        message(STATUS "Add install rule for ${_file} (to ${_dir})")
      endif()
    endforeach()
  endif()

  # This is the actual extension package.
  set(_pkg_main "${CMAKE_CURRENT_BINARY_DIR}/H${name}.c")
  add_custom_command(
    OUTPUT ${_pkg_main}
    COMMAND ${HALCON_hcomp_EXECUTABLE} -u -H -p${name} ${_pkg_defs}
    DEPENDS ${_ARG_DEF_FILES} ${HALCON_hcomp_EXECUTABLE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating extension package main for ${name}"
  )

  add_library(${name} SHARED ${_pkg_main} ${_ARG_SOURCES})
  if(DEFINED _ARG_INCLUDE_DIRECTORIES)
    target_include_directories(${name} PRIVATE ${_ARG_INCLUDE_DIRECTORIES})
  endif()
  set_target_properties(${name} PROPERTIES
    OUTPUT_NAME               "${name}${_suffix}"
    C_VISIBILITY_PRESET       hidden
    CXX_VISIBILITY_PRESET     hidden
    VISIBILITY_INLINES_HIDDEN ON
  )
  target_link_libraries(${name} HALCON::Halcon)

  # Add the documentation as a dependency on the main target so that it is
  # always built.
  add_dependencies(${name} ${name}_help ${name}_html)

  set(_install_targets ${name})

  # C language interface
  if("C" IN_LIST _ARG_LANGUAGES)
    set(_pkg_cint
      ${CMAKE_CURRENT_BINARY_DIR}/HC${name}.c
      ${CMAKE_CURRENT_BINARY_DIR}/HC${name}.h)
    add_custom_command(
      OUTPUT ${_pkg_cint}
      COMMAND ${HALCON_hcomp_EXECUTABLE} -u -C -p${name} ${_pkg_defs}
      DEPENDS ${_ARG_DEF_FILES} ${HALCON_hcomp_EXECUTABLE}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating C language interface for ${name}"
    )
    add_library(${name}cint SHARED ${_pkg_cint})
    target_include_directories(${name}cint PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
      $<INSTALL_INTERFACE:${name}/include>
    )
    set_target_properties(${name}cint PROPERTIES
       OUTPUT_NAME               "${name}c${_suffix}"
       PUBLIC_HEADER             "${CMAKE_CURRENT_BINARY_DIR}/HC${name}.h"
       C_VISIBILITY_PRESET       hidden
       CXX_VISIBILITY_PRESET     hidden
       VISIBILITY_INLINES_HIDDEN ON
     )
    target_link_libraries(${name}cint ${name} HALCON::CInt)
    list(APPEND _install_targets ${name}cint)
  endif()

  # C++ language interface
  if("CXX" IN_LIST _ARG_LANGUAGES)
    set(_pkg_cppint
      ${CMAKE_CURRENT_BINARY_DIR}/HCPP${name}.cpp
      ${CMAKE_CURRENT_BINARY_DIR}/HCPP${name}.h)
    add_custom_command(
      OUTPUT ${_pkg_cppint}
      COMMAND ${HALCON_hcomp_EXECUTABLE} -u -P -p${name} ${_pkg_defs}
      DEPENDS ${_ARG_DEF_FILES} ${HALCON_hcomp_EXECUTABLE}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating C++ language interface for ${name}"
    )
    add_library(${name}cppint SHARED ${_pkg_cppint})
    target_include_directories(${name}cppint PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
      $<INSTALL_INTERFACE:${name}/include>
    )
    set_target_properties(${name}cppint PROPERTIES
      OUTPUT_NAME               "${name}cpp${_suffix}"
      PUBLIC_HEADER             "${CMAKE_CURRENT_BINARY_DIR}/HCPP${name}.h"
      C_VISIBILITY_PRESET       hidden
      CXX_VISIBILITY_PRESET     hidden
      VISIBILITY_INLINES_HIDDEN ON
    )
    target_link_libraries(${name}cppint ${name} HALCON::CppInt)
    list(APPEND _install_targets ${name}cppint)
  endif()

  # Create the install rules for the shared libraries. For Windows systems
  # HALCON expects these to be in the 'bin' subdirectory instead of the 'lib'
  # subdirectory used for Unix-like systems.
  if(WIN32)
    install(TARGETS ${_install_targets}
      RUNTIME DESTINATION "${name}/bin${HALCON_BUILD_SUFFIX}/${HALCON_ARCHITECTURE}"
      LIBRARY DESTINATION "${name}/bin${HALCON_BUILD_SUFFIX}/${HALCON_ARCHITECTURE}"
      ARCHIVE DESTINATION "${name}/lib${HALCON_BUILD_SUFFIX}/${HALCON_ARCHITECTURE}"
      PUBLIC_HEADER DESTINATION "${name}/include"
    )
  else()
    install(TARGETS ${_install_targets}
      RUNTIME DESTINATION "${name}/lib${HALCON_BUILD_SUFFIX}/${HALCON_ARCHITECTURE}"
      LIBRARY DESTINATION "${name}/lib${HALCON_BUILD_SUFFIX}/${HALCON_ARCHITECTURE}"
      ARCHIVE DESTINATION "${name}/lib${HALCON_BUILD_SUFFIX}/${HALCON_ARCHITECTURE}"
      PUBLIC_HEADER DESTINATION "${name}/include"
    )
  endif()

  # C# language interface. Note that this requires CMake 3.8 or later, and
  # only works with the Visual Studio 10 2010 or later CMake generator.
  if("CSharp" IN_LIST _ARG_LANGUAGES)
    set(_pkg_dotnetint ${CMAKE_CURRENT_BINARY_DIR}/HDOTNET${name}.cs)
    add_custom_command(
      OUTPUT ${_pkg_dotnetint}
      COMMAND ${HALCON_hcomp_EXECUTABLE} -u -N -p${name} ${_pkg_defs}
      DEPENDS ${_ARG_DEF_FILES} ${HALCON_hcomp_EXECUTABLE}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating C# language interface for ${name}"
    )
    add_library(${name}dotnetint SHARED ${_pkg_dotnetint})
    set_target_properties(${name}dotnetint PROPERTIES
      OUTPUT_NAME "${name}dotnet${_suffix}"
    )
    # Add in some .NET reference libraries.
    set_property(TARGET ${name}dotnetint PROPERTY VS_DOTNET_REFERENCES
      "System;${HALCON_DotNetInt_ASSEMBLY}")

    install(TARGETS ${name}dotnetint
      RUNTIME DESTINATION "${name}/bin${HALCON_BUILD_SUFFIX}/dotnet35"
      LIBRARY DESTINATION "${name}/bin${HALCON_BUILD_SUFFIX}/dotnet35"
    )
  endif()
endfunction(HALCON_AddExtensionPackage)

# Extra stuff necessary to use HALCON static libraries. The deep learning
# functionality uses OpenMP with various compilers, making static linking more
# troublesome. Note that on Windows, we do not need to set a linker option
# explicitly.
if(HALCON_STATIC AND NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
  if(CMAKE_C_COMPILER_ID STREQUAL "Intel"
    OR CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    target_link_options(HALCON::Halcon INTERFACE "-qopenmp")
  endif()
endif()
