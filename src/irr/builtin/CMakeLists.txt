cmake_minimum_required(VERSION 3.9)

function(irr_add_resource filename variable)
  message(STATUS "Starting generating resource from ${filename}")
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/build/${filename}.h
    COMMAND "${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/embedResource.py ${CMAKE_CURRENT_SOURCE_DIR}/${filename} ${variable} ${CMAKE_CURRENT_BINARY_DIR}/build/${filename}.h"
    COMMENT "Generated resource from ${filename}"
    VERBATIM
  )
  add_custom_target(builtin_resource DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/build/${filename}.h)

  # execute_process(COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/embedResource.py ${CMAKE_CURRENT_SOURCE_DIR}/${filename})
endfunction()


if(IRR_EMBED_BUILTIN_RESOURCES)
	irr_add_resource(shader1.txt embeddedShader)
endif()