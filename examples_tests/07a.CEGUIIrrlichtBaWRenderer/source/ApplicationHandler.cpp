#include "ApplicationHandler.hpp"

ApplicationHandler::ApplicationHandler(irr::core::smart_refctd_ptr<irr::IrrlichtDevice> _device)
	: guiManager(_device), sceneManager(_device), device(_device)
{
	device->getCursorControl()->setVisible(true);
	device->setEventReceiver(&quitEventReceiver);

    setupManagers();
    device->setEventReceiver(&guiManager);
}

ApplicationHandler::~ApplicationHandler()
{

}

bool ApplicationHandler::getStatus()
{
	return status;
}

void ApplicationHandler::work()
{
	auto driver = device->getVideoDriver();

	uint64_t lastFPSTime = 0;
	while (device->run() && quitEventReceiver.keepOpen())
	{
		driver->beginScene(true, true, irr::video::SColor(255, 255, 255, 255));

		// TODO: render scene (render before GUI!)

		guiManager.render();

		driver->endScene();

		uint64_t time = device->getTimer()->getRealTime();
		if (time - lastFPSTime > 1000)
		{
			std::wostringstream str;
			str << L"IrrlichtBaW Renderer [" << driver->getName() << "] FPS:" << driver->getFPS() << " PrimitvesDrawn:" << driver->getPrimitiveCountDrawn();

			device->setWindowCaption(str.str().c_str());
			lastFPSTime = time;
		}
	}
}

