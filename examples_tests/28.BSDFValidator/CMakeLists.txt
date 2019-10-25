
include(common RESULT_VARIABLE RES)
if(NOT RES)
	message(FATAL_ERROR "common.cmake not found. Should be in {repo_root}/cmake directory")
endif()

update_git_submodule(../media/cegui_alfisko)


add_compile_definitions(CEGUI_STATIC CEGUI_BUILD_STATIC_FACTORY_MODULE)

set(BSDF_VALIDATOR_LIBS
	${IRR_EXT_CEGUI_LIB}
	${CEGUI_LIBS}
)


irr_create_executable_project(
	""
	""
	"${IRR_EXT_CEGUI_INCLUDE_DIRS}"
	"${BSDF_VALIDATOR_LIBS}"
)
