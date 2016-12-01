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

message(STATUS " ")
message(STATUS "The following settings have been applied:")
message(STATUS " -> Dependencies found:")
message(STATUS "    * OpenGL:     ${OPENGL_FOUND}")

if ((UNIX) OR ((APPLE) AND (IRRLICHT_ENABLE_X11_SUPPORT)))
	message(STATUS "    * X11:        ${X11_FOUND}")
	
	if (IRRLICHT_BUILD_EXAMPLES)
		message(STATUS "    * X11xf86vm:  ${X11_xf86vmode_FOUND}")	
	endif ()
	
endif ()

message(STATUS " ")
message(STATUS " -> Options set:")
message(STATUS "    * Profiling Support:   ${IRRLICHT_ENABLE_PROFILING}")
message(STATUS "    * DirectX9 Support:    ${IRRLICHT_ENABLE_DIRECTX9}")
message(STATUS "    * X11 Support:         ${IRRLICHT_ENABLE_X11_SUPPORT}")
message(STATUS "    * Build static lib:    ${IRRLICHT_STATIC_LIBRARY}")
message(STATUS "    * Install media files: ${IRRLICHT_INSTALL_MEDIA_FILES}")
message(STATUS "    * Build examples:      ${IRRLICHT_BUILD_EXAMPLES}")
message(STATUS " ")
message(STATUS " -> Compiler settings:")
message(STATUS "    * GCC like compiler:            ${GCC_LIKE_COMPILER}")
message(STATUS "    * MSVC like compiler:           ${MSVC_LIKE_COMPILER}")
message(STATUS "    * C++ Compiler flags:           ${CMAKE_CXX_FLAGS}")
message(STATUS "    * C++ Compiler flags (release): ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "    * C++ Compiler flags (debug):   ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "    * C Compiler flags:             ${CMAKE_C_FLAGS}")
message(STATUS "    * C Compiler flags (release):   ${CMAKE_C_FLAGS_RELEASE}")
message(STATUS "    * C Compiler flags (debug):     ${CMAKE_C_FLAGS_DEBUG}")
message(STATUS "    * Linker flags (executable):    ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "    * Linker flags (module):        ${CMAKE_MODULE_LINKER_FLAGS}")
message(STATUS "    * Linker flags (static lib):    ${CMAKE_STATIC_LINKER_FLAGS}")
message(STATUS "    * Linker flags (shared lib):    ${CMAKE_SHARED_LINKER_FLAGS}")

GET_DIRECTORY_PROPERTY( DirDefines DIRECTORY ${CMAKE_SOURCE_DIR} COMPILE_DEFINITIONS )
foreach( SingleDefine ${DirDefines} )
    message(STATUS "    * Use define: " ${SingleDefine} )
endforeach()

message(STATUS " ")
message(STATUS " -> Install Path: ${CMAKE_INSTALL_PREFIX}")
message(STATUS " ")