void ApplicationHandler::setupManagers()
{
	auto setupGUI = [&]()
	{
		guiManager.createRootWindowFromLayout(guiManager.readWindowLayout("../../media/brdf_explorer/MainWindow.layout"));
		
		auto onEventForColorPicked = [](const ::CEGUI::Colour& _ceguiColor, irr::core::vector3df& _irrColor) 
		{
			_irrColor.X = _ceguiColor.getRed();
			_irrColor.Y = _ceguiColor.getGreen();
			_irrColor.Z = _ceguiColor.getBlue();
		};

		guiManager.createColourPicker(false, "LightParamsWindow/ColorWindow", "Color", "pickerLightColor", std::bind(onEventForColorPicked, std::placeholders::_1, std::ref(commonData.guiState.light.color)));
		guiManager.createColourPicker(true, "MaterialParamsWindow/EmissiveWindow", "Emissive", "pickerEmissiveColor", std::bind(onEventForColorPicked, std::placeholders::_1, std::ref(commonData.guiState.emissive.color)));
		guiManager.createColourPicker(true, "MaterialParamsWindow/AlbedoWindow", "Albedo", "pickerAlbedoColor", std::bind(onEventForColorPicked, std::placeholders::_1, std::ref(commonData.guiState.albedo.constantColor)));

        auto guiRootWindow = guiManager.getRootWindow();
        // Material window: Subscribe to sliders' events and set its default value to
        // 0.0.
        for (uint32_t i = 0u; i < 6u; ++i)
        {
            const std::string RIWindowName = "MaterialParamsWindow/RefractionIndexWindow" + std::to_string(i + 1u);

            guiManager.registerSliderEvent((RIWindowName + "/Slider").c_str(), i < 3u ? CommondData::sliderRealRIRange : CommondData::sliderImagRIRange, 0.01f,
                [guiRootWindow, this, RIWindowName, i](const ::CEGUI::EventArgs&)
                {
                    auto val = static_cast<::CEGUI::Slider*>(guiRootWindow->getChild(RIWindowName + "/Slider"))->getCurrentValue();
                    val = std::max(val, i < 3u ? 0.04f : 0.f);
                    
                    guiRootWindow->getChild(RIWindowName + "/LabelPercent")->setText(GUIManager::toStringFloat(val, 2));

                    if (i < 3u) 
                        (&commonData.guiState.refractionIndex.constantReal.X)[i % 3] = val;     // real
                    else 
                        (&commonData.guiState.refractionIndex.constantImag.X)[i % 3] = val;     // imag
                });
        }

        guiManager.registerSliderEvent("MaterialParamsWindow/MetallicWindow/Slider", CommondData::sliderMetallicRange, 0.01f,
            [guiRootWindow, this](const CEGUI::EventArgs&)
            {
                auto metallic = static_cast<::CEGUI::Slider*>(guiRootWindow->getChild("MaterialParamsWindow/MetallicWindow/Slider"))->getCurrentValue();
                guiRootWindow->getChild("MaterialParamsWindow/MetallicWindow/LabelPercent")->setText(GUIManager::toStringFloat(metallic, 2));

                commonData.guiState.metallic.constValue = metallic;
            });

        guiManager.registerSliderEvent("MaterialParamsWindow/RoughnessWindow/Slider", CommondData::sliderRoughness1Range, 0.01f,
            [guiRootWindow, this](const ::CEGUI::EventArgs&)
            {
                const auto sliderValue = static_cast<::CEGUI::Slider*>(guiRootWindow->getChild("MaterialParamsWindow/RoughnessWindow/Slider"))->getCurrentValue();
                const auto sliderValueText = GUIManager::toStringFloat(sliderValue, 2);

                guiRootWindow->getChild("MaterialParamsWindow/RoughnessWindow/LabelPercent1")->setText(sliderValueText);

                commonData.guiState.roughness.constValue1 = sliderValue;
            });

        guiManager.registerSliderEvent("MaterialParamsWindow/RoughnessWindow/Slider2", CommondData::sliderRoughness2Range, 0.01f,
            [guiRootWindow, this](const ::CEGUI::EventArgs&)
            {
                auto roughness = static_cast<::CEGUI::Slider*>(guiRootWindow->getChild("MaterialParamsWindow/RoughnessWindow/Slider2"))->getCurrentValue();
                guiRootWindow->getChild("MaterialParamsWindow/RoughnessWindow/LabelPercent2")->setText(GUIManager::toStringFloat(roughness, 2));

                commonData.guiState.roughness.constValue2 = roughness;
            });

        // Set the sliders' text objects to their default value (whatever value the
        // slider was set to).
        {
            // Roughness slider, first one
            guiRootWindow->getChild("MaterialParamsWindow/RoughnessWindow/LabelPercent2")->setText(GUIManager::toStringFloat(static_cast<::CEGUI::Slider*>(guiRootWindow->getChild("MaterialParamsWindow/RoughnessWindow/Slider2"))->getCurrentValue(), 2));

            // Roughness slider, second one
            guiRootWindow->getChild("MaterialParamsWindow/RoughnessWindow/LabelPercent1")->setText(GUIManager::toStringFloat(static_cast<::CEGUI::Slider*>(guiRootWindow->getChild("MaterialParamsWindow/RoughnessWindow/Slider"))->getCurrentValue(), 2));

            // Refractive index slider
            for (uint32_t i = 0u; i < 6u; ++i)
            {
                const std::string RIWindowName = "RefractionIndexWindow" + std::to_string(i + 1u);
                auto slider = static_cast<::CEGUI::Slider*>(guiRootWindow->getChild("MaterialParamsWindow/" + RIWindowName + "/Slider"));

                if (i < 3u)
                    slider->setCurrentValue(1.33f);
                float val = slider->getCurrentValue();
                guiRootWindow->getChild("MaterialParamsWindow/" + RIWindowName + "/LabelPercent")->setText(GUIManager::toStringFloat(val, 2));
            }

            // Metallic slider
            guiRootWindow->getChild("MaterialParamsWindow/MetallicWindow/LabelPercent")->setText(GUIManager::toStringFloat(static_cast<::CEGUI::Slider*>(guiRootWindow->getChild("MaterialParamsWindow/MetallicWindow/Slider"))->getCurrentValue(), 2));

            // Bump-mapping's height slider
            guiRootWindow->getChild("MaterialParamsWindow/BumpWindow/LabelPercent")->setText(GUIManager::toStringFloat(static_cast<::CEGUI::Slider*>(guiRootWindow->getChild("MaterialParamsWindow/BumpWindow/Spinner"))->getCurrentValue(), 2));
        }

        // light animation checkbox
        auto lightAnimated = static_cast<::CEGUI::ToggleButton*>(guiRootWindow->getChild("LightParamsWindow/AnimationWindow/Checkbox"));

        lightAnimated->subscribeEvent(CEGUI::ToggleButton::EventSelectStateChanged,
            [guiRootWindow, this](const CEGUI::EventArgs& e)
            {
                const ::CEGUI::WindowEventArgs& we = static_cast<const ::CEGUI::WindowEventArgs&>(e);
                commonData.guiState.light.animated = static_cast<::CEGUI::ToggleButton*>(we.window)->isSelected();

                guiRootWindow->getChild("LightParamsWindow/PositionWindow")->setDisabled(commonData.guiState.light.animated);
            }
        );

        lightAnimated->setSelected(true);

        guiManager.registerSliderEvent("LightParamsWindow/IntensityWindow/IntensitySlider", CommondData::sliderLightIntensityRange, 1.f,
            [guiRootWindow, this](const ::CEGUI::EventArgs&)
            {
                auto intensity = static_cast<::CEGUI::Slider*>(guiRootWindow->getChild("LightParamsWindow/IntensityWindow/IntensitySlider"))->getCurrentValue();
                commonData.guiState.light.intensity = intensity + 1.f;
            });

        static_cast<::CEGUI::Slider*>(guiRootWindow->getChild("LightParamsWindow/IntensityWindow/IntensitySlider"))->setCurrentValue(800.f);

        auto lightZ = static_cast<::CEGUI::Spinner*>(guiRootWindow->getChild("LightParamsWindow/PositionWindow/LightZ"));
        lightZ->subscribeEvent(CEGUI::Spinner::EventValueChanged,
            [guiRootWindow, this](const ::CEGUI::EventArgs& e)
            {
                const ::CEGUI::WindowEventArgs& we = static_cast<const ::CEGUI::WindowEventArgs&>(e);
                commonData.guiState.light.constantPosition.Z = static_cast<::CEGUI::Spinner*>(we.window)->getCurrentValue();
            }
        );

        auto lightY = static_cast<::CEGUI::Spinner*>(guiRootWindow->getChild("LightParamsWindow/PositionWindow/LightY"));
        lightY->subscribeEvent(CEGUI::Spinner::EventValueChanged,
            [guiRootWindow, this](const ::CEGUI::EventArgs& e)
            {
                const ::CEGUI::WindowEventArgs& we = static_cast<const ::CEGUI::WindowEventArgs&>(e);
                commonData.guiState.light.constantPosition.Y = static_cast<::CEGUI::Spinner*>(we.window)->getCurrentValue();
            }
        );

        auto lightX = static_cast<::CEGUI::Spinner*>(guiRootWindow->getChild("LightParamsWindow/PositionWindow/LightX"));
        lightX->subscribeEvent(CEGUI::Spinner::EventValueChanged,
            [guiRootWindow, this](const ::CEGUI::EventArgs& e)
            {
                const ::CEGUI::WindowEventArgs& we = static_cast<const ::CEGUI::WindowEventArgs&>(e);
                commonData.guiState.light.constantPosition.X = static_cast<::CEGUI::Spinner*>(we.window)->getCurrentValue();
            }
        );

        // Isotropic checkbox
        auto isotropic = static_cast<::CEGUI::ToggleButton*>(guiRootWindow->getChild("MaterialParamsWindow/RoughnessWindow/Checkbox"));
        isotropic->subscribeEvent(
            ::CEGUI::ToggleButton::EventSelectStateChanged,
            [guiRootWindow, this](const ::CEGUI::EventArgs& e)
            {
                const ::CEGUI::WindowEventArgs& we = static_cast<const ::CEGUI::WindowEventArgs&>(e);
                commonData.guiState.roughness.isIsotropic = static_cast<::CEGUI::ToggleButton*>(we.window)->isSelected();
                static_cast<::CEGUI::Slider*>(guiRootWindow->getChild("MaterialParamsWindow/RoughnessWindow/Slider2"))->setDisabled(commonData.guiState.roughness.isIsotropic);

                if (commonData.guiState.roughness.isIsotropic)
                    guiRootWindow->getChild("MaterialParamsWindow/RoughnessWindow/LabelPercent2")->setText(GUIManager::toStringFloat(static_cast<::CEGUI::Slider*>(guiRootWindow->getChild("MaterialParamsWindow/RoughnessWindow/Slider"))->getCurrentValue(), 2));
            });

        // Load Model button
        auto button_loadModel = static_cast<::CEGUI::PushButton*>(guiRootWindow->getChild("LoadModelButton"));

        button_loadModel->subscribeEvent(::CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&ApplicationHandler::onEventForMeshBrowse, this));

        // AO texturing & bump-mapping texturing window
        auto button_browse_AO = static_cast<::CEGUI::PushButton*>(guiRootWindow->getChild("MaterialParamsWindow/AOWindow/Button"));

        button_browse_AO->subscribeEvent(::CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&ApplicationHandler::onEventForAOTextureBrowse, this));

        static_cast<CEGUI::DefaultWindow*>(guiRootWindow->getChild("MaterialParamsWindow/AOWindow/ImageButton"))->subscribeEvent(::CEGUI::Window::EventMouseClick, CEGUI::Event::Subscriber(&ApplicationHandler::onEventForAOTextureBrowse, this));
        static_cast<CEGUI::Editbox*>(guiRootWindow->getChild("MaterialParamsWindow/AOWindow/Editbox"))->subscribeEvent(::CEGUI::Editbox::EventTextAccepted, CEGUI::Event::Subscriber(&ApplicationHandler::onEventForAOTextureBrowse_EditBox, this));

        auto ao_enabled = static_cast<::CEGUI::ToggleButton*>(guiRootWindow->getChild("MaterialParamsWindow/AOWindow/Checkbox"));
        ao_enabled->subscribeEvent(CEGUI::ToggleButton::EventSelectStateChanged,
            [guiRootWindow, this](const ::CEGUI::EventArgs& e)
            {
                const ::CEGUI::WindowEventArgs& we = static_cast<const CEGUI::WindowEventArgs&>(e);
                commonData.guiState.ambientOcclusion.enabled = static_cast<CEGUI::ToggleButton*>(we.window)->isSelected();
            });

        auto button_browse_bump_map = static_cast<::CEGUI::PushButton*>(guiRootWindow->getChild("MaterialParamsWindow/BumpWindow/Button"));

        button_browse_bump_map->subscribeEvent(::CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&ApplicationHandler::onEventForBumpTextureBrowse, this));

        static_cast<CEGUI::DefaultWindow*>(guiRootWindow->getChild("MaterialParamsWindow/BumpWindow/ImageButton"))->subscribeEvent(::CEGUI::Window::EventMouseClick, CEGUI::Event::Subscriber(&ApplicationHandler::onEventForBumpTextureBrowse, this));
        static_cast<CEGUI::Editbox*>(guiRootWindow->getChild("MaterialParamsWindow/BumpWindow/Editbox"))->subscribeEvent(::CEGUI::Editbox::EventTextAccepted, CEGUI::Event::Subscriber(&ApplicationHandler::onEventForBumpTextureBrowse_EditBox, this));

        guiManager.registerSliderEvent("MaterialParamsWindow/BumpWindow/Spinner", CommondData::sliderBumpHeightRange, 1.0f,
            [guiRootWindow, this](const ::CEGUI::EventArgs&)
            {
                auto height = static_cast<::CEGUI::Slider*>(guiRootWindow->getChild("MaterialParamsWindow/BumpWindow/Spinner"))->getCurrentValue();
                guiRootWindow->getChild("MaterialParamsWindow/BumpWindow/LabelPercent")->setText(GUIManager::toStringFloat(height, 2));

                commonData.guiState.bumpMapping.height = height; 
                commonData.derivMapGeneration.HeightFactorChanged = true;
                commonData.derivMapGeneration.TimePointLastHeightFactorChange = Clock::now();
            });

        auto bump_enabled = static_cast<::CEGUI::ToggleButton*>(guiRootWindow->getChild("MaterialParamsWindow/BumpWindow/Checkbox"));
        bump_enabled->subscribeEvent(CEGUI::ToggleButton::EventSelectStateChanged,
            [guiRootWindow, this](const ::CEGUI::EventArgs& e)
            {
                const ::CEGUI::WindowEventArgs& we = static_cast<const CEGUI::WindowEventArgs&>(e);
                commonData.guiState.bumpMapping.enabled = static_cast<CEGUI::ToggleButton*>(we.window)->isSelected();
            });

        initializeDropdown();
        initializeTooltip();

        // Setting up the texture preview window
        std::array<::CEGUI::PushButton*, 4> texturePreviewIcon = 
        {
            static_cast<::CEGUI::PushButton*>(guiRootWindow->getChild("TextureViewWindow/Texture0Window/Texture")),
            static_cast<::CEGUI::PushButton*>(guiRootWindow->getChild("TextureViewWindow/Texture1Window/Texture")),
            static_cast<::CEGUI::PushButton*>(guiRootWindow->getChild("TextureViewWindow/Texture2Window/Texture")),
            static_cast<::CEGUI::PushButton*>(guiRootWindow->getChild("TextureViewWindow/Texture3Window/Texture"))
        };

        for (const auto& texture : texturePreviewIcon)
            texture->subscribeEvent(::CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&ApplicationHandler::onEventForTextureBrowse, this));

        // Setting up the master windows & their default opacity
        auto* window_material = static_cast<::CEGUI::FrameWindow*>(guiRootWindow->getChild("MaterialParamsWindow"));
        window_material->subscribeEvent(CEGUI::FrameWindow::EventCloseClicked, 
            [guiRootWindow](const CEGUI::EventArgs&) 
            {
                static_cast<::CEGUI::FrameWindow*>(
                    guiRootWindow->getChild("MaterialParamsWindow"))
                    ->setVisible(false);
            });

        guiManager.setOpacity("MaterialParamsWindow", CommondData::defaultOpacity);

        auto* window_light = static_cast<::CEGUI::FrameWindow*>(guiRootWindow->getChild("LightParamsWindow"));
        window_light->subscribeEvent(::CEGUI::FrameWindow::EventCloseClicked,
            [guiRootWindow, this](const CEGUI::EventArgs&)
            {
                static_cast<CEGUI::FrameWindow*>(guiRootWindow->getChild("LightParamsWindow"))->setVisible(false);
            });

        guiManager.setOpacity("LightParamsWindow", CommondData::defaultOpacity);

        auto* window_texture = static_cast<::CEGUI::FrameWindow*>(guiRootWindow->getChild("TextureViewWindow"));
        window_texture->subscribeEvent(CEGUI::FrameWindow::EventCloseClicked, 
            [guiRootWindow, this](const CEGUI::EventArgs&)
            {
                static_cast<::CEGUI::FrameWindow*>(guiRootWindow->getChild("TextureViewWindow"))->setVisible(false);
            });

        guiManager.setOpacity("TextureViewWindow", CommondData::defaultOpacity);

        setGUIForConstantIoR();

	};

	auto setupScene = [&]()
	{
        // there may be something needed by stuff that will be displayed on screen like a mesh
	};

	setupGUI();
	setupScene();
}

