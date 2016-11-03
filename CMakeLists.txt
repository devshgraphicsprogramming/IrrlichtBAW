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
cmake_policy(VERSION 2.6)

PROJECT(Irrlicht)

SET(CMAKE_MODULE_PATH 
	${CMAKE_MODULE_PATH} 
	${CMAKE_SOURCE_DIR}/CMake
	${CMAKE_SOURCE_DIR}/CMake/Dependencies
	${CMAKE_SOURCE_DIR}/CMake/Compiler
	${CMAKE_SOURCE_DIR}/CMake/OperatingSystems
	${CMAKE_SOURCE_DIR}/CMake/Options
	${CMAKE_SOURCE_DIR}/CMake/Packages
)

# Contains the version information...
INCLUDE(IrrlichtVersion)

# Contains a list of source files...
INCLUDE(IrrlichtSource)

# Load compiler dependent settings...
INCLUDE(CompilerGccLikeSettings)
INCLUDE(CompilerMSVCLikeSettings)

# Load os dependent settings...
INCLUDE(UnixLikeSettings)
INCLUDE(MacOSXSettings)
INCLUDE(WindowsSettings)

# Load option depending settings...
INCLUDE(OptionProfiling)
INCLUDE(OptionDirectX9)
INCLUDE(OptionX11Support)
INCLUDE(OptionStaticLib)
INCLUDE(OptionBuildExamples)
INCLUDE(OptionInstallMediaFiles)

# Generic definitions for all compilers and operating systems...
ADD_DEFINITIONS(	
	-DPNG_THREAD_UNSAFE_OK
	-DPNG_NO_MMX_CODE
	-DPNG_NO_MNG_FEATURES
)

# Add dependency definitions
INCLUDE(DependencyOpenGL)
INCLUDE(DependencyDirectX9)
INCLUDE(DependencyX11)

# Print all settings
INCLUDE(PrintSettings)
INCLUDE(WriteSettings)

# Target settings
SET_SOURCE_FILES_PROPERTIES(ALL_HEADER_FILES
                            PROPERTIES HEADER_FILE_ONLY TRUE)
							
LIST(APPEND ALL_SOURCE_FILES ${ALL_HEADER_FILES})

# Cache back the flags (seems to be an error of cmake for msys makefile generator)
if (NOT CFLAG_WRITE_BACK)
	SET(CFLAG_WRITE_BACK TRUE CACHE BOOL "(do not change!)" FORCE)
	mark_as_advanced(FORCE CFLAG_WRITE_BACK)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE STRING "compile flags" FORCE)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "compile flags" FORCE)
endif ()

if (IRRLICHT_STATIC_LIBRARY)
	ADD_LIBRARY (Irrlicht ${ALL_SOURCE_FILES} )
else ()
	ADD_LIBRARY (Irrlicht SHARED ${ALL_SOURCE_FILES} )
endif ()
SET_TARGET_PROPERTIES(Irrlicht PROPERTIES VERSION ${VERSION_STRING} SOVERSION ${SOVERSION_STRING})

SET_TARGET_PROPERTIES(Irrlicht PROPERTIES COMPILE_DEFINITIONS "IRRLICHT_EXPORTS=1" )
TARGET_LINK_LIBRARIES(Irrlicht ${IRRLICHT_DEPENDENCY_LIBRARIES} ${OS_DEPENDENT_LIBRARIES})

INSTALL(TARGETS Irrlicht
	ARCHIVE DESTINATION  "lib" CONFIGURATIONS Debug Release MinSizeRel RelWithDebInfo
	LIBRARY DESTINATION  "lib" CONFIGURATIONS Debug Release MinSizeRel RelWithDebInfo
	RUNTIME DESTINATION  "bin" CONFIGURATIONS Debug Release MinSizeRel RelWithDebInfo
)

INSTALL(FILES ${IRRLICHT_PUBLIC_HEADER_FILES} DESTINATION "includes/Irrlicht")

if (IRRLICHT_INSTALL_MEDIA_FILES)
	INSTALL(DIRECTORY ${IRRLICHT_MEDIA_DIR} DESTINATION  ".")
endif ()

if (IRRLICHT_BUILD_EXAMPLES)
	SET(IRRLICHT_LIB_FILE $<TARGET_LINKER_FILE_NAME:Irrlicht>)
	SET(IRRLICHT_SHARED_FILE $<TARGET_FILE_NAME:Irrlicht>)

	SET(ZZZ_EXAMPLE_SINGLE_COMPILE OFF CACHE BOOL "(Do not set!) If this is enabled, cmake assumes that an example should be compiled for itself (This variable is set automatically, so do not set!).")
	mark_as_advanced(FORCE ZZZ_EXAMPLE_SINGLE_COMPILE)
	SET(IRRLICHT_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include/" CACHE PATH "The include directory for Irrlicht headers.")
	mark_as_advanced(FORCE IRRLICHT_INCLUDE_DIR)
	SET(IRRLICHT_LIBRARY     "${CMAKE_CURRENT_BINARY_DIR}/${IRRLICHT_LIB_FILE}" CACHE FILEPATH "The path to the irrlicht library.")
	mark_as_advanced(FORCE IRRLICHT_LIBRARY)
	if (NOT IRRLICHT_STATIC_LIBRARY)
		SET(IRRLICHT_SHARED_BIN  "${CMAKE_CURRENT_BINARY_DIR}/${IRRLICHT_SHARED_FILE}" CACHE FILEPATH "The path to the irrlicht shared library.")
		mark_as_advanced(FORCE IRRLICHT_SHARED_BIN)
	endif ()

	if (IRRLICHT_BUILD_EXAMPLES)
		ADD_SUBDIRECTORY(examples)
	endif ()
	
	if 	(ZZZ_EXAMPLE_SINGLE_COMPILE)
		message(SEND_ERROR "The variable ZZZ_EXAMPLE_SINGLE_COMPILE must not be set when compiling the irrlicht library!")
	endif ()
endif ()

