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

SET (IRRLICHT_ENABLE_PROFILING OFF CACHE BOOL "Enabled profiling information for Irrlicht library (GCC like compilers only at the moment).")
if(IRRLICHT_ENABLE_PROFILING)

	if (MSVC_LIKE_COMPILER)
		SET (CMAKE_STATIC_LINKER_FLAGS
			"${CMAKE_STATIC_LINKER_FLAGS} /PROFILE"
		)
		SET (CMAKE_SHARED_LINKER_FLAGS
			"${CMAKE_SHARED_LINKER_FLAGS} /PROFILE"
		)
		SET (CMAKE_MODULE_LINKER_FLAGS
			"${CMAKE_MODULE_LINKER_FLAGS} /PROFILE"
		)
		SET (CMAKE_EXE_LINKER_FLAGS
			"${CMAKE_EXE_LINKER_FLAGS} /PROFILE"
		)
		message(STATUS "Enable profiling for MSVC like compilers...")
		
	elseif (GCC_LIKE_COMPILER)
		SET (CMAKE_CXX_FLAGS
			"${CMAKE_CXX_FLAGS} -pg"
		)
		SET (CMAKE_C_FLAGS
			"${CMAKE_C_FLAGS} -pg"
		)
		message(STATUS "Enable profiling for GCC like compilers...")
		
	endif ()
		
endif()
