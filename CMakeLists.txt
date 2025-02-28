#
# D++ (DPP), The Lightweight C++ Discord Library
#
# Copyright 2021 Craig Edwards <support@brainbox.cc>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

cmake_minimum_required (VERSION 3.12)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(DPP_BUILD_TEST "Build the test program" ON)

add_compile_definitions(DPP_BUILD)

#if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
#	set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
#endif()

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/include/dpp/version.h" version_h)

if(NOT version_h MATCHES "DPP_VERSION_SHORT ([0-9][0-9])([0-9][0-9])([0-9][0-9])")
	message(FATAL_ERROR "Cannot get DPP_VERSION_SHORT from version.h")
endif()

math(EXPR DPP_VERSION_MAJOR ${CMAKE_MATCH_1})
math(EXPR DPP_VERSION_MINOR ${CMAKE_MATCH_2})
math(EXPR DPP_VERSION_PATCH ${CMAKE_MATCH_3})

string(CONCAT DPP_VERSION ${DPP_VERSION_MAJOR}.${DPP_VERSION_MINOR}.${DPP_VERSION_PATCH})

project(libdpp VERSION ${DPP_VERSION} LANGUAGES CXX)

if(WIN32 AND NOT MINGW)
	if (NOT WINDOWS_32_BIT)
		message("-- Building for windows with precompiled packaged dependencies")
		#set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
		set(ZLIB_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/win32/lib")
		set(ZLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/win32/include")
		set(OPENSSL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/win32/include")
		set(OPENSSL_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/win32/lib")
		ADD_DEFINITIONS(/bigobj)

		link_libraries("${CMAKE_CURRENT_SOURCE_DIR}/win32/lib/libssl.lib")
		link_libraries("${CMAKE_CURRENT_SOURCE_DIR}/win32/lib/libcrypto.lib")
		link_libraries("${CMAKE_CURRENT_SOURCE_DIR}/win32/lib/zlib.lib")
		link_libraries("${CMAKE_CURRENT_SOURCE_DIR}/win32/lib/libsodium.lib")
		link_libraries("${CMAKE_CURRENT_SOURCE_DIR}/win32/lib/opus.lib")

		set(OPUS_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/win32/include")
		set(OPUS_LIBRARIES "${CMAKE_CURRENT_SOURCE_DIR}/win32/lib/opus.lib")
		set(sodium_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/win32/include")
		set(sodium_LIBRARY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/win32/lib/libsodium.lib")
		set(sodium_LIBRARY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/win32/lib/libsodium.lib")
		set(HAVE_OPUS_OPUS_H "${CMAKE_CURRENT_SOURCE_DIR}/win32/include/opus/opus.h")
		set(OPUS_FOUND 1)
		SET(sodium_VERSION_STRING "win32 bundled")

		include_directories("${CMAKE_CURRENT_SOURCE_DIR}/win32/include")

		add_compile_definitions(OPENSSL_SYS_WIN32)
		add_compile_definitions(_WINSOCK_DEPRECATED_NO_WARNINGS)
		add_compile_definitions(WIN32_LEAN_AND_MEAN)
		add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
		add_compile_definitions(_CRT_NONSTDC_NO_DEPRECATE)

	endif()
endif()

if(NOT BUILD_SHARED_LIBS)
	if(UNIX)
		message("-- Building static library.")
		
		if(UNIX AND NOT APPLE)
			set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
		endif()
		
		set(OPENSSL_USE_STATIC_LIBS ON)
		set(sodium_USE_STATIC_LIBS ON)
		set(OPUS_USE_STATIC_LIBS TRUE)
	else()
		message(WARNING "-- Building of static library not supported on non UNIX systems.")
	endif()
endif()

include("cmake/colour.cmake")
if (MINGW OR NOT WIN32)
	include("cmake/FindSodium.cmake")
	include("cmake/FindOpus.cmake")
endif()

if(DEFINED OPUS_FOUND)
	if(DEFINED sodium_VERSION_STRING)
		add_compile_definitions(HAVE_VOICE)
		
		message("-- Sodium ${Green}${sodium_VERSION_STRING}${ColourReset}")
		
		set(HAVE_VOICE 1)
	endif()
endif()

if(HAVE_VOICE)
	message("-- Detected ${Green}libsodium${ColourReset} and ${Green}libopus${ColourReset}. VOICE support will be ${Green}enabled${ColourReset}")
else(HAVE_VOICE)
	message("-- Could not detect ${Green}libsodium${ColourReset} and/or ${Green}libopus${ColourReset}. VOICE support will be ${Red}disabled${ColourReset}")
endif(HAVE_VOICE)

aux_source_directory("src" coresrc)

string(ASCII 27 Esc)

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)
if(MINGW OR NOT WIN32)
	find_package(ZLIB REQUIRED)
endif(MINGW OR NOT WIN32)

if(APPLE)
	set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl")

	find_package(OpenSSL REQUIRED)
else()
	if(MINGW OR NOT WIN32)
		find_package(OpenSSL REQUIRED)
	endif()
endif()

include_directories(${OPENSSL_INCLUDE_DIR} ${ZLIB_INCLUDE_DIRS})

include_directories("include")

find_package(Git QUIET)

if(NOT GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
	message(FATAL_ERROR "You are using a git version of D++ but do not have git installed. Install git (not 'gh') and try again.")
endif()

if(EXISTS "${PROJECT_SOURCE_DIR}/.git")
	message("-- Building ${Green}git${ColourReset} version. ${Green}Be aware git versions may be unstable!${ColourReset}")
else()
	message("-- Building ${Green}stable${ColourReset} version ${Green}${DPP_VERSION}${ColourReset}")
endif()

if(UNIX OR MSYS)
	find_program(LDCONFIG_EXECUTABLE "ldconfig")
endif()

if(MSVC)
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17 /Od /DEBUG /Zi /sdl /MP /DFD_SETSIZE=1024")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17 /O2 /Oi /Oy /GL /Gy /sdl /MP /DFD_SETSIZE=1024")
	endif()
	string(REGEX REPLACE "/W[1|2|3|4]" "/W3" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
else()
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wempty-body -Wignored-qualifiers -Wimplicit-fallthrough -Wmissing-field-initializers -Wsign-compare -Wtype-limits -Wuninitialized -Wshift-negative-value -pthread -g -Og -fPIC")
	else()
		if (BUILD_O3)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wempty-body -Wignored-qualifiers -Wimplicit-fallthrough -Wmissing-field-initializers -Wsign-compare -Wtype-limits -Wuninitialized -Wshift-negative-value -pthread -O3 -fPIC")
		else()
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wempty-body -Wignored-qualifiers -Wimplicit-fallthrough -Wmissing-field-initializers -Wsign-compare -Wtype-limits -Wuninitialized -Wshift-negative-value -pthread -Os -fPIC")
		endif()
	endif()

	if (NOT MINGW)
		add_link_options("-rdynamic")
	endif ()
endif()

set(modules_dir "src")

file(GLOB subdirlist ${modules_dir}/dpp)

foreach (fullmodname ${subdirlist})
    get_filename_component(modname ${fullmodname} NAME)
    set (modsrc "")
	
	file(GLOB modsrc "${modules_dir}/dpp/*.cpp" "${modules_dir}/dpp/events/*.cpp" "${modules_dir}/dpp/cluster/*.cpp")
	
	if(BUILD_SHARED_LIBS)
        add_library(${modname} SHARED ${modsrc})
	else()
		add_library(${modname} STATIC ${modsrc})
	endif()

	if (WIN32 AND NOT MINGW)
		if (NOT WINDOWS_32_BIT)
			target_link_libraries(${modname} PUBLIC "${PROJECT_SOURCE_DIR}/win32/lib/libssl.lib"
				"${PROJECT_SOURCE_DIR}/win32/lib/libcrypto.lib"
				"${PROJECT_SOURCE_DIR}/win32/lib/libsodium.lib"
				"${PROJECT_SOURCE_DIR}/win32/lib/opus.lib"
				"${PROJECT_SOURCE_DIR}/win32/lib/zlib.lib")
		else()
			target_link_libraries(${modname} PUBLIC "${PROJECT_SOURCE_DIR}/win32/32/lib/libssl.lib"
				"${PROJECT_SOURCE_DIR}/win32/32/lib/libcrypto.lib"
				"${PROJECT_SOURCE_DIR}/win32/32/lib/libsodium.lib"
				"${PROJECT_SOURCE_DIR}/win32/32/lib/opus.lib"
				"${PROJECT_SOURCE_DIR}/win32/32/lib/zlib.lib")
		endif()

	else()
		target_link_libraries(${modname} PUBLIC ${OPENSSL_CRYPTO_LIBRARY} ${OPENSSL_SSL_LIBRARY} ${ZLIB_LIBRARIES})
		if (MINGW)
			target_link_libraries(${modname} PUBLIC wsock32 ws2_32 crypt32)
		endif ()
	endif()

	if (HAVE_VOICE)
		target_link_libraries(${modname} PUBLIC ${sodium_LIBRARY_RELEASE} ${OPUS_LIBRARIES})
		
		include_directories(${OPUS_INCLUDE_DIRS} ${sodium_INCLUDE_DIR})
	endif()
endforeach()

target_compile_features(dpp PRIVATE cxx_std_17)

if (DPP_BUILD_TEST)
	add_executable(test ${coresrc})
	target_compile_features(test PRIVATE cxx_std_17)
	target_link_libraries(test PUBLIC ${modname})
endif()

if(WIN32 AND NOT MINGW)
	if (NOT WINDOWS_32_BIT)
		configure_file("${PROJECT_SOURCE_DIR}/win32/bin/zlib1.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" COPYONLY)
		configure_file("${PROJECT_SOURCE_DIR}/win32/bin/libcrypto-1_1-x64.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" COPYONLY)
		configure_file("${PROJECT_SOURCE_DIR}/win32/bin/libssl-1_1-x64.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" COPYONLY)
		configure_file("${PROJECT_SOURCE_DIR}/win32/bin/libsodium.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" COPYONLY)
		configure_file("${PROJECT_SOURCE_DIR}/win32/bin/opus.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" COPYONLY)
	else()
		configure_file("${PROJECT_SOURCE_DIR}/win32/32/bin/zlib1.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" COPYONLY)
		configure_file("${PROJECT_SOURCE_DIR}/win32/32/bin/libcrypto-1_1.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" COPYONLY)
		configure_file("${PROJECT_SOURCE_DIR}/win32/32/bin/libssl-1_1.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" COPYONLY)
		configure_file("${PROJECT_SOURCE_DIR}/win32/32/bin/libsodium.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" COPYONLY)
		configure_file("${PROJECT_SOURCE_DIR}/win32/32/bin/opus.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" COPYONLY)
	endif()
endif()

if(NOT WIN32)
	# Installation

	include(GNUInstallDirs)
	install(TARGETS dpp LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
	install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
	install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -DLDCONFIG_EXECUTABLE=${LDCONFIG_EXECUTABLE} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/PostInstall.cmake)")

	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/dpp.pc.in" "${CMAKE_BINARY_DIR}/dpp.pc" @ONLY)
	install(FILES "${CMAKE_BINARY_DIR}/dpp.pc" DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
elseif(MINGW)
	install(TARGETS dpp LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
	install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
endif()

include("cmake/CPackSetup.cmake")						# Setup information for packaging and distribution

# CPack initialization for distribution
include(CPack)