/*
    Event definitions for injecting externally into CEGUI event handling system under certain circumstances
*/

void ApplicationHandler::onEventForAOTextureBrowse(const CEGUI::EventArgs& event)
{
    const auto fileDialog = guiManager.openFileDialog(CommondData::imageFileDialogTitle.data(), commonData.imageFileDialogFilters);

    if (fileDialog.first)
    {
        auto guiBox = static_cast<CEGUI::Editbox*>(guiManager.getRootWindow()->getChild("MaterialParamsWindow/AOWindow/Editbox"));

        //loadTextureSlot_CPUTex(ETEXTURE_SLOT::TEXTURE_AO, loadCPUTexture(fileDialog.second));   // load texture -> use IrrlichtBaW loaders! TODO TODO TODO

        guiBox->setText(fileDialog.second);
        /*updateTooltip(
            "MaterialParamsWindow/AOWindow/ImageButton",
            ext::cegui::ssprintf("%s (%ux%u)\nLeft-click to select a new texture.",
                p.second.c_str(), cputexture->getSize()[0], cputexture->getSize()[1])
                .c_str());*/
    }
}

void ApplicationHandler::onEventForAOTextureBrowse_EditBox(const CEGUI::EventArgs& event)
{
    auto guiBox = static_cast<CEGUI::Editbox*>(guiManager.getRootWindow()->getChild("MaterialParamsWindow/AOWindow/Editbox"));

    if (guiManager.doesFileExists(guiBox->getText().c_str()))
    {
        //loadTextureSlot_CPUTex(ETEXTURE_SLOT::TEXTURE_AO, loadCPUTexture(box->getText()));  // load texture -> use IrrlichtBaW loaders! TODO TODO TODO

        /*updateTooltip(
            "MaterialParamsWindow/AOWindow/ImageButton",
            irr::ext::cegui::ssprintf("%s (%ux%u)\nLeft-click to select a new texture.",
                box->getText().c_str(), cputexture->getSize()[0], cputexture->getSize()[1])
                .c_str());*/
    }
    else 
    {
        std::string errorMessage;
        errorMessage += std::string(guiBox->getText().c_str()) + ": The file couldn't be opened.";
        guiManager.replace(errorMessage, "\\", "\\\\");
        guiManager.showErrorMessage("Error", errorMessage.c_str());
    }
}

