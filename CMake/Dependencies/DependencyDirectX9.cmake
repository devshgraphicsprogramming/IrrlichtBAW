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

if(IRRLICHT_ENABLE_DIRECTX9)
	if(WIN32)
		message(STATUS "Looking for DirectX9 header and library...")
		
		find_package(DirectX)
		
		if (DirectX9_FOUND)
			MESSAGE(STATUS "DirectX Correctly Found: ")
			MESSAGE(STATUS " * Include-Dir: ${DirectX9_INCLUDE_DIR}")
			MESSAGE(STATUS " * Libraries: ${DirectX9_LIBRARIES}")
			INCLUDE_DIRECTORIES("${DirectX9_INCLUDE_DIR}")
			SET(IRRLICHT_DEPENDENCY_LIBRARIES
				${IRRLICHT_DEPENDENCY_LIBRARIES}
				${DirectX9_LIBRARIES}
			)			
			
		else ()
			MESSAGE(SEND_ERROR "DirectX environment missing")
			
		endif ()
	
	else ()
		message(SEND_ERROR "Operating system does not support DirectX.")
		
	endif ()
endif ()

