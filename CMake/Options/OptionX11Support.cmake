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

if(WIN32)
	SET (IRRLICHT_ENABLE_X11_SUPPORT OFF CACHE BOOL "Enabled X11 support.")
else()
	SET (IRRLICHT_ENABLE_X11_SUPPORT ON CACHE BOOL "Enabled X11 support.")
endif()

if (IRRLICHT_ENABLE_X11_SUPPORT)
	if (WIN32)
		message(FATAL SEND_ERROR "X11 support is not possible on Windows!")
	else ()
		message(STATUS "Enable X11 support...")
	endif ()
else ()
	if (UNIX)
		message(FATAL SEND_ERROR "X11 support is mandatory on Unix like operating systems!")
	endif ()
endif ()
