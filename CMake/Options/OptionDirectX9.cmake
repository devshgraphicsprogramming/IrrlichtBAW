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
	SET (IRRLICHT_ENABLE_DIRECTX9 ON CACHE BOOL "Enabled DirectX9 support.")
else()
	SET (IRRLICHT_ENABLE_DIRECTX9 OFF CACHE BOOL "Enabled DirectX9 support.")
endif()

if (IRRLICHT_ENABLE_DIRECTX9)
	message(STATUS "Enable DirectX support...")
	ADD_DEFINITIONS(
		-DIRR_COMPILE_WITH_DX9_DEV_PACK
	)
					
else ()
	message(STATUS "Disable DirectX support...")
	ADD_DEFINITIONS(
		-DNO_IRR_COMPILE_WITH_DIRECTINPUT_JOYSTICK_
		-DNO_IRR_COMPILE_WITH_DIRECT3D_9_
	)
		
endif ()
