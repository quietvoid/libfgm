cmake_minimum_required(VERSION 3.0.0)

set (ProjectName "libfgm")

project (${ProjectName})

#Suppress creation of ZERO_CHECK
set(CMAKE_SUPPRESS_REGENERATION true)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Configs" FORCE)
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE PATH "Install prefix" FORCE)

if(MSVC)
  set(CMAKE_USE_RELATIVE_PATHS ON CACHE INTERNAL "" FORCE)
endif()

# ----------------------------------------------------------------------------
#  Check for c++11 support
# ----------------------------------------------------------------------------

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	if(COMPILER_SUPPORTS_CXX11)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
	elseif(COMPILER_SUPPORTS_CXX0X)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -pthread")
	else()
		message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no complete C++11 support. Please use a different C++ compiler.")
	endif()
else()
	if(MSVC)
		if(MSVC_VERSION LESS 1800)
			message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no complete C++11 support. Please use a different C++ compiler.")
		endif()
	else()
		message(STATUS "Not able to check if ${CMAKE_CXX_COMPILER} compiler has complete C++11 support. Project may not work properly.")
	endif()
endif()


# ----------------------------------------------------------------------------
#  Build & install layouts
# ----------------------------------------------------------------------------

# Save libs and executables in the same place
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}" CACHE PATH "Output directory for applications" )

# ----------------------------------------------------------------------------
# Solution folders
# ----------------------------------------------------------------------------
if(ENABLE_SOLUTION_FOLDERS)
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
  set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakeTargets")
endif()

include_directories(".")

# --------------------
# OpenCV 
# --------------------

FIND_PACKAGE(OpenCV REQUIRED core imgcodecs imgproc video)
include_directories( ${OpenCV_INCLUDE_DIRS} )

if(MSVC)
	if("${OpenCV_STATIC}" STREQUAL "ON" OR "${OpenCV_SHARED}" STREQUAL "OFF")
		message(STATUS "Setting static runtime for MSVC")
		foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO) 
			string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
		endforeach(flag_var)
	endif()
endif()

# --------------------
# Project
# --------------------

include_directories("${CMAKE_SOURCE_DIR}")

file(GLOB SRCFILES ${SRCFILES} "*.cpp" "*.c")
file(GLOB HFILES ${HFILES} "*.h" "*.hpp")
	
set (addExe ${SRCFILES} ${HFILES})
add_executable(${ProjectName} ${addExe})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${ProjectName})
target_link_libraries (${ProjectName}  ${OpenCV_LIBS})
include_directories (${CMAKE_CURRENT_BINARY_DIR})