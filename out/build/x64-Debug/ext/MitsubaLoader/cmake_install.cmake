# Install script for directory: C:/Projects/IrrlichtBAW/ext/MitsubaLoader

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Projects/IrrlichtBAW/out/install/x64-Debug")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./include/irr/ext/MITSUBA_LOADER" TYPE FILE FILES
      "C:/Projects/IrrlichtBAW/ext/MitsubaLoader/CElementEmitter.h"
      "C:/Projects/IrrlichtBAW/ext/MitsubaLoader/CElementFactory.h"
      "C:/Projects/IrrlichtBAW/ext/MitsubaLoader/CElementFilm.h"
      "C:/Projects/IrrlichtBAW/ext/MitsubaLoader/CElementSampler.h"
      "C:/Projects/IrrlichtBAW/ext/MitsubaLoader/CElementSensor.h"
      "C:/Projects/IrrlichtBAW/ext/MitsubaLoader/CElementTransform.h"
      "C:/Projects/IrrlichtBAW/ext/MitsubaLoader/CMitsubaLoader.h"
      "C:/Projects/IrrlichtBAW/ext/MitsubaLoader/CMitsubaScene.h"
      "C:/Projects/IrrlichtBAW/ext/MitsubaLoader/CShapeCreator.h"
      "C:/Projects/IrrlichtBAW/ext/MitsubaLoader/IElement.h"
      "C:/Projects/IrrlichtBAW/ext/MitsubaLoader/ParserUtil.h"
      "C:/Projects/IrrlichtBAW/ext/MitsubaLoader/PropertyElement.h"
      "C:/Projects/IrrlichtBAW/ext/MitsubaLoader/Shape.h"
      )
  endif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./debug/include/irr/ext/MITSUBA_LOADER" TYPE FILE FILES
      "C:/Projects/IrrlichtBAW/ext/MitsubaLoader/CElementEmitter.h"
      "C:/Projects/IrrlichtBAW/ext/MitsubaLoader/CElementFactory.h"
      "C:/Projects/IrrlichtBAW/ext/MitsubaLoader/CElementFilm.h"
      "C:/Projects/IrrlichtBAW/ext/MitsubaLoader/CElementSampler.h"
      "C:/Projects/IrrlichtBAW/ext/MitsubaLoader/CElementSensor.h"
      "C:/Projects/IrrlichtBAW/ext/MitsubaLoader/CElementTransform.h"
      "C:/Projects/IrrlichtBAW/ext/MitsubaLoader/CMitsubaLoader.h"
      "C:/Projects/IrrlichtBAW/ext/MitsubaLoader/CMitsubaScene.h"
      "C:/Projects/IrrlichtBAW/ext/MitsubaLoader/CShapeCreator.h"
      "C:/Projects/IrrlichtBAW/ext/MitsubaLoader/IElement.h"
      "C:/Projects/IrrlichtBAW/ext/MitsubaLoader/ParserUtil.h"
      "C:/Projects/IrrlichtBAW/ext/MitsubaLoader/PropertyElement.h"
      "C:/Projects/IrrlichtBAW/ext/MitsubaLoader/Shape.h"
      )
  endif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./lib/irr/ext/MITSUBA_LOADER" TYPE STATIC_LIBRARY FILES "C:/Projects/IrrlichtBAW/out/build/x64-Debug/ext/MitsubaLoader/IrrExtMITSUBA_LOADER_d.lib")
  endif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/./debug/lib/irr/ext/MITSUBA_LOADER" TYPE STATIC_LIBRARY FILES "C:/Projects/IrrlichtBAW/out/build/x64-Debug/ext/MitsubaLoader/IrrExtMITSUBA_LOADER_d.lib")
  endif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
endif()

