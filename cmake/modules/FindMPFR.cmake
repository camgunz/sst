# Try to find the MPFR library
# See http://www.mpfr.org/
#
# This module supports requiring a minimum version, e.g. you can do
#   find_package(MPFR 2.3.0)
# to require version 2.3.0 to newer of MPFR.
#
# Once done this will define
#
#  MPFR_FOUND - system has MPFR lib with correct version
#  MPFR_INCLUDE_DIR - the MPFR include directory
#  MPFR_LIBRARIES - the MPFR library
#  MPFR_VERSION - MPFR version

# Copyright (c) 2006, 2007 Montel Laurent, <montel@kde.org>
# Copyright (c) 2008, 2009 Gael Guennebaud, <g.gael@free.fr>
# Copyright (c) 2010 Jitse Niesen, <jitse@maths.leeds.ac.uk>
# Redistribution and use is allowed according to the terms of the BSD license.

# Set MPFR_INCLUDE_DIR

FIND_PATH(MPFR_INCLUDE_DIR
  NAMES mpfr.h
  PATHS
  $ENV{MPFRDIR} ${INCLUDE_INSTALL_DIR}
)

# Set MPFR_FIND_VERSION to 1.0.0 if no minimum version is specified

IF(NOT MPFR_FIND_VERSION)
  IF(NOT MPFR_FIND_VERSION_MAJOR)
    SET(MPFR_FIND_VERSION_MAJOR 1)
  ENDIF(NOT MPFR_FIND_VERSION_MAJOR)
  IF(NOT MPFR_FIND_VERSION_MINOR)
    SET(MPFR_FIND_VERSION_MINOR 0)
  ENDIF(NOT MPFR_FIND_VERSION_MINOR)
  IF(NOT MPFR_FIND_VERSION_PATCH)
    SET(MPFR_FIND_VERSION_PATCH 0)
  ENDIF(NOT MPFR_FIND_VERSION_PATCH)

  SET(MPFR_FIND_VERSION "${MPFR_FIND_VERSION_MAJOR}")
  SET(MPFR_FIND_VERSION "${MPFR_FIND_VERSION}.${MPFR_FIND_VERSION_MINOR}")
  SET(MPFR_FIND_VERSION "${MPFR_FIND_VERSION}.${MPFR_FIND_VERSION_PATCH}")
ENDIF()


IF(MPFR_INCLUDE_DIR)

  # Set MPFR_VERSION
  
  FILE(READ "${MPFR_INCLUDE_DIR}/mpfr.h" _mpfr_version_header)
  
  STRING(REGEX MATCH "define[ \t]+MPFR_VERSION_MAJOR[ \t]+([0-9]+)"
      _mpfr_major_version_match "${_mpfr_version_header}")
  SET(MPFR_MAJOR_VERSION "${CMAKE_MATCH_1}")
  STRING(REGEX MATCH "define[ \t]+MPFR_VERSION_MINOR[ \t]+([0-9]+)"
         _mpfr_minor_version_match "${_mpfr_version_header}")
  SET(MPFR_MINOR_VERSION "${CMAKE_MATCH_1}")
  STRING(REGEX MATCH "define[ \t]+MPFR_VERSION_PATCHLEVEL[ \t]+([0-9]+)"
         _mpfr_patchlevel_version_match "${_mpfr_version_header}")
  SET(MPFR_PATCHLEVEL_VERSION "${CMAKE_MATCH_1}")
  
  SET(MPFR_VERSION ${MPFR_MAJOR_VERSION})
  SET(MPFR_VERSION "${MPFR_VERSION}.${MPFR_MINOR_VERSION}")
  SET(MPFR_VERSION "${MPFR_VERSION}.${MPFR_PATCHLEVEL_VERSION}")
  
  # Check whether found version exceeds minimum version
  
  IF(${MPFR_VERSION} VERSION_LESS ${MPFR_FIND_VERSION})
    SET(MPFR_VERSION_OK FALSE)
    MESSAGE(STATUS
      "MPFR version ${MPFR_VERSION} found in "
      "${MPFR_INCLUDE_DIR}, but at least version "
      "${MPFR_FIND_VERSION} is required"
    )
    ELSE(${MPFR_VERSION} VERSION_LESS ${MPFR_FIND_VERSION})
    SET(MPFR_VERSION_OK TRUE)
  ENDIF(${MPFR_VERSION} VERSION_LESS ${MPFR_FIND_VERSION})
ENDIF(MPFR_INCLUDE_DIR)

# Set MPFR_LIBRARIES

FIND_LIBRARY(MPFR_LIBRARIES
  mpfr
  PATHS
  $ENV{MPFRDIR} ${LIB_INSTALL_DIR})

# Epilogue

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
  MPFR
  DEFAULT_MSG
  MPFR_INCLUDE_DIR
  MPFR_LIBRARIES
  MPFR_VERSION_OK
)

MARK_AS_ADVANCED(
  MPFR_INCLUDE_DIR
  MPFR_LIBRARIES
)

