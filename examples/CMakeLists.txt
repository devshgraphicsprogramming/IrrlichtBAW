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

cmake_minimum_required(VERSION 2.6)

message(STATUS " -> Configure all examples:")

SET(IRRLICHT_MEDIA_DIR   "${CMAKE_SOURCE_DIR}/../media" CACHE PATH "The directory where the media files are stored in.")

ADD_SUBDIRECTORY(01.HelloWorld)
ADD_SUBDIRECTORY(02.Quake3Map)
ADD_SUBDIRECTORY(03.CustomSceneNode)
ADD_SUBDIRECTORY(04.Movement)
ADD_SUBDIRECTORY(05.UserInterface)
ADD_SUBDIRECTORY(06.2DGraphics)
ADD_SUBDIRECTORY(07.Collision)
ADD_SUBDIRECTORY(08.SpecialFX)
ADD_SUBDIRECTORY(09.Meshviewer)
ADD_SUBDIRECTORY(10.Shaders)
ADD_SUBDIRECTORY(11.PerPixelLighting)
ADD_SUBDIRECTORY(12.TerrainRendering)
ADD_SUBDIRECTORY(13.RenderToTexture)
ADD_SUBDIRECTORY(14.Win32Window)
ADD_SUBDIRECTORY(15.LoadIrrFile)
ADD_SUBDIRECTORY(16.Quake3MapShader)
ADD_SUBDIRECTORY(17.HelloWorld_Mobile)
ADD_SUBDIRECTORY(18.SplitScreen)
ADD_SUBDIRECTORY(19.MouseAndJoystick)
ADD_SUBDIRECTORY(20.ManagedLights)
ADD_SUBDIRECTORY(21.Quake3Explorer)
ADD_SUBDIRECTORY(22.MaterialViewer)
ADD_SUBDIRECTORY(23.SMeshHandling)
ADD_SUBDIRECTORY(24.CursorControl)
ADD_SUBDIRECTORY(25.XmlHandling)
ADD_SUBDIRECTORY(26.OcclusionQuery)
ADD_SUBDIRECTORY(Demo)

message(STATUS " ")

