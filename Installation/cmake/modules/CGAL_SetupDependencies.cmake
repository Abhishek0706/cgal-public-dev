include(${CGAL_MODULES_DIR}/CGAL_Macros.cmake)
include(CMakeParseArguments)

function(CGAL_external_library)
  set(options REQUIRED WARN_MISSING)
  set(oneValueArgs NAME PREFIX VERSION)
  set(multiValueArgs) # no multi-value args
  cmake_parse_arguments(CGAL_external_library "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  option(WITH_${CGAL_external_library_NAME} 
    "Enable support for the external library ${CGAL_external_library_NAME}" ON)

  list(APPEND CGAL_EXTERNAL_LIBRARIES ${CGAL_external_library_NAME})
  set(CGAL_EXTERNAL_LIBRARIES ${CGAL_EXTERNAL_LIBRARIES} PARENT_SCOPE)

  if(CGAL_external_library_REQUIRED)
    set(CGAL_${CGAL_external_library_NAME}_REQUIRED TRUE PARENT_SCOPE)
  else()
    set(CGAL_${CGAL_external_library_NAME}_REQUIRED FALSE PARENT_SCOPE)
  endif()

  if(CGAL_external_library_WARN_MISSING)
    set(CGAL_${CGAL_external_library_NAME}_WARN_MISSING TRUE PARENT_SCOPE)
  else()
    set(CGAL_${CGAL_external_library_NAME}_WARN_MISSING FALSE PARENT_SCOPE)
  endif()

  if("${CGAL_external_library_PREFIX}" STREQUAL "")
    # The name is the prefix
    set(CGAL_${CGAL_external_library_NAME}_PREFIX ${CGAL_external_library_NAME} PARENT_SCOPE)
  else()
    set(CGAL_${CGAL_external_library_NAME}_PREFIX ${CGAL_external_library_PREFIX} PARENT_SCOPE)
  endif()

  if(NOT "${CGAL_external_library_VERSION}" STREQUAL "")
    set(CGAL_${CGAL_external_library_NAME}_VERSION ${CGAL_external_library_VERSION} PARENT_SCOPE)
  endif()
endfunction()

set(CGAL_EXTERNAL_LIBRARIES)
# This is the place to tell which external libs are supported.
# Boost is non-negotiable
CGAL_external_library(NAME Boost
  REQUIRED
  VERSION 1.33.1)
CGAL_external_library(NAME Boost_system
  REQUIRED
  PREFIX Boost_SYSTEM
  VERSION 1.33.1)
CGAL_external_library(NAME Boost_thread
  REQUIRED 
  PREFIX Boost_THREAD 
  VERSION 1.33.1)
CGAL_external_library(NAME Boost_program_options
  PREFIX Boost_PROGRAM_OPTIONS 
  VERSION 1.33.1)
CGAL_external_library(NAME GMP WARN_MISSING)
CGAL_external_library(NAME MPFR WARN_MISSING)
CGAL_external_library(NAME Qt3)
CGAL_external_library(NAME Qt4
  PREFIX QT)
CGAL_external_library(NAME ZLIB)
CGAL_external_library(NAME OpenGL
  PREFIX OPENGL)
CGAL_external_library(NAME LEDA)
CGAL_external_library(NAME MPFI)
CGAL_external_library(NAME RS)
CGAL_external_library(NAME RS3)
CGAL_external_library(NAME OpenNL)
CGAL_external_library(NAME TAUCS)
CGAL_external_library(NAME Eigen3
  PREFIX EIGEN3)
CGAL_external_library(NAME BLAS)
CGAL_external_library(NAME LAPACK)
CGAL_external_library(NAME QGLVIEWER)
CGAL_external_library(NAME ESBTL)
CGAL_external_library(NAME Coin3D
  PREFIX COIN3D)
CGAL_external_library(NAME NTL)
CGAL_external_library(NAME IPE)
#  There exists FindF2C, FindMKL, but they are only used to
#  support supporting libs.
if(NOT WIN32)
  # GMPXX is not supported on WIN32 machines
  CGAL_external_library(NAME GMPXX)
endif()

include(${CGAL_MODULES_DIR}/CGAL_TweakFindBoost.cmake)

function(CGAL_setup_dependency lib)
  set(vlib ${CGAL_${lib}_PREFIX})
  
  if(${lib} MATCHES "Boost_")
    string(REPLACE "Boost_" "" component ${lib})
    find_package(Boost ${CGAL_${lib}_VERSION} QUIET COMPONENTS ${component})
  else()
    find_package(${lib} ${CGAL_${lib}_VERSION} QUIET)
  endif()
  # Turn off the output-because this is too noisy.
  # message(STATUS "  Use${lib}-file:      ${${vlib}_USE_FILE}") 
  # message(STATUS "  ${lib} include:      ${${vlib}_INCLUDE_DIR}")
  # message(STATUS "  ${lib} libraries:    ${${vlib}_LIBRARIES}")
  # message(STATUS "  ${lib} definitions:  ${${vlib}_DEFINITIONS}")
  CGAL_up_if_defined(${vlib}_FOUND)
  CGAL_up_if_defined(${vlib}_USE_FILE)
  CGAL_up_if_defined(${vlib}_INCLUDE_DIR)
  CGAL_up_if_defined(${vlib}_LIBRARIES)
  CGAL_up_if_defined(${vlib}_DEFINITIONS)
    
  # Clean the Boost_LIBRARIES variable from the last find, to prevent a
  # package that just uses Boost to link against this
  unset(Boost_LIBRARIES)
endfunction()

# Special handling still required.
#
# if (${lib} STREQUAL "GMP") 
#   get_dependency_version(GMP)
# endif()

# if (${lib} STREQUAL "MPFR") 
#   set( MPFR_DEPENDENCY_INCLUDE_DIR ${GMP_INCLUDE_DIR} )
#   set( MPFR_DEPENDENCY_LIBRARIES   ${GMP_LIBRARIES} )
#   get_dependency_version(MPFR)
# endif()

# if (${lib} STREQUAL "LEDA") 
#   # special case for LEDA - add a flag
#   message( STATUS "$LEDA cxx flags:   ${LEDA_CXX_FLAGS}" )
#   uniquely_add_flags( CMAKE_CXX_FLAGS ${LEDA_CXX_FLAGS} )
# endif()