void ApplicationHandler::onEventForBumpTextureBrowse(const CEGUI::EventArgs& event)
{
    const auto fileDialog = guiManager.openFileDialog(CommondData::imageFileDialogTitle.data(), commonData.imageFileDialogFilters);

    if (fileDialog.first) 
    {
        auto guiBox = static_cast<CEGUI::Editbox*>(guiManager.getRootWindow()->getChild("MaterialParamsWindow/BumpWindow/Editbox"));

        // loadTextureSlot_CPUTex(ETEXTURE_SLOT::TEXTURE_BUMP, loadCPUTexture(p.second));       // load texture -> use IrrlichtBaW loaders! TODO TODO TODO

        guiBox->setText(fileDialog.second);
        /*updateTooltip(
            "MaterialParamsWindow/BumpWindow/ImageButton",
            ext::cegui::ssprintf("%s (%ux%u)\nLeft-click to select a new texture.",
                p.second.c_str(), cputexture->getSize()[0], cputexture->getSize()[1])
                .c_str());*/
    }
}

void ApplicationHandler::onEventForBumpTextureBrowse_EditBox(const CEGUI::EventArgs& event)
{
    auto guiBox = static_cast<CEGUI::Editbox*>(guiManager.getRootWindow()->getChild("MaterialParamsWindow/BumpWindow/Editbox"));

    if (guiManager.doesFileExists(guiBox->getText().c_str())) 
    {
        // loadTextureSlot_CPUTex(ETEXTURE_SLOT::TEXTURE_BUMP, loadCPUTexture(box->getText()));     // load texture -> use IrrlichtBaW loaders! TODO TODO TODO

        /*updateTooltip(
            "MaterialParamsWindow/BumpWindow/ImageButton",
            ext::cegui::ssprintf("%s (%ux%u)\nLeft-click to select a new texture.",
                box->getText().c_str(), cputexture->getSize()[0], cputexture->getSize()[1])
                .c_str());*/
    }
    else 
    {
        std::string errorMessage;
        errorMessage += std::string(guiBox->getText().c_str()) + ": The file couldn't be opened.";
        guiManager.replace(errorMessage, "\\", "\\\\");
        guiManager.showErrorMessage("Error", errorMessage.c_str());
    }
}

void ApplicationHandler::onEventForTextureBrowse(const CEGUI::EventArgs& event)
{
    const CEGUI::WindowEventArgs& windowEventArguments = static_cast<const CEGUI::WindowEventArgs&>(event);
    const auto parent = static_cast<CEGUI::PushButton*>(windowEventArguments.window)->getParent()->getName();
    const auto fileDialog = guiManager.openFileDialog(CommondData::imageFileDialogTitle.data(), commonData.imageFileDialogFilters);

    const auto path_label = GUIManager::ssprintf("TextureViewWindow/%s/LabelWindow/Label", parent.c_str());
    const auto path_texture = GUIManager::ssprintf("TextureViewWindow/%s/Texture", parent.c_str());

    if (fileDialog.first) 
    {
        auto guiBox = static_cast<CEGUI::Editbox*>(guiManager.getRootWindow()->getChild(path_label));
        const auto v = guiManager.split(fileDialog.second, '\\');

        /*

        ETEXTURE_SLOT type;
        if (parent == "Texture0Window")
            type = ETEXTURE_SLOT::TEXTURE_SLOT_1;
        else if (parent == "Texture1Window")
            type = ETEXTURE_SLOT::TEXTURE_SLOT_2;
        else if (parent == "Texture2Window")
            type = ETEXTURE_SLOT::TEXTURE_SLOT_3;
        else if (parent == "Texture3Window")
            type = ETEXTURE_SLOT::TEXTURE_SLOT_4;

        loadTextureSlot_CPUTex(type, loadCPUTexture(p.second)); // load texture -> use IrrlichtBaW loaders! TODO TODO TODO

        */

        guiBox->setText(v[v.size() - 1]);
        /*updateTooltip(
            path_texture.c_str(),
            ext::cegui::ssprintf("%s (%ux%u)\nLeft-click to select a new texture.",
                p.second.c_str(), cputexture->getSize()[0], cputexture->getSize()[1])
                .c_str());*/
    }
}

