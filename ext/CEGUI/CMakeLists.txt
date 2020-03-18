include(../../cmake/common.cmake)
update_git_submodule(./stb)

# to remove later, only adding my source
set(IRR_EXT_CEGUI_H
  ExtCEGUI.h
	CEGUIOpenGLState.h
	CEGUIOpenGLClip.h
	Helpers.h
	"IrrlichtBaW/include/ExtGeometryBuffer.h"
	"IrrlichtBaW/include/ExtImageCodec.h"
	"IrrlichtBaW/include/ExtRenderer.h"
	"IrrlichtBaW/include/ExtRenderTarget.h"
	"IrrlichtBaW/include/ExtShaders.h"
	"IrrlichtBaW/include/ExtTexture.h"
	"IrrlichtBaW/include/ExtTextureTarget.h"
	"IrrlichtBaW/include/ExtViewportTarget.h"
)

set(IRR_EXT_CEGUI_SRC
  ExtCEGUI.cpp
	CEGUIOpenGLState.cpp
	CEGUIOpenGLClip.cpp
	Helpers.cpp
	"IrrlichtBaW/src/ExtGeometryBuffer.cpp"
	"IrrlichtBaW/src/ExtImageCodec.cpp"
	"IrrlichtBaW/src/ExtRenderer.cpp"
	"IrrlichtBaW/src/ExtTexture.cpp"
	"IrrlichtBaW/src/ExtTextureTarget.cpp"
	"IrrlichtBaW/src/ExtViewportTarget.cpp"
)

set(IRR_EXT_CEGUI_EXTERNAL_INCLUDE
  "${CEGUI_INCLUDE_PATH}"
  "${IRR_ROOT_PATH}/3rdparty"
  "${CMAKE_BINARY_DIR}/3rdparty/CEGUI/dependencies/install/include"
  "${IRR_ROOT_PATH}/src/3rdparty/"
)

add_compile_definitions(CEGUI_STATIC CEGUI_BUILD_STATIC_FACTORY_MODULE)

irr_create_ext_library_project(
  CEGUI
  "${IRR_EXT_CEGUI_H}"
  "${IRR_EXT_CEGUI_SRC}"
  "${IRR_EXT_CEGUI_EXTERNAL_INCLUDE}"
  ""
)

add_dependencies(IrrExtCEGUI CEGUIBase-0_Static)
add_dependencies(IrrExtCEGUI CEGUICommonDialogs-0_Static)
add_dependencies(IrrExtCEGUI CEGUICoreWindowRendererSet_Static)
add_dependencies(IrrExtCEGUI CEGUIExpatParser_Static)
add_dependencies(IrrExtCEGUI CEGUIOpenGLRenderer-0_Static)
add_dependencies(IrrExtCEGUI CEGUISTBImageCodec_Static)
