# CMake project for gmsv_rcon2
#
cmake_minimum_required (VERSION 3.8)

project ("gmsv_rcon2" CXX)

#
# Output library path
#
if(NOT "${GMSV_TARGET_PATH}" STREQUAL "")
	message("Using output path: ${GMSV_TARGET_PATH}/")
	set(LIBRARY_OUTPUT_PATH "${GMSV_TARGET_PATH}/")
endif()

file(GLOB src_files
	"src/*.h"
	"src/*.cpp"
)

add_library (gmsv_rcon2 SHARED ${src_files})
include_directories (SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/GarrysMod)

#
# Weird Linux naming for GM
#
set_target_properties(gmsv_rcon2 
	PROPERTIES
	SUFFIX ".dll"
	PREFIX ""
)

#
# SSDK lookup and includes
#
if("${GMSV_SSDK_PATH}" STREQUAL "")
	message(SEND_ERROR "GMSV_SSDK_PATH are not set!")
else()
	message("Using SSKD path: ${GMSV_SSDK_PATH}")
	include_directories (SYSTEM ${GMSV_SSDK_PATH}/mp/src/public/vphysics)
	include_directories (SYSTEM ${GMSV_SSDK_PATH}/mp/src/public/tier0)
	include_directories (SYSTEM ${GMSV_SSDK_PATH}/mp/src/public/tier1)
	include_directories (SYSTEM ${GMSV_SSDK_PATH}/mp/src/public/tier2)
	include_directories (SYSTEM ${GMSV_SSDK_PATH}/mp/src/public/tier3)
	include_directories (SYSTEM ${GMSV_SSDK_PATH}/mp/src/common)
	include_directories (SYSTEM ${GMSV_SSDK_PATH}/mp/src/public)
	
	if(${GMSV_PLATFORM} MATCHES "LINUX")
		target_link_directories(gmsv_rcon2 PRIVATE ${GMSV_SSDK_PATH}/mp/src/lib/public/linux32/)
	elseif(${GMSV_PLATFORM} MATCHES "WINDOWS")
		target_link_directories(gmsv_rcon2 PRIVATE ${GMSV_SSDK_PATH}/mp/src/lib/public/)
	endif()
endif()

#
# Platform auto-detection
#
if("${GMSV_PLATFORM}" STREQUAL "")
	message("Target platform is not set, trying to detect it ...")
	if (UNIX)
		message("Target platform is set to Linux")
		set(GMSV_PLATFORM "LINUX")
	elseif(WIN32 OR MSVC)
		message("Target platform is set to Windows")
		set(GMSV_PLATFORM "WINDOWS")
	else()
		message("Failed to detect platform automatically!")
	endif()
endif()

#
# Platform selection
#
if(${GMSV_PLATFORM} MATCHES "LINUX")
	message("Target platform is Linux")
	add_definitions(-D_PLATFORM_LINUX=1)
	set_target_properties(gmsv_rcon2 
		PROPERTIES
		OUTPUT_NAME "gmsv_rcon2_linux"
	)
	
	target_link_libraries (gmsv_rcon2 ${GMSV_SSDK_PATH}/mp/src/lib/public/linux32/libtier0.so)
	target_link_libraries (gmsv_rcon2 ${GMSV_SSDK_PATH}/mp/src/lib/public/linux32/tier1.a)
	target_link_libraries (gmsv_rcon2 ${GMSV_SSDK_PATH}/mp/src/lib/public/linux32/tier2.a)
	target_link_libraries (gmsv_rcon2 ${GMSV_SSDK_PATH}/mp/src/lib/public/linux32/tier3.a)
	target_link_libraries (gmsv_rcon2 ${GMSV_SSDK_PATH}/mp/src/lib/public/linux32/libvstdlib.so)
elseif(${GMSV_PLATFORM} MATCHES "WINDOWS")
	message("Target platform is Windows")
	add_definitions(-D_PLATFORM_WINDOWS=1)
	set_target_properties(gmsv_rcon2 
		PROPERTIES
		OUTPUT_NAME "gmsv_rcon2_win32"
	)

	target_link_libraries (gmsv_rcon2 ${GMSV_SSDK_PATH}/mp/src/lib/public/tier0.lib)
	target_link_libraries (gmsv_rcon2 ${GMSV_SSDK_PATH}/mp/src/lib/public/tier1.lib)
	target_link_libraries (gmsv_rcon2 ${GMSV_SSDK_PATH}/mp/src/lib/public/tier2.lib)
	target_link_libraries (gmsv_rcon2 ${GMSV_SSDK_PATH}/mp/src/lib/public/tier3.lib)
	target_link_libraries (gmsv_rcon2 ${GMSV_SSDK_PATH}/mp/src/lib/public/vstdlib.lib)
	target_link_libraries (gmsv_rcon2 ws2_32.lib)

	set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
	if(MSVC)
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:\"LIBCMT\" /NODEFAULTLIB:\"LIBCPMT\"")
	endif()
else()
	message(SEND_ERROR "Target platform is Unknown")
	add_definitions(-D_PLATFORM_UKNOWN=1)
endif()

# Gmod Module definition
add_definitions(-DGMMODULE=1)

# Common stuff
set(X86 TRUE)
set(CMAKE_CXX_STANDARD 11)
if(NOT MSVC)
	set(CMAKE_CXX_FLAGS "-m32")
endif()

#
# Different target configurations
#
if("${GMSV_TARGET}" STREQUAL "")
	set(GMSV_TARGET "Release")
endif()
message("Target configurations type - ${GMSV_TARGET}")
if("${GMSV_TARGET}" STREQUAL "Release")
	if(MSVC)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -s -DNDEBUG")
	endif()
elseif("${GMSV_TARGET}" STREQUAL "Debug")
	if(MSVC)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DEBUG /Od")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
	endif()
endif()