void ApplicationHandler::onEventForMeshBrowse(const CEGUI::EventArgs& event)
{
    const auto fileDialog = guiManager.openFileDialog(CommondData::meshFileDialogTitle.data(), commonData.meshFileDialogFilters);

    /* TODO after dealing with 2D

    if (fileDialog.first)
        loadMeshAndReplaceTextures(p.second);
    */
}

/*
    Certain extra settings and gui stuff initialization 
*/

void ApplicationHandler::initializeDropdown()
{
    static const std::vector<const char*> drop_ID = { "Constant", "Texture 0", "Texture 1", "Texture 2", "Texture 3" };
    const auto default_halignment = ::CEGUI::HA_RIGHT;
    const auto default_width = ::CEGUI::UDim(0.5f, 0.0f);
    const auto default_position = ::CEGUI::UVector2(::CEGUI::UDim(0.0f, 0.0f), ::CEGUI::UDim(0.125f, 0.0f));

    auto guiRootWindow = guiManager.getRootWindow();

    auto* albedo_drop = guiManager.createDropDownList("MaterialParamsWindow/AlbedoDropDownList", "DropDown_Albedo", drop_ID,
        [guiRootWindow, this](const ::CEGUI::EventArgs&)
        {
            auto* list = static_cast<::CEGUI::Combobox*>(guiRootWindow->getChild("MaterialParamsWindow/AlbedoDropDownList/DropDown_Albedo"));
            list->setProperty("NormalEditTextColour", guiManager.WhiteProperty);
            guiRootWindow->getChild("MaterialParamsWindow/AlbedoWindow")->setDisabled(list->getSelectedItem()->getText() != "Constant");

            commonData.guiState.albedo.sourceDropdown = guiManager.getDropdownState(CommondData::DROPDOWN_ALBEDO_NAME.data());
            //updateMaterial();   TODO
        });

    albedo_drop->setHorizontalAlignment(default_halignment);
    albedo_drop->setWidth(default_width);
    albedo_drop->setPosition(default_position);

    auto* roughness_drop = guiManager.createDropDownList("MaterialParamsWindow/RoughnessDropDownList", "DropDown_Roughness", drop_ID, 
        [guiRootWindow, this](const ::CEGUI::EventArgs&)
        {
            auto* list = static_cast<CEGUI::Combobox*>(guiRootWindow->getChild("MaterialParamsWindow/RoughnessDropDownList/DropDown_Roughness"));
            list->setProperty("NormalEditTextColour", guiManager.WhiteProperty);

            guiRootWindow->getChild("MaterialParamsWindow/RoughnessWindow")->setDisabled(list->getSelectedItem()->getText() != "Constant");
            
            commonData.guiState.roughness.sourceDropdown = guiManager.getDropdownState(CommondData::DROPDOWN_ROUGHNESS_NAME.data());
            //updateMaterial();     TODO
        });

    roughness_drop->setHorizontalAlignment(default_halignment);
    roughness_drop->setWidth(default_width);
    roughness_drop->setPosition(default_position);

    auto* ri_drop = guiManager.createDropDownList("MaterialParamsWindow/RIDropDownList", "DropDown_RI", drop_ID,
        [guiRootWindow, this](const ::CEGUI::EventArgs&)
        {
            auto* list = static_cast<::CEGUI::Combobox*>(guiRootWindow->getChild("MaterialParamsWindow/RIDropDownList/DropDown_RI"));
            list->setProperty("NormalEditTextColour", guiManager.WhiteProperty);

            bool realRIComesFromTexture = list->getSelectedItem()->getText() != "Constant";
            guiRootWindow->getChild("MaterialParamsWindow/RefractionIndexWindow1")->setDisabled(realRIComesFromTexture);
            guiRootWindow->getChild("MaterialParamsWindow/RefractionIndexWindow2")->setDisabled(realRIComesFromTexture);
            guiRootWindow->getChild("MaterialParamsWindow/RefractionIndexWindow3")->setDisabled(realRIComesFromTexture);

            if (realRIComesFromTexture)
                resetGUIAfterConstantIoR();
            else
                setGUIForConstantIoR();

            commonData.guiState.refractionIndex.sourceDropdown = guiManager.getDropdownState(CommondData::DROPDOWN_RI_NAME.data());
            //updateMaterial();     TODO
        });

    ri_drop->setHorizontalAlignment(default_halignment);
    ri_drop->setWidth(default_width);
    ri_drop->setPosition(default_position);

    auto* metallic_drop = guiManager.createDropDownList("MaterialParamsWindow/MetallicDropDownList", "DropDown_Metallic", drop_ID,
        [guiRootWindow, this](const ::CEGUI::EventArgs&)
        {
            auto guiRootWindow = guiManager.getRootWindow();

            auto* list = static_cast<::CEGUI::Combobox*>(guiRootWindow->getChild("MaterialParamsWindow/MetallicDropDownList/DropDown_Metallic"));
            list->setProperty("NormalEditTextColour", guiManager.WhiteProperty);

            guiRootWindow->getChild("MaterialParamsWindow/MetallicWindow")->setDisabled(list->getSelectedItem()->getText() != "Constant");

            commonData.guiState.metallic.sourceDropdown = guiManager.getDropdownState(CommondData::DROPDOWN_METALLIC_NAME.data());
            //updateMaterial();     TODO
        });

    metallic_drop->setHorizontalAlignment(default_halignment);
    metallic_drop->setWidth(default_width);
    metallic_drop->setPosition(default_position);
}

