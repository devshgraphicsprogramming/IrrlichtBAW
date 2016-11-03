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

# this is only for UNIX or apple a dependency
if ((UNIX) OR ((APPLE) AND (IRRLICHT_ENABLE_X11_SUPPORT)))

	message(STATUS "Looking for X11 library...")

	find_package(X11)
        find_package(XRandR)

	if (X11_FOUND)
	  MESSAGE(STATUS "XRandR library correctly Found:")
	  MESSAGE(STATUS " * Include-Dir: ${X11_INCLUDE_DIR}")
	  MESSAGE(STATUS " * Libraries: ${X11_LIBRARIES}")	

	  INCLUDE_DIRECTORIES(
	    ${XRANDR_INCLUDE_DIR}
	    )
	  
	  SET(IRRLICHT_DEPENDENCY_LIBRARIES
	    ${IRRLICHT_DEPENDENCY_LIBRARIES}
	    ${XRANDR_LIBRARIES}
	    )
        else()
	  MESSAGE(ERROR "XRandR library is missing. Please install it before start compiling.")
        endif()

	if (X11_FOUND)
		MESSAGE(STATUS "X11 library correctly Found:")
		MESSAGE(STATUS " * Include-Dir: ${X11_INCLUDE_DIR}")
		MESSAGE(STATUS " * Libraries: ${X11_LIBRARIES}")	
		
		INCLUDE_DIRECTORIES(
			${X11_INCLUDE_DIR}
		)
		
		SET(IRRLICHT_DEPENDENCY_LIBRARIES
			${IRRLICHT_DEPENDENCY_LIBRARIES}
			${X11_LIBRARIES}
		)
			
	else ()
		MESSAGE(ERROR "X11 library is missing. Please install it before start compiling.")
	
	endif ()

	# only examples need this library
	if (X11_xf86vmode_FOUND AND (IRRLICHT_BUILD_EXAMPLES))
		MESSAGE(STATUS "X11 xf86vm library correctly Found:")
		MESSAGE(STATUS " * Include-Dir: ${X11_xf86vmode_INCLUDE_PATH}")
		MESSAGE(STATUS " * Libraries: ${X11_Xxf86vm_LIB}")	
		
		INCLUDE_DIRECTORIES(
			${X11_xf86vmode_INCLUDE_PATH}
		)
		
		SET(IRRLICHT_DEPENDENCY_LIBRARIES
			${IRRLICHT_DEPENDENCY_LIBRARIES}
			${X11_Xxf86vm_LIB}
		)
	else ()
		MESSAGE(WARNING "X11 xf86vm library is missing. Not all examples may compile correctly...")
	endif ()
	
endif ()
