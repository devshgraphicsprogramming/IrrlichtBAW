#
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

SET(MSVC_LIKE_COMPILER OFF)

if ("x${CMAKE_CXX_COMPILER_ID}" STREQUAL "xMSVC")
	message(STATUS "Detect MSVC compiler...")
	SET(MSVC_LIKE_COMPILER ON)
elseif ("x${CMAKE_CXX_COMPILER_ID}" STREQUAL "xIntel")
	if(WIN32)
		message(STATUS "Detect Intel compiler and handle it like MSVC...")
		SET(MSVC_LIKE_COMPILER ON)
	endif ()
endif ()

if (MSVC_LIKE_COMPILER)
	SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wall /EHs-c- /GR-")
	SET (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O3")
	SET (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O3")
endif ()
