#define _IRR_STATIC_LIB_
#include <iostream>
#include <cstdio>
#include <irrlicht.h>
#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/OpenGL/GL3Renderer.h>
#include <CEGUI/System.h>

#include "../common/QToQuitEventReceiver.h"

using namespace irr;
using namespace core;

/*
	For AnastaZIuk CEGUI learning purposes,
	doing according to documentation and tutorials
	from CEGUI's webpage.

	The example will be deleted soon.
*/

class ResourceProviderAlias
{
	public:

		constexpr ResourceProviderAlias(const std::string_view _alias, const std::string_view _path) : alias(_alias), path(_path) {}

		std::string_view alias;
		std::string_view path;
};

enum E_RESOURCE_PROVIDER_ALIAS
{
	ERPA_SCHEMES,
	ERPA_IMAGESETS,
	ERPA_FONTS,
	ERPA_LAYOUTS,
	ERPA_LOOKNFEELS,
	ERPA_LUA_SCRIPTS,
	ERPA_XML_SCHEMES,

	ERPA_COUNT
};

constexpr std::array<ResourceProviderAlias, ERPA_COUNT> resourceProviderAliases =
{
	ResourceProviderAlias("schemes", "../../media/cegui_data_files/schemes/"),
	ResourceProviderAlias("imagesets", "../../media/cegui_data_files/imagesets/"),
	ResourceProviderAlias("fonts", "../../media/cegui_data_files/fonts/"),
	ResourceProviderAlias("layouts", "../../media/cegui_data_files/layouts/"),
	ResourceProviderAlias("looknfeels", "../../media/cegui_data_files/looknfeel/"),
	ResourceProviderAlias("lua_scripts", "../../media/cegui_data_files/lua_scripts/"),
	ResourceProviderAlias("xml_schemas", "../../media/cegui_data_files/xml_schemas/")
};

int main()
{
	irr::SIrrlichtCreationParameters params;
	params.Bits = 24; 
	params.ZBufferBits = 24; 
	params.DriverType = video::EDT_OPENGL; 
	params.WindowSize = dimension2d<uint32_t>(1280, 720);
	params.Fullscreen = false;
	params.Vsync = true; 
	params.Doublebuffer = true;
	params.Stencilbuffer = false;
	auto device = createDeviceEx(params);

	if (!device)
		return 1; 

	CEGUI::OpenGL3Renderer& guiRenderer = CEGUI::OpenGL3Renderer::bootstrapSystem();
	auto guiResourceProvider = reinterpret_cast<CEGUI::DefaultResourceProvider*>(CEGUI::System::getSingleton().getResourceProvider());

	for(auto& resourceProvider : resourceProviderAliases)
		guiResourceProvider->setResourceGroupDirectory(resourceProvider.alias.data(), resourceProvider.path.data());

	CEGUI::ImageManager::setImagesetDefaultResourceGroup(resourceProviderAliases[ERPA_IMAGESETS].alias.data());
	CEGUI::Font::setDefaultResourceGroup(resourceProviderAliases[ERPA_FONTS].alias.data());
	CEGUI::Scheme::setDefaultResourceGroup(resourceProviderAliases[ERPA_SCHEMES].alias.data());
	CEGUI::WidgetLookManager::setDefaultResourceGroup(resourceProviderAliases[ERPA_LOOKNFEELS].alias.data());
	CEGUI::WindowManager::setDefaultResourceGroup(resourceProviderAliases[ERPA_LAYOUTS].alias.data());
	CEGUI::ScriptModule::setDefaultResourceGroup(resourceProviderAliases[ERPA_LUA_SCRIPTS].alias.data());

	CEGUI::XMLParser* parser = CEGUI::System::getSingleton().getXMLParser();
	if (parser->isPropertyPresent("SchemaDefaultResourceGroup"))
		parser->setProperty("schemas", resourceProviderAliases[ERPA_XML_SCHEMES].alias.data());

	// load a scheme
	auto& taharezLookScheme = CEGUI::SchemeManager::getSingleton().createFromFile("TaharezLook.scheme");

	auto& mouse = CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor();
	mouse.setDefaultImage("TaharezLook/MouseArrow");
	mouse.setVisible(true);

	CEGUI::System::getSingleton().getDefaultGUIContext().setDefaultTooltipType("TaharezLook/Tooltip");

	/*  
		explicitly load layout contating useful data stuff such as positions or
		sizes of widgets (windows) to avoid doing it manually as some steps bellow
	*/

	auto myRoot = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("TaharezLookOverview.layout", resourceProviderAliases[ERPA_LAYOUTS].alias.data());
	CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(myRoot);

	/*

	auto myRoot = CEGUI::WindowManager::getSingleton().createWindow("DefaultWindow", "root");
	CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(myRoot);

	auto taharezFrameWindow = static_cast<CEGUI::FrameWindow*>(CEGUI::WindowManager::getSingleton().createWindow("TaharezLook/FrameWindow", "testWindow"));
	myRoot->addChild(taharezFrameWindow);

	taharezFrameWindow->setPosition(CEGUI::UVector2(CEGUI::UDim(0.25f, 0.0f), CEGUI::UDim(0.25f, 0.0f)));	// position a quarter of the way in from the top-left of parent.
	taharezFrameWindow->setSize(CEGUI::USize(CEGUI::UDim(0.5f, 0.0f), CEGUI::UDim(0.5f, 0.0f)));			// set size to be half the size of the parent
	taharezFrameWindow->setText("Hello IrrlichtBaW! :)");

	*/

	device->getCursorControl()->setVisible(true);
	QToQuitEventReceiver receiver;
	device->setEventReceiver(&receiver);

	auto* driver = device->getVideoDriver();
	auto* smgr = device->getSceneManager();

	uint64_t lastFPSTime = 0;
	while(device->run() && receiver.keepOpen())
	{
		driver->beginScene(true, true, video::SColor(255,255,255,255) );
        
		// TODO render in 3D using IrrlichtBaW API
		// render()

		// rendering GUI stuff
		CEGUI::System::getSingleton().getDefaultGUIContext().injectMousePosition(400, 300);
		CEGUI::System::getSingleton().renderAllGUIContexts();
        
		driver->endScene();

	
		uint64_t time = device->getTimer()->getRealTime();
		if (time-lastFPSTime > 1000)
		{
			std::wostringstream str;
			str << L"GPU Mesh Demo - Irrlicht Engine [" << driver->getName() << "] FPS:" << driver->getFPS() << " PrimitvesDrawn:" << driver->getPrimitiveCountDrawn();

			device->setWindowCaption(str.str().c_str());
			lastFPSTime = time;
		}
	}

	CEGUI::System::destroy();
	CEGUI::OpenGL3Renderer::destroy(static_cast<CEGUI::OpenGL3Renderer&>(guiRenderer));

	return 0;
}






