cmake_minimum_required(VERSION 3.17)

if(NOT DEFINED PROJECT_NAME)
	set(IS_NOT_SUBPROJECT ON)
endif()

project(template_lib
	VERSION 0.0.1
	DESCRIPTION "Templated C Library"
	HOMEPAGE_URL ""
	LANGUAGES "C"
)

#
# Options
#
option(TL_TESTS "Build the unit tests when enabled." ON)

#
# Configuration
#
include(GNUInstallDirs)

set(CMAKE_C_STANDARD 99)
set(TL_TARGET "${PROJECT_NAME}")
set(TL_INCLUDE_BUILD_DIR "${PROJECT_SOURCE_DIR}/src")
set(TL_INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/")

#
# Targets
#
add_library(${TL_TARGET} INTERFACE)
target_include_directories(
		${TL_TARGET} INTERFACE
		$<BUILD_INTERFACE:${TL_INCLUDE_BUILD_DIR}>
		$<INSTALL_INTERFACE:>
)

#
# Enable Testing
#
if(TL_TESTS)
	enable_testing()
	add_subdirectory(tests)
endif()

#
# Installation
#
if(IS_NOT_SUBPROJECT)
	install(
		DIRECTORY ${TL_INCLUDE_BUILD_DIR}
		DESTINATION ${TL_INCLUDE_INSTALL_DIR}
	)
endif(IS_NOT_SUBPROJECT)

