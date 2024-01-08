#
# Copyright (c) 2024 Blynk Technologies Inc.
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(blynk_over_zephyr)

target_sources(app PRIVATE src/main.c)

zephyr_get_include_directories_for_lang_as_string(       C includes)
zephyr_get_system_include_directories_for_lang_as_string(C system_includes)
zephyr_get_compile_definitions_for_lang_as_string(       C definitions)
zephyr_get_compile_options_for_lang_as_string(           C options)

set(blynk_ncp_cflags
  "${includes} ${definitions} ${options} ${system_includes}"
  )

include(ExternalProject)

# Add an external project to be able download and build the third
# party library. In this case downloading is not necessary as it has
# been committed to the repository.
set(blynk_ncp_src_dir   ${CMAKE_CURRENT_SOURCE_DIR}/../../blynk_ncp)
set(blynk_ncp_build_dir ${CMAKE_CURRENT_BINARY_DIR}/blynk_ncp)

set(BLYNK_NCP_DIR     ${blynk_ncp_build_dir}/lib)
set(BLYNK_INCLUDE_DIR ${blynk_ncp_src_dir}/src/BlynkNcpDriver/src)

if(CMAKE_GENERATOR STREQUAL "Unix Makefiles")
# https://www.gnu.org/software/make/manual/html_node/MAKE-Variable.html
set(submake "$(MAKE)")
else() # Obviously no MAKEFLAGS. Let's hope a "make" can be found somewhere.
set(submake "make")
endif()

ExternalProject_Add(
  blynk_example                 # Name for custom target
  PREFIX     ${blynk_ncp_build_dir} # Root dir for entire project
  SOURCE_DIR ${blynk_ncp_src_dir}
  BINARY_DIR ${blynk_ncp_src_dir} # This particular build system is invoked from the root
  CONFIGURE_COMMAND ""    # Skip configuring the project, e.g. with autoconf
  BUILD_COMMAND
  ${submake}
  PREFIX=${blynk_ncp_build_dir}
  CC=${CMAKE_C_COMPILER}
  AR=${CMAKE_AR}
  CFLAGS=${blynk_ncp_cflags}
  INSTALL_COMMAND ""      # This particular build system has no install command
  BUILD_BYPRODUCTS ${BLYNK_NCP_DIR}/libblynkncp.a
  DEPENDS zephyr_interface
  )

# Create a wrapper CMake library that our app can link with
add_library(blynk_ncp_lib STATIC IMPORTED GLOBAL)
add_dependencies(
  blynk_ncp_lib
  blynk_example
  )

list(APPEND LIB_INCLUDE_DIRS ${BLYNK_INCLUDE_DIR})
list(APPEND LIB_INCLUDE_DIRS ${blynk_ncp_src_dir}/include)

set_target_properties(blynk_ncp_lib PROPERTIES IMPORTED_LOCATION             ${BLYNK_NCP_DIR}/libblynkncp.a)
set_target_properties(blynk_ncp_lib PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${LIB_INCLUDE_DIRS}")

target_link_libraries(app PUBLIC blynk_ncp_lib)
target_link_libraries(blynk_ncp_lib INTERFACE kernel)
