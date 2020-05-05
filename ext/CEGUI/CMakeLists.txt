include(../../cmake/common.cmake)
update_git_submodule(./stb)

set(IRR_EXT_CEGUI_H
  ExtCEGUI.h
	CEGUIOpenGLState.h
	CEGUIOpenGLClip.h
	Helpers.h
	IrrlichtBaW/IrrlichtBaWRenderer.hpp
	IrrlichtBaW/IrrlichtBaWRenderTarget.hpp
	IrrlichtBaW/IrrlichtBaWRenderQueue.hpp
	IrrlichtBaW/IrrlichtBaWGeometryBuffer.hpp
	IrrlichtBaW/IrrlichtBaWRenderingWindow.hpp
	IrrlichtBaW/IrrlichtBaWRenderEffect.hpp
	IrrlichtBaW/IrrlichtBaWTextureTarget.hpp
	IrrlichtBaW/IrrlichtBaWTexture.hpp
)

set(IRR_EXT_CEGUI_SRC
  ExtCEGUI.cpp
	CEGUIOpenGLState.cpp
	CEGUIOpenGLClip.cpp
	Helpers.cpp
	IrrlichtBaW/IrrlichtBaWRenderer.cpp
	IrrlichtBaW/IrrlichtBaWRenderTarget.cpp
	IrrlichtBaW/IrrlichtBaWRenderQueue.cpp
	IrrlichtBaW/IrrlichtBaWGeometryBuffer.cpp
	IrrlichtBaW/IrrlichtBaWRenderingWindow.cpp
	IrrlichtBaW/IrrlichtBaWRenderEffect.cpp
	IrrlichtBaW/IrrlichtBaWTextureTarget.cpp
	IrrlichtBaW/IrrlichtBaWTexture.cpp
)

set(IRR_EXT_CEGUI_EXTERNAL_INCLUDE
  "${CEGUI_INCLUDE_PATH}"
  "${IRR_ROOT_PATH}/3rdparty"
  "${CMAKE_BINARY_DIR}/3rdparty/CEGUI/dependencies/install/include"
  "${IRR_ROOT_PATH}/src/3rdparty/"
  "${CMAKE_CURRENT_LIST_DIR}/IrrlichtBaW"
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