void ApplicationHandler::initializeTooltip()
{
    auto guiRootWindow = guiManager.getRootWindow();

    static_cast<CEGUI::DefaultWindow*>(
        guiRootWindow->getChild("MaterialParamsWindow/BumpWindow/ImageButton"))
        ->setTooltipText("Left-click to select a bump-mapping texture.");
    static_cast<CEGUI::DefaultWindow*>(
        guiRootWindow->getChild("MaterialParamsWindow/AOWindow/ImageButton"))
        ->setTooltipText("Left-click to select an AO texture.");
    static_cast<CEGUI::DefaultWindow*>(
        guiRootWindow->getChild("TextureViewWindow/Texture0Window/Texture"))
        ->setTooltipText("Left-click to select a new texture.");
    static_cast<CEGUI::DefaultWindow*>(
        guiRootWindow->getChild("TextureViewWindow/Texture1Window/Texture"))
        ->setTooltipText("Left-click to select a new texture.");
    static_cast<CEGUI::DefaultWindow*>(
        guiRootWindow->getChild("TextureViewWindow/Texture2Window/Texture"))
        ->setTooltipText("Left-click to select a new texture.");
    static_cast<CEGUI::DefaultWindow*>(
        guiRootWindow->getChild("TextureViewWindow/Texture3Window/Texture"))
        ->setTooltipText("Left-click to select a new texture.");
}

