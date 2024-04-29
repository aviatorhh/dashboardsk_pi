# ~~~
# Summary:      Local, non-generic plugin setup
# Copyright (c) 2020-2021 Mike Rossiter
# License:      GPLv3+
# ~~~

# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 3 of the License, or (at your option) any later
# version.

# -------- Options ----------

set(OCPN_TEST_REPO
    "nohal/opencpn-plugins"
    CACHE STRING "Default repository for untagged builds")
set(OCPN_BETA_REPO
    "nohal/dashboardsk_pi-beta"
    CACHE STRING "Default repository for tagged builds matching 'beta'")
set(OCPN_RELEASE_REPO
    "nohal/dashboardsk_pi-stable"
    CACHE STRING "Default repository for tagged builds not matching 'beta'")

#
# -------  Plugin setup --------
#
set(PKG_NAME DashboardSK_pi)
set(PKG_VERSION "0.3.0")
set(PKG_PRERELEASE "beta") # Empty, or a tag like 'beta'

set(DISPLAY_NAME DashboardSK) # Dialogs, installer artifacts, ...
set(PLUGIN_API_NAME DashboardSK) # As of GetCommonName() in plugin API
set(PKG_SUMMARY "DashboardSK plugin for OpenCPN")
set(PKG_DESCRIPTION
    [=[
DashboardSK plugin for OpenCPN
]=])

set(PKG_AUTHOR "Pavel Kalian")
set(PKG_IS_OPEN_SOURCE "yes")
set(PKG_HOMEPAGE https://github.com/nohal/dashboardsk_pi)
set(PKG_INFO_URL https://opencpn.org/OpenCPN/plugins/dashboardSK.html)

option(WITH_TESTS "Whether or not to build the tests" OFF)
option(SANITIZE "What sanitizers to use" "")

if(NOT "${SANITIZE}" STREQUAL "OFF" AND NOT "${SANITIZE}" STREQUAL "")
  add_compile_options(-fsanitize=${SANITIZE} -fno-omit-frame-pointer)
  add_link_options(-fsanitize=${SANITIZE} -fno-omit-frame-pointer)
endif()

add_definitions(-DDASHBOARDSK_USE_SVG)
add_definitions(-DocpnUSE_GL)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(HDR_DASHBOARD
    ${CMAKE_SOURCE_DIR}/include/dashboardsk.h
    ${CMAKE_SOURCE_DIR}/include/dashboard.h
    ${CMAKE_SOURCE_DIR}/include/instrument.h
    ${CMAKE_SOURCE_DIR}/include/simplenumberinstrument.h
    ${CMAKE_SOURCE_DIR}/include/simplegaugeinstrument.h
    ${CMAKE_SOURCE_DIR}/include/simpletextinstrument.h
    ${CMAKE_SOURCE_DIR}/include/simplepositioninstrument.h
    ${CMAKE_SOURCE_DIR}/include/simplehistograminstrument.h
    ${CMAKE_SOURCE_DIR}/include/zone.h
    ${CMAKE_SOURCE_DIR}/include/displayscale.h
    ${CMAKE_SOURCE_DIR}/include/pager.h)
set(SRC_DASHBOARD
    ${CMAKE_SOURCE_DIR}/src/dashboardsk.cpp
    ${CMAKE_SOURCE_DIR}/src/dashboard.cpp
    ${CMAKE_SOURCE_DIR}/src/instrument.cpp
    ${CMAKE_SOURCE_DIR}/src/simplenumberinstrument.cpp
    ${CMAKE_SOURCE_DIR}/src/simplegaugeinstrument.cpp
    ${CMAKE_SOURCE_DIR}/src/simpletextinstrument.cpp
    ${CMAKE_SOURCE_DIR}/src/simplepositioninstrument.cpp
    ${CMAKE_SOURCE_DIR}/src/simplehistograminstrument.cpp
    ${CMAKE_SOURCE_DIR}/src/pager.cpp)

set(SRC_GUI_DESKTOP ${CMAKE_SOURCE_DIR}/src/dashboardskgui.cpp
                    ${CMAKE_SOURCE_DIR}/src/dashboardskguiimpl.cpp)
set(SRC_GUI_ANDROID ${CMAKE_SOURCE_DIR}/src/dashboardskguiandroid.cpp
                    ${CMAKE_SOURCE_DIR}/src/dashboardskguiimpl.cpp)

if(QT_ANDROID)
  set(SRC_GUI ${SRC_GUI_ANDROID})
else()
  set(SRC_GUI ${SRC_GUI_DESKTOP})
endif()

set(SRC ${SRC_DASHBOARD} ${SRC_GUI} ${CMAKE_SOURCE_DIR}/src/dashboardsk_pi.cpp)

set(PKG_API_LIB api-18) # A dir in opencpn-libs/ e. g., api-17 or api-16

macro(late_init)
  # Perform initialization after the PACKAGE_NAME library, compilers and
  # ocpn::api is available.

  # Fix OpenGL deprecated warnings in Xcode
  target_compile_definitions(${PACKAGE_NAME} PRIVATE GL_SILENCE_DEPRECATION)
  # Prepare doxygen config
  configure_file(${CMAKE_SOURCE_DIR}/doc/Doxyfile.in
                 ${CMAKE_BINARY_DIR}/Doxyfile)
  configure_file(${CMAKE_SOURCE_DIR}/doc/header.html.in
                 ${CMAKE_BINARY_DIR}/header.html)
  # Prepare asciidoxy
  configure_file(${CMAKE_SOURCE_DIR}/doc/api.adoc.in
                 ${CMAKE_BINARY_DIR}/api.adoc @ONLY)
  configure_file(${CMAKE_SOURCE_DIR}/doc/packages.toml
                 ${CMAKE_BINARY_DIR}/packages.toml)
  configure_file(${CMAKE_SOURCE_DIR}/doc/contents.toml
                 ${CMAKE_BINARY_DIR}/contents.toml)
endmacro()

macro(add_plugin_libraries)
  # Add libraries required by this plugin
  if(WIN32)
    add_subdirectory("${CMAKE_SOURCE_DIR}/opencpn-libs/WindowsHeaders")
    target_link_libraries(${PACKAGE_NAME} windows::headers)

    # add_subdirectory("${CMAKE_SOURCE_DIR}/opencpn-libs/glu")
    # target_link_libraries(${PACKAGE_NAME} ocpn::glu_static)
  endif()

  add_subdirectory("${CMAKE_SOURCE_DIR}/opencpn-libs/plugin_dc")
  target_link_libraries(${PACKAGE_NAME} ocpn::plugin-dc)

  add_subdirectory("opencpn-libs/wxJSON")
  target_link_libraries(${PACKAGE_NAME} ocpn::wxjson)
  if(${WITH_TESTS})
    include(CTest)
    add_subdirectory("${CMAKE_SOURCE_DIR}/tests")
    add_dependencies(${CMAKE_PROJECT_NAME} tests)
  endif()
endmacro()

add_custom_target(
  doxygen-docs
  COMMAND doxygen
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

add_custom_target(
  asciidoxy-docs
  COMMAND asciidoxy -D apidocs --spec-file packages.toml api.adoc
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  DEPENDS doxygen-docs)
