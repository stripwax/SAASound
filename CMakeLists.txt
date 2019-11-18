cmake_minimum_required(VERSION 3.0)
project(SAASound)

set(MAJOR_VERSION 3)
set(MINOR_VERSION 3)
set(PATCH_VERSION 0)

option(BUILD_SHARED_LIBS "Build a dynamic library instead of static library" ON)

if (APPLE)
  option(BUILD_FRAMEWORK "Build a Mac OS X framework instead of a shared library" OFF)
  set(CMAKE_OSX_ARCHITECTURES "i386;x86_64")
  set(BUILD_SHARED_LIBS ON)
endif ()

file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
file(GLOB API_HEADERS include/*.h)

if (MSVC)
  configure_file(win32/${PROJECT_NAME}.rc.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.rc @ONLY)
  set(RESOURCES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.rc win32/${PROJECT_NAME}.def)
endif()

add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${API_HEADERS} ${RESOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
  VERSION ${MAJOR_VERSION}.${MINOR_VERSION}
  SOVERSION ${MAJOR_VERSION}
  CXX_STANDARD 11)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_BINARY_DIR}
  include)

if (APPLE AND BUILD_FRAMEWORK)
  set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME SAASound
    FRAMEWORK TRUE
    FRAMEWORK_VERSION C
    MACOSX_FRAMEWORK_IDENTIFIER org.rebuzz)
  set_source_files_properties(${API_HEADERS} PROPERTIES
    MACOSX_PACKAGE_LOCATION Headers)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9")
endif ()

configure_file(src/config.h.in ${CMAKE_BINARY_DIR}/config.h)
target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_CONFIG_H=1)

install(TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  FRAMEWORK DESTINATION "/Library/Frameworks"
  PUBLIC_HEADER DESTINATION include)

if (NOT APPLE OR NOT BUILD_FRAMEWORK)
  install(FILES ${API_HEADERS} DESTINATION include)
endif ()
