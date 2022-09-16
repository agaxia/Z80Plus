# Z80Plus - CMakeLists.txt
#  ______  ______ ______    ___
# /\___  \/\  __ \\  __ \ _/\  \___
# \/__/  /\_\  __ \\ \/\ \\___   __\
#    /\_____\\_____\\_____\__/\__\_/
#    \/_____//_____//_____/  \/__/
# Copyright (C) 2021-2022 Sofía Ortega Sosa.
# Released under the terms of the MIT license.

cmake_minimum_required(VERSION 3.14) # install(TARGETS ARCHIVE [DESTINATION])

if(	CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR AND
	NOT CMAKE_BUILD_TYPE				   AND
	NOT CMAKE_CONFIGURATION_TYPES
)
	set(CMAKE_BUILD_TYPE Release)
endif()

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/API/Z80Plus.hpp" _)
string(REGEX MATCH ".*Z80_PLUS_LIBRARY_VERSION_STRING \"([^\n]*)\".*" _ ${_})

project(Z80Plus
	VERSION ${CMAKE_MATCH_1}
	LANGUAGES CXX
	DESCRIPTION "")

unset(_)
message("${PROJECT_NAME} v${PROJECT_VERSION}")

include(GNUInstallDirs)

if(DEFINED ${PROJECT_NAME}_SHARED_LIBS)
	set(BUILD_SHARED_LIBS "${${PROJECT_NAME}_SHARED_LIBS}")
endif()

set(${PROJECT_NAME}_INSTALL_CMAKEDIR
"${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}" CACHE STRING
"Specify the directory in which to install the CMake config-file package.")

set(${PROJECT_NAME}_INSTALL_PKGCONFIGDIR
"${CMAKE_INSTALL_LIBDIR}/pkgconfig" CACHE STRING
"Specify the directory in which to install the pkg-config file.")

option(${PROJECT_NAME}_WITH_CMAKE_SUPPORT
"Generate and install the CMake config-file package."
NO)

option(${PROJECT_NAME}_WITH_PKGCONFIG_SUPPORT
"Generate and install the pkg-config file."
NO)

option(${PROJECT_NAME}_WITH_STANDARD_DOCUMENTS
"Install the standard text documents distributed with the package: \
AUTHORS, LICENSE, HISTORY and README.md."
NO)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")
find_package(Z80 REQUIRED)

add_library(
	${PROJECT_NAME}
	"${CMAKE_CURRENT_SOURCE_DIR}/API/Z80Plus.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/sources/Z80Plus.cpp")

set_target_properties(
	${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 11
	CXX_STANDARD_REQUIRED YES
	VERSION	${PROJECT_VERSION}
	SOVERSION ${PROJECT_VERSION_MAJOR}
	DEBUG_POSTFIX "-debug"
	PUBLIC_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/API/Z80Plus.hpp")

target_link_libraries(${PROJECT_NAME} PUBLIC Z80)

target_include_directories(
	${PROJECT_NAME} PUBLIC
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/API>"
	"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

if(BUILD_SHARED_LIBS)
	if(WIN32)
		if(PROJECT_VERSION_PATCH STREQUAL "")
			set(PROJECT_VERSION_PATCH 0)
		endif()

		configure_file(
			"${CMAKE_CURRENT_SOURCE_DIR}/sources/Z80Plus.rc.in"
			"${PROJECT_BINARY_DIR}/Z80Plus.rc"
			@ONLY)

		target_sources(${PROJECT_NAME} PRIVATE "${PROJECT_BINARY_DIR}/Z80Plus.rc")
	endif()
else()
	target_compile_definitions(${PROJECT_NAME} PUBLIC Z80_STATIC)
endif()

install(TARGETS ${PROJECT_NAME}
	EXPORT "${PROJECT_NAME}_Targets"
	RUNTIME COMPONENT "${PROJECT_NAME}_Runtime"
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
		COMPONENT "${PROJECT_NAME}_Runtime"
		NAMELINK_COMPONENT "${PROJECT_NAME}_Development"
	ARCHIVE COMPONENT "${PROJECT_NAME}_Development"
	PUBLIC_HEADER
		DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
		COMPONENT "${PROJECT_NAME}_Development")

if(${PROJECT_NAME}_WITH_CMAKE_SUPPORT OR ${PROJECT_NAME}_WITH_PKGCONFIG_SUPPORT)
	include(CMakePackageConfigHelpers)

	if(${PROJECT_NAME}_WITH_CMAKE_SUPPORT)
		if (BUILD_SHARED_LIBS)
			set(_type Shared)
		else()
			set(_type Static)
		endif()

		install(EXPORT "${PROJECT_NAME}_Targets"
			DESTINATION ${${PROJECT_NAME}_INSTALL_CMAKEDIR}
			FILE ${PROJECT_NAME}${_type}Targets.cmake
			COMPONENT ${PROJECT_NAME}_Development)

		unset(_type)

		write_basic_package_version_file(
			"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
			VERSION ${PROJECT_VERSION}
			COMPATIBILITY SameMajorVersion)

		configure_package_config_file(
			"${CMAKE_CURRENT_SOURCE_DIR}/support/${PROJECT_NAME}Config.cmake.in"
			"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
			INSTALL_DESTINATION ${${PROJECT_NAME}_INSTALL_CMAKEDIR})

		install(FILES
			"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
			"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
			DESTINATION ${${PROJECT_NAME}_INSTALL_CMAKEDIR}
			COMPONENT ${PROJECT_NAME}_Development)
	endif()

	if(${PROJECT_NAME}_WITH_PKGCONFIG_SUPPORT)
		configure_file(
			"${CMAKE_CURRENT_SOURCE_DIR}/support/${PROJECT_NAME}.pc.in"
			"${PROJECT_BINARY_DIR}/${PROJECT_NAME}.pc"
			@ONLY)

		install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc"
			DESTINATION ${${PROJECT_NAME}_INSTALL_PKGCONFIGDIR}
			COMPONENT ${PROJECT_NAME}_Development)
	endif()
endif()

if(${PROJECT_NAME}_WITH_STANDARD_DOCUMENTS)
	install(FILES	"${CMAKE_CURRENT_SOURCE_DIR}/AUTHORS"
			"${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
			"${CMAKE_CURRENT_SOURCE_DIR}/HISTORY"
			"${CMAKE_CURRENT_SOURCE_DIR}/README.md"
		DESTINATION ${CMAKE_INSTALL_DOCDIR}
		COMPONENT "${PROJECT_NAME}_Runtime")
endif()

# CMakeLists.txt EOF
