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

message(STATUS "Looking for OpenGL header and library...")

find_package(OpenGL)

if (OPENGL_FOUND)
    MESSAGE(STATUS "OpenGL Correctly Found:")
	MESSAGE(STATUS " * Include-Dir: ${OPENGL_INCLUDE_DIR}")
	MESSAGE(STATUS " * Libraries: ${OPENGL_LIBRARIES}")	
    INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR})
	SET(IRRLICHT_DEPENDENCY_LIBRARIES
	${IRRLICHT_DEPENDENCY_LIBRARIES}
	${OPENGL_LIBRARIES}
	)
	
else ()
    MESSAGE(STATUS "OpenGL environment missing")
	
endif ()