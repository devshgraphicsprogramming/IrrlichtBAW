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
ADD_SUBDIRECTORY(02.GPU_Mesh)
ADD_SUBDIRECTORY(03.BuiltInNodes)
ADD_SUBDIRECTORY(04.SpherePoints)
ADD_SUBDIRECTORY(05.MeshLoaders)
ADD_SUBDIRECTORY(06.HardwareSkinning)
ADD_SUBDIRECTORY(07.HardwareInstancing)
ADD_SUBDIRECTORY(08.PerfTest)
ADD_SUBDIRECTORY(09.AllocPerfTest)
ADD_SUBDIRECTORY(10.RayCastCollision)

message(STATUS " ")