void ApplicationHandler::setGUIForConstantIoR()
{
    auto guiRootWindow = guiManager.getRootWindow();

    auto* list = static_cast<::CEGUI::Combobox*>(guiRootWindow->getChild(
        "MaterialParamsWindow/MetallicDropDownList/DropDown_Metallic"));

    auto selection_Constant = list->getListboxItemFromIndex(0u);
    auto selection_current = list->getSelectedItem();
    if (selection_current != selection_Constant) // set metallic-source dropdown state to Constant
    {
        list->setItemSelectState(selection_Constant, true);
        list->setItemSelectState(selection_current, false);
    }
    auto slider = static_cast<::CEGUI::Slider*>(guiRootWindow->getChild("MaterialParamsWindow/MetallicWindow/Slider"));
    slider->setCurrentValue(0.f); // set slider of constant metallic value to 0
    guiRootWindow->getChild("MaterialParamsWindow/MetallicWindow/LabelPercent")->setText("N/A"); // display N/A as metallic constant value

    // appropriately set GUIState "cache"
    commonData.guiState.metallic.sourceDropdown = EDS_CONSTANT;
    commonData.guiState.metallic.constValue = 0.f;

    // disable metallic options in GUI
    guiRootWindow->getChild("MaterialParamsWindow/MetallicWindow")->setDisabled(true);
    guiRootWindow->getChild("MaterialParamsWindow/MetallicDropDownList")->setDisabled(true);
}

void ApplicationHandler::resetGUIAfterConstantIoR()
{
    auto guiRootWindow = guiManager.getRootWindow();

    guiRootWindow->getChild("MaterialParamsWindow/MetallicWindow/LabelPercent")->setText("0.00");

    // re-enable metallic options
    guiRootWindow->getChild("MaterialParamsWindow/MetallicWindow")->setDisabled(false);
    guiRootWindow->getChild("MaterialParamsWindow/MetallicDropDownList")->setDisabled(false);
}

void ApplicationHandler::setLightPosition(const irr::core::vector3df& _lightPos)
{
    using namespace std::literals::string_literals;

    auto guiRootWindow = guiManager.getRootWindow();
    const char* xyz[]{ "X", "Y", "Z" };
    uint32_t i = 0u;
    for (const char* c : xyz)
    {
        auto lightCoord = static_cast<::CEGUI::Spinner*>(guiRootWindow->getChild("LightParamsWindow/PositionWindow/Light"s + c));
        lightCoord->setCurrentValue((&_lightPos.X)[i++]);
    }

    commonData.guiState.light.constantPosition = _lightPos;
}