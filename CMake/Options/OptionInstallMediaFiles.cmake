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

SET (IRRLICHT_INSTALL_MEDIA_FILES ON CACHE BOOL "Installs the media files.")
SET(IRRLICHT_MEDIA_DIR "${CMAKE_SOURCE_DIR}/media" CACHE PATH "The directory where the media files are stored in.")

if (IRRLICHT_INSTALL_MEDIA_FILES)
	message(STATUS "Install media files...")
else ()
	message(STATUS "Do not install media files...")
endif ()
