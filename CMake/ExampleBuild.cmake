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

SET(CMAKE_MODULE_PATH
	${CMAKE_MODULE_PATH}
	${CMAKE_CURRENT_LIST_DIR}/Dependencies
	${CMAKE_CURRENT_LIST_DIR}/Compiler
	${CMAKE_CURRENT_LIST_DIR}/OperatingSystems
	${CMAKE_CURRENT_LIST_DIR}/Options
	${CMAKE_CURRENT_LIST_DIR}/Packages
)

FUNCTION(BUILD_APP_GENERIC APP_NAME APP_SOURCE_FILES APP_HEADER_FILES APP_INSTALL_FILES, APP_INSTALL_DIRS APP_IS_WIN32 APP_INSTALL_PATH)

	message(STATUS "    * Configure: ${APP_NAME}...")
	project(${APP_NAME})

	# Set variables for irrlicht and media file reference. When this CMakeList file is called from Irrlicht build as a subproject, then the Irrlicht build will overwrite them
	SET(ZZZ_EXAMPLE_SINGLE_COMPILE ON CACHE BOOL "(Do not set!) If this is enabled, cmake assumes that an example should be compiled for itself (This variable is set automatically, so do not set!).")
	mark_as_advanced(FORCE ZZZ_EXAMPLE_SINGLE_COMPILE)
	SET(IRRLICHT_INSTALL_EXAMPLE_SOURCE ON CACHE BOOL "If this is enabled, cmake will install source files for the example.")
	SET(IRRLICHT_MEDIA_DIR   "${CMAKE_SOURCE_DIR}/../../media" CACHE PATH "The directory where the media files are stored in.")
	SET(IRRLICHT_INCLUDE_DIR "NOT-FOUND" CACHE PATH "The include directory for Irrlicht headers.")
	SET(IRRLICHT_LIBRARY     "NOT-FOUND" CACHE FILEPATH "The path to the irrlicht library.")
	SET(IRRLICHT_STATIC_LIBRARY OFF CACHE BOOL "Enabled static lib for irrlicht.")
	if (NOT IRRLICHT_STATIC_LIBRARY)
		SET(IRRLICHT_SHARED_BIN  "NOT-FOUND" CACHE FILEPATH "The path to the irrlicht shared library.")
	endif ()

	# Add irrlicht include dir
	INCLUDE_DIRECTORIES(
		${IRRLICHT_INCLUDE_DIR}
	)

	# Handle source files
	SET_SOURCE_FILES_PROPERTIES(APP_HEADER_FILES
		PROPERTIES HEADER_FILE_ONLY TRUE
	)

	SET(ALL_APP_SOURCE_FILES
		${APP_SOURCE_FILES}
		${APP_HEADER_FILES}
	)

	if (IRRLICHT_INSTALL_EXAMPLE_SOURCE)
		SET(APP_INSTALL_FILES
			${APP_INSTALL_FILES}
			${ALL_APP_SOURCE_FILES}
		)
	endif ()

	# When the examples are build without irrlicht library, we need to apply the same settings like for irrlicht
	if (ZZZ_EXAMPLE_SINGLE_COMPILE)
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
		INCLUDE(DependencyFontConfig)
		INCLUDE(DependencyX11)
		INCLUDE(DependencyFreeType)

	endif ()

	# Cache back the flags (seems to be an error of cmake for msys makefile generator)
	if (NOT CFLAG_WRITE_BACK)
		SET(CFLAG_WRITE_BACK TRUE CACHE BOOL "(do not change!)" FORCE)
		mark_as_advanced(FORCE CFLAG_WRITE_BACK)
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE STRING "compile flags" FORCE)
		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "compile flags" FORCE)
	endif ()

	# Create target
	if (APP_IS_WIN32)
		ADD_EXECUTABLE(${APP_NAME} WIN32 ${ALL_APP_SOURCE_FILES})
	else ()
		ADD_EXECUTABLE(${APP_NAME} ${ALL_APP_SOURCE_FILES})
	endif ()
	#TARGET_LINK_LIBRARIES(${APP_NAME} ${IRRLICHT_LIBRARY} ${IRRLICHT_DEPENDENCY_LIBRARIES} ${OS_DEPENDENT_LIBRARIES})
        TARGET_LINK_LIBRARIES(${APP_NAME} Irrlicht ${IRRLICHT_DEPENDENCY_LIBRARIES} ${OS_DEPENDENT_LIBRARIES})
	# Target installation
	INSTALL(TARGETS ${APP_NAME}
		RUNTIME DESTINATION  "${APP_INSTALL_PATH}" CONFIGURATIONS Debug Release MinSizeRel RelWithDebInfo
	)

	# Install additional files
	if (NOT "${APP_INSTALL_FILES}" STREQUAL "")
		INSTALL(FILES ${APP_INSTALL_FILES} DESTINATION "${APP_INSTALL_PATH}")
	endif ()

	# Install additional directories
	if (NOT "${APP_INSTALL_DIRS}" STREQUAL "")
		INSTALL(DIRECTORY ${APP_INSTALL_DIRS} DESTINATION "${APP_INSTALL_PATH}")
	endif ()

	# Install shared library
	if (NOT IRRLICHT_STATIC_LIBRARY)
		INSTALL(FILES ${IRRLICHT_SHARED_BIN} DESTINATION  "${APP_INSTALL_PATH}")
	endif()

	# Install media files
	if (ZZZ_EXAMPLE_SINGLE_COMPILE)
		if (IRRLICHT_INSTALL_MEDIA_FILES)
			INSTALL(DIRECTORY ${IRRLICHT_MEDIA_DIR} DESTINATION  ".")
		endif ()
	endif()

	# Basic settings check, in case the example is build as stand-alone
	if (ZZZ_EXAMPLE_SINGLE_COMPILE)

		# check irrlicht include directory
		if (NOT EXISTS "${IRRLICHT_INCLUDE_DIR}/")
			message(SEND_ERROR "Irrlicht include path ${IRRLICHT_INCLUDE_DIR} does not exist. Build example might not be possible...")
		endif ()

		# check if irrlicht library is in place
		if (NOT EXISTS "${IRRLICHT_LIBRARY}")
			message(SEND_ERROR "Irrlicht library ${IRRLICHT_LIBRARY} does not exist. Build example might not be possible...")
		endif ()

		# check if shared irrlicht library is in place if needed
		if (NOT IRRLICHT_STATIC_LIBRARY)
			if(NOT EXISTS "${IRRLICHT_SHARED_BIN}")
				message(SEND_ERROR "Irrlicht shared library ${IRRLICHT_SHARED_BIN} does not exist. Build example might not be possible...")
			endif ()
		endif ()

		# check if media files are in place if they should be installed
		if (IRRLICHT_INSTALL_MEDIA_FILES)
			if (NOT EXISTS "${IRRLICHT_MEDIA_DIR}/")
				message(WARNING "Irrlicht media path ${IRRLICHT_MEDIA_DIR} does not exist. Build example will not run without media files...")
			endif ()
		endif ()

	endif ()

ENDFUNCTION()

FUNCTION(BUILD_EXAMPLE EXAMPLE_NAME EXAMPLE_SOURCE_FILES EXAMPLE_HEADER_FILES EXAMPLE_INSTALL_FILES, EXAMPLE_INSTALL_DIRS)
	BUILD_APP_GENERIC("${EXAMPLE_NAME}" "${EXAMPLE_SOURCE_FILES}" "${EXAMPLE_HEADER_FILES}" "${EXAMPLE_INSTALL_FILES}" "${EXAMPLE_INSTALL_DIRS}" FALSE "example/${EXAMPLE_NAME}")
ENDFUNCTION()

FUNCTION(BUILD_WIN32_EXAMPLE EXAMPLE_NAME EXAMPLE_SOURCE_FILES EXAMPLE_HEADER_FILES EXAMPLE_INSTALL_FILES, EXAMPLE_INSTALL_DIRS)
	BUILD_APP_GENERIC("${EXAMPLE_NAME}" "${EXAMPLE_SOURCE_FILES}" "${EXAMPLE_HEADER_FILES}" "${EXAMPLE_INSTALL_FILES}" "${EXAMPLE_INSTALL_DIRS}" TRUE "example/${EXAMPLE_NAME}")
ENDFUNCTION()
