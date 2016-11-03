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

file(WRITE "${CMAKE_BINARY_DIR}/CMakeSettings.txt" "set(IRRLICHT_ENABLE_PROFILING ${IRRLICHT_ENABLE_PROFILING} CACHE BOOL  '')\n")
file(APPEND "${CMAKE_BINARY_DIR}/CMakeSettings.txt" "set(IRRLICHT_ENABLE_DIRECTX9 ${IRRLICHT_ENABLE_DIRECTX9} CACHE BOOL  '')\n")
file(APPEND "${CMAKE_BINARY_DIR}/CMakeSettings.txt" "set(IRRLICHT_ENABLE_X11_SUPPORT ${IRRLICHT_ENABLE_X11_SUPPORT} CACHE BOOL  '')\n")
file(APPEND "${CMAKE_BINARY_DIR}/CMakeSettings.txt" "set(IRRLICHT_STATIC_LIBRARY ${IRRLICHT_STATIC_LIBRARY} CACHE BOOL  '')\n")
file(APPEND "${CMAKE_BINARY_DIR}/CMakeSettings.txt" "set(IRRLICHT_INSTALL_MEDIA_FILES ${IRRLICHT_INSTALL_MEDIA_FILES} CACHE BOOL  '')\n")
file(APPEND "${CMAKE_BINARY_DIR}/CMakeSettings.txt" "set(IRRLICHT_BUILD_EXAMPLES ${IRRLICHT_BUILD_EXAMPLES} CACHE BOOL  '')\n")

file(APPEND "${CMAKE_BINARY_DIR}/CMakeSettings.txt" "set(CMAKE_CXX_FLAGS '${CMAKE_CXX_FLAGS}' CACHE STRING  '')\n")
file(APPEND "${CMAKE_BINARY_DIR}/CMakeSettings.txt" "set(CMAKE_CXX_FLAGS_RELEASE '${CMAKE_CXX_FLAGS_RELEASE}' CACHE STRING  '')\n")
file(APPEND "${CMAKE_BINARY_DIR}/CMakeSettings.txt" "set(CMAKE_CXX_FLAGS_DEBUG '${CMAKE_CXX_FLAGS_DEBUG}' CACHE STRING  '')\n")
file(APPEND "${CMAKE_BINARY_DIR}/CMakeSettings.txt" "set(CMAKE_C_FLAGS '${CMAKE_C_FLAGS}' CACHE STRING  '')\n")
file(APPEND "${CMAKE_BINARY_DIR}/CMakeSettings.txt" "set(CMAKE_C_FLAGS_RELEASE '${CMAKE_C_FLAGS_RELEASE}' CACHE STRING  '')\n")
file(APPEND "${CMAKE_BINARY_DIR}/CMakeSettings.txt" "set(CMAKE_C_FLAGS_DEBUG '${CMAKE_C_FLAGS_DEBUG}' CACHE STRING  '')\n")
file(APPEND "${CMAKE_BINARY_DIR}/CMakeSettings.txt" "set(CMAKE_EXE_LINKER_FLAGS '${CMAKE_EXE_LINKER_FLAGS}' CACHE STRING  '')\n")
file(APPEND "${CMAKE_BINARY_DIR}/CMakeSettings.txt" "set(CMAKE_MODULE_LINKER_FLAGS '${CMAKE_MODULE_LINKER_FLAGS}' CACHE STRING  '')\n")
file(APPEND "${CMAKE_BINARY_DIR}/CMakeSettings.txt" "set(CMAKE_STATIC_LINKER_FLAGS '${CMAKE_STATIC_LINKER_FLAGS}' CACHE STRING  '')\n")
file(APPEND "${CMAKE_BINARY_DIR}/CMakeSettings.txt" "set(CMAKE_SHARED_LINKER_FLAGS '${CMAKE_SHARED_LINKER_FLAGS}' CACHE STRING  '')\n")

file(APPEND "${CMAKE_BINARY_DIR}/CMakeSettings.txt" "set(CMAKE_INSTALL_PREFIX '${CMAKE_INSTALL_PREFIX}' CACHE PATH  '')\n")




