cmake_minimum_required (VERSION 3.15)
if (${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    set (CMAKE_SYSTEM_VERSION 8.1 CACHE TYPE INTERNAL FORCE) #Force 8.1 SDK, to keep it compatible with win7
endif()
cmake_policy(SET CMP0091 NEW)
project (DAA C CXX)
find_package (Threads)


if(MSVC)
	set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /std:c++latest" )
elseif(MSVC)
	message(FATAL_ERROR "ERROR: You need a C++20 compatible compiler")
endif()

message("GENERATOR USED: '${CMAKE_GENERATOR}'")
message("COMPILER USED: '${CMAKE_CXX_COMPILER_ID}'")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/release/@daa_mod")

set(CMAKE_CXX_STANDARD 23) # we want c++latest, because MSVC only includes some c++20 features if you set latest
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

SET(CMAKE_INCLUDE_CURRENT_DIR ON) 
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#set(CMAKE_SUPPRESS_REGENERATION true)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_subdirectory(src)