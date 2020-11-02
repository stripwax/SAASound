cmake_minimum_required(VERSION 3.0)
project(SAASound)

if (APPLE)
  option(BUILD_FRAMEWORK "Build a Mac OS X framework instead of a shared library" OFF)
  set(CMAKE_OSX_ARCHITECTURES "i386;x86_64")
endif ()

set(EXTERNAL_CLK_HZ "8000000" CACHE STRING "External clock in Hz")
option(SAAFREQ_FIXED_CLOCKRATE "Use precomputed frequency table, external clock is locked" OFF)
set(SAMPLE_RATE_HZ "44100" CACHE STRING "Output sample rate in Hz")
set(DEFAULT_OVERSAMPLE "0" CACHE STRING "Default oversample factor")
option(USE_CONFIG_FILE "Compile support for loading a config file on startup (SAAsound.cfg)" ON)
include(CMakeDependentOption)
cmake_dependent_option(CONFIG_FILE_PATH "Path to config file" "SAASound.cfg" "USE_CONFIG_FILE" "")

#include(CheckStructHasMember)
#CHECK_STRUCT_HAS_MEMBER("struct dirent" d_type dirent.h HAVE_STRUCT_DIRENT_D_TYPE LANGUAGE CXX)

file(GLOB SOURCES src/*.cpp LIST_DIRECTORIES false)
file(GLOB HEADERS src/*.h LIST_DIRECTORIES false)
file(GLOB API_HEADERS include/*.h LIST_DIRECTORIES false)

if (MSVC)
  set(RESOURCES resources/SAASound.def)
endif ()

add_library(SAASound SHARED ${SOURCES} ${HEADERS} ${API_HEADERS} ${RESOURCES})

set_target_properties(SAASound PROPERTIES
  VERSION 3.4
  SOVERSION 3
  CXX_STANDARD 11)

target_include_directories(SAASound PRIVATE
  ${CMAKE_BINARY_DIR}
  include)

if (APPLE AND BUILD_FRAMEWORK)
  set_target_properties(SAASound PROPERTIES
    OUTPUT_NAME SAASound
    FRAMEWORK TRUE
    FRAMEWORK_VERSION C
    MACOSX_FRAMEWORK_IDENTIFIER org.rebuzz)
  set_source_files_properties(${API_HEADERS} PROPERTIES
    MACOSX_PACKAGE_LOCATION Headers)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9")
endif ()

configure_file(src/config.h.in ${CMAKE_BINARY_DIR}/config.h)
target_compile_definitions(SAASound PRIVATE HAVE_CONFIG_H=1)

install(TARGETS SAASound
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  FRAMEWORK DESTINATION "/Library/Frameworks"
  PUBLIC_HEADER DESTINATION include)

if (NOT APPLE OR NOT BUILD_FRAMEWORK)
  install(FILES ${API_HEADERS} DESTINATION include)
endif ()
