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
	ERPA_XML_SCHEMES,

	ERPA_COUNT
};

constexpr std::array<ResourceProviderAlias, ERPA_COUNT> resourceProviderAliases =
{
	ResourceProviderAlias("schemes", "../../media/cegui_alfisko/schemes/"),
	ResourceProviderAlias("imagesets", "../../media/cegui_alfisko/imagesets/"),
	ResourceProviderAlias("fonts", "../../media/cegui_alfisko/fonts/"),
	ResourceProviderAlias("layouts", "../../media/cegui_alfisko/layouts/"),
	ResourceProviderAlias("looknfeels", "../../media/cegui_alfisko/looknfeel/"),
	ResourceProviderAlias("xml_schemas", "../../media/cegui_alfisko/xml_schemas/")
};

int main()
{
	irr::SIrrlichtCreationParameters params;
	params.Bits = 24; 
	params.ZBufferBits = 24; 
	params.DriverType = video::EDT_OPENGL; 
	params.WindowSize = dimension2d<uint32_t>(800, 600);
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

	CEGUI::ImageManager::setImagesetDefaultResourceGroup("imagesets");
	CEGUI::Font::setDefaultResourceGroup("fonts");
	CEGUI::Scheme::setDefaultResourceGroup("schemes");
	CEGUI::WidgetLookManager::setDefaultResourceGroup("looknfeels");
	CEGUI::WindowManager::setDefaultResourceGroup("layouts");

	CEGUI::XMLParser* parser = CEGUI::System::getSingleton().getXMLParser();
	if (parser->isPropertyPresent("SchemaDefaultResourceGroup"))
		parser->setProperty("SchemaDefaultResourceGroup", resourceProviderAliases[ERPA_XML_SCHEMES].alias.data());

	// load a scheme
	CEGUI::SchemeManager::getSingleton().createFromFile("Alfisko.scheme");

	// since schemes contains some data, we can use it to set some assets
	CEGUI::System::getSingleton().getDefaultGUIContext().setDefaultFont("DejaVuSans-12");
	CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor().setDefaultImage("Alfisko/MouseArrow");

	auto myRoot = CEGUI::WindowManager::getSingleton().createWindow("DefaultWindow", "_MasterRoot");
	CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(myRoot);

	device->getCursorControl()->setVisible(false);
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






