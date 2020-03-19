function(irr_add_builtin_resource)

  # generate header file
  add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BUILD_DIR}/builtinResources.h" "${CMAKE_CURRENT_BUILD_DIR}/builtinResourceData.cpp"
    COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/builtinHeaderGen.py ${CMAKE_CURRENT_BUILD_DIR}/builtinResources.h ${CMAKE_SOURCE_DIR} ${BUILTIN_RESOURCE_LIST}
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/builtinDataGen.py ${CMAKE_CURRENT_BUILD_DIR}/builtinResourceData.cpp ${CMAKE_SOURCE_DIR} ${BUILTIN_RESOURCE_LIST}
    COMMENT "Generated built-in resources"
    VERBATIM
  )

  # set up dependency
  add_custom_target(builtin_resources DEPENDS "${CMAKE_CURRENT_BUILD_DIR}/builtinResources.h" "${CMAKE_CURRENT_BUILD_DIR}/builtinResourceData.cpp")
  add_dependencies(Irrlicht builtin_resources)

endfunction()


if(IRR_EMBED_BUILTIN_RESOURCES)
  #fill this list with resource paths relative to top level of IrrlichtBaW's source
  set(BUILTIN_RESOURCE_LIST src/irr/builtin/shader1.txt)

	irr_add_builtin_resource()
endif()