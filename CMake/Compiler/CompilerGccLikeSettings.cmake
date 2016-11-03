# The ZLIB license
#
# Copyright (c) 2015 André Netzeband
#
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgement in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.
#

SET(GCC_LIKE_COMPILER OFF)

if ("x${CMAKE_CXX_COMPILER_ID}" STREQUAL "xClang")
	message(STATUS "Detect CLANG compiler and handle it like GCC...")
	SET(GCC_LIKE_COMPILER ON)
elseif ("x${CMAKE_CXX_COMPILER_ID}" STREQUAL "xGNU")
	message(STATUS "Detect GCC compiler...")
	SET(GCC_LIKE_COMPILER ON)
elseif ("x${CMAKE_CXX_COMPILER_ID}" STREQUAL "xIntel")
	if(UNIX)
		message(STATUS "Detect Intel compiler and handle it like GCC...")
		SET(GCC_LIKE_COMPILER ON)
	endif ()
endif ()

if (GCC_LIKE_COMPILER)
	SET (CMAKE_CXX_FLAGS
		"${CMAKE_CXX_FLAGS} -pipe -fno-exceptions -fstrict-aliasing -std=gnu++98 -g -fstack-protector-all -fno-omit-frame-pointer -fno-exceptions -fuse-ld=gold -msse3 -ggdb3 -D_IRR_STATIC_LIB_ -D_DEBUG -w"
#		"${CMAKE_CXX_FLAGS} -pipe -fno-exceptions -fno-rtti -fstrict-aliasing -std=gnu++98 -g -fstack-protector-all -fno-omit-frame-pointer -fno-exceptions -fuse-ld=gold -msse3 -ggdb3 -D_IRR_STATIC_LIB_ -D_DEBUG -w"
#		"${CMAKE_CXX_FLAGS} -Wall -pipe -fno-exceptions -fno-rtti -fstrict-aliasing -std=gnu++98 -Wextra -Wall -g -W -fstack-protector-all -Wno-unused-parameter -fno-omit-frame-pointer -fno-exceptions -fuse-ld=gold -msse3 -ggdb3 -D_IRR_STATIC_LIB_ -D_DEBUG"

	)
	SET (CMAKE_CXX_FLAGS_RELEASE
		"${CMAKE_CXX_FLAGS_RELEASE} -fexpensive-optimizations -O3"
	)
	SET (CMAKE_C_FLAGS_RELEASE
		"${CMAKE_C_FLAGS_RELEASE} -O3 -fexpensive-optimizations"
	)
	
	INCLUDE(CheckCSourceCompiles)
	CHECK_C_SOURCE_COMPILES("
		#ifndef __x86_64__
		#error
		#endif
		
		int main(void) { return 0; }
		" 
		WITH_64BIT
		)

	if(WITH_64BIT)
		message(STATUS "Detect 64bit GCC like compiler...")
		SET(CMAKE_CL_64 TRUE CACHE BOOL "" FORCE)
		mark_as_advanced(CMAKE_CL_64)
	else()
		message(STATUS "Detect 32bit GCC like compiler...")
		SET(CMAKE_CL_64 FALSE CACHE BOOL "" FORCE)
		mark_as_advanced(CMAKE_CL_64)
	endif()
endif ()

