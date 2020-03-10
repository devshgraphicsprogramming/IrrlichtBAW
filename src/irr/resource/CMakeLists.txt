cmake_minimum_required(VERSION 3.9)

function(irr_add_resource varname filename outputfilename)
  execute_process(COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/embedResource.py ${varname} ${CMAKE_CURRENT_SOURCE_DIR}/${outputfilename} ${CMAKE_CURRENT_SOURCE_DIR}/${filename})
  message(STATUS "Generated resource ${varname}")
endfunction()


if(_IRR_RESOURCE_COMPILE_EVEN_IN_DEBUG)

	irr_add_resource(variableName exampleResource.txt exampleResOutput)

endif()