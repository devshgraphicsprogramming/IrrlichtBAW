include(common RESULT_VARIABLE RES)
if(NOT RES)
	message(FATAL_ERROR "common.cmake not found. Should be in {repo_root}/cmake directory")
endif()

update_git_submodule(../media/cegui_alfisko)

add_compile_definitions(CEGUI_STATIC CEGUI_BUILD_STATIC_FACTORY_MODULE)

set(IRRLICHT_RENDERER_CPP
	"${CMAKE_CURRENT_LIST_DIR}/source/SceneManager.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/source/GUIManager.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/source/ApplicationHandler.cpp"
)

set(IRRLICHT_RENDERER_HPP
	"${CMAKE_CURRENT_LIST_DIR}/include/SceneManager.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/include/GUIManager.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/include/ApplicationHandler.hpp"
)

set(IRRLICHT_RENDERER_SOURCE
	"${IRRLICHT_RENDERER_CPP}"
	"${IRRLICHT_RENDERER_HPP}"
)

set(IRRLICHT_RENDERER_INCLUDE_PATH
	"${IRR_EXT_CEGUI_INCLUDE_DIRS}"
	"${CMAKE_CURRENT_LIST_DIR}/include"
)

set(CEGUI_TEST_LIBS
	${IRR_EXT_CEGUI_LIB}
	${CEGUI_LIBS}
)

irr_create_executable_project(
	"${IRRLICHT_RENDERER_SOURCE}"
	""
	"${IRRLICHT_RENDERER_INCLUDE_PATH}"
	"${CEGUI_TEST_LIBS}"
)