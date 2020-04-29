#include "GUIManager.hpp"

using namespace irr;
using namespace asset;
using namespace CEGUI;

GUIManager::GUIManager(irr::core::smart_refctd_ptr<irr::IrrlichtDevice> _device)
    :   device(_device), driver(device->getVideoDriver())
{
    init();
}

std::pair<bool, std::string> GUIManager::openFileDialog(const char* title, const std::vector<std::string>& filters)
{
    device->getCursorControl()->setVisible(true);

    auto getResult = [&]()
    {
        auto buffer = pfd::open_file(title, ".", filters).result();
        if (!buffer.empty())
            return std::make_pair(true, buffer[0]);
        return std::make_pair(false, std::string());
    };

    auto result = getResult();
    device->getCursorControl()->setVisible(false);
    return result;
}

void GUIManager::init()
{
    guiRenderer = &CEGUI::OpenGL3Renderer::bootstrapSystem();
    guiRenderer->setDisplaySize(Sizef(float(driver->getScreenSize().Width), float(driver->getScreenSize().Height)));

    auto guiResourceProvider = reinterpret_cast<CEGUI::DefaultResourceProvider*>(CEGUI::System::getSingleton().getResourceProvider());

    for (auto& resourceProvider : resourceProviderAliases)
        guiResourceProvider->setResourceGroupDirectory(resourceProvider.alias.data(), resourceProvider.path.data());

    CEGUI::ImageManager::setImagesetDefaultResourceGroup(resourceProviderAliases[ERPA_IMAGESETS].alias.data());
    CEGUI::Font::setDefaultResourceGroup(resourceProviderAliases[ERPA_FONTS].alias.data());
    CEGUI::Scheme::setDefaultResourceGroup(resourceProviderAliases[ERPA_SCHEMES].alias.data());
    CEGUI::WidgetLookManager::setDefaultResourceGroup(resourceProviderAliases[ERPA_LOOKNFEELS].alias.data());
    CEGUI::WindowManager::setDefaultResourceGroup(resourceProviderAliases[ERPA_LAYOUTS].alias.data());
    CEGUI::ScriptModule::setDefaultResourceGroup(resourceProviderAliases[ERPA_LUA_SCRIPTS].alias.data());

    CEGUI::XMLParser* parser = CEGUI::System::getSingleton().getXMLParser();
    if (parser->isPropertyPresent("SchemaDefaultResourceGroup"))
        parser->setProperty("SchemaDefaultResourceGroup", resourceProviderAliases[ERPA_XML_SCHEMES].alias.data());

    SchemeManager::getSingleton().createFromFile("Alfisko.scheme", resourceProviderAliases[ERPA_SCHEMES].alias.data());
    SchemeManager::getSingleton().createFromFile("AlfiskoCommonDialogs.scheme", resourceProviderAliases[ERPA_SCHEMES].alias.data());
    FontManager::getSingleton().createFromFile("Cousine-Regular.font");

    auto& guiContext = System::getSingleton().getDefaultGUIContext();
    guiContext.setDefaultFont("Cousine-Regular");
    guiContext.getMouseCursor().setDefaultImage("Alfisko/MouseArrow");
    guiContext.setDefaultTooltipType("Alfisko/Tooltip");

    System::getSingleton().notifyDisplaySizeChanged(Sizef(float(driver->getScreenSize().Width), float(driver->getScreenSize().Height)));

    initialiseCEGUICommonDialogs();
}

void GUIManager::destroy()
{
    CEGUI::System::destroy();
    CEGUI::OpenGL3Renderer::destroy(*guiRenderer);
}

void GUIManager::render()
{
    // STATE wtf
    //setOpenGLClip();
    CEGUI::System::getSingleton().renderAllGUIContexts();
    //resetOpenGLClip();
}

E_DROPDOWN_STATE GUIManager::getDropdownState(const char* _dropdownName) const
{
    auto* list = static_cast<::CEGUI::Combobox*>(guiRootWindow->getChild(_dropdownName));

    auto mapStrToEnum = [](const std::string& _str)
    {
        const char* texture = "Texture";
        if (_str.compare(0, strlen(texture), texture) == 0)
            return static_cast<E_DROPDOWN_STATE>(EDS_TEX0 + _str[strlen(texture) + 1] - '0');
        else return EDS_CONSTANT;
    };

    return mapStrToEnum(list->getSelectedItem()->getText());
}

bool GUIManager::OnEvent(const SEvent& event)
{
    CEGUI::GUIContext& guiContext = CEGUI::System::getSingleton().getDefaultGUIContext();

    switch (event.EventType) 
    {
        case irr::EET_KEY_INPUT_EVENT:
        {
            if (event.KeyInput.PressedDown)
                return guiContext.injectKeyDown(toCEGUIKey(event.KeyInput.Key)) || guiContext.injectChar(event.KeyInput.Char);
            else
                return guiContext.injectKeyUp(toCEGUIKey(event.KeyInput.Key));
        } break;

        case irr::EET_MOUSE_INPUT_EVENT:
        {
            return guiContext.injectMousePosition(event.MouseInput.X, event.MouseInput.Y) ||
            [&event,&guiContext]
            {
                switch (event.MouseInput.Event)
                {
                    case irr::EMOUSE_INPUT_EVENT::EMIE_LMOUSE_PRESSED_DOWN:
                    {
                        return guiContext.injectMouseButtonDown(CEGUI::MouseButton::LeftButton);
                    } break;

                    case irr::EMOUSE_INPUT_EVENT::EMIE_LMOUSE_LEFT_UP:
                    {
                        return guiContext.injectMouseButtonUp(CEGUI::MouseButton::LeftButton);
                    } break;

                    case irr::EMOUSE_INPUT_EVENT::EMIE_RMOUSE_PRESSED_DOWN:
                    {
                        return guiContext.injectMouseButtonDown(CEGUI::MouseButton::RightButton);
                    } break;

                    case irr::EMOUSE_INPUT_EVENT::EMIE_RMOUSE_LEFT_UP:
                    {
                        return guiContext.injectMouseButtonUp(CEGUI::MouseButton::RightButton);
                    } break;

                    case irr::EMOUSE_INPUT_EVENT::EMIE_MMOUSE_PRESSED_DOWN:
                    {
                        return guiContext.injectMouseButtonDown(CEGUI::MouseButton::MiddleButton);
                    } break;

                    case irr::EMOUSE_INPUT_EVENT::EMIE_MMOUSE_LEFT_UP:
                    {
                        return guiContext.injectMouseButtonUp(CEGUI::MouseButton::MiddleButton);
                    } break;

                    case irr::EMOUSE_INPUT_EVENT::EMIE_LMOUSE_DOUBLE_CLICK:
                    {
                        return guiContext.injectMouseButtonDoubleClick(CEGUI::MouseButton::LeftButton);
                    } break;

                    case irr::EMOUSE_INPUT_EVENT::EMIE_RMOUSE_DOUBLE_CLICK:
                    {
                        return guiContext.injectMouseButtonDoubleClick(CEGUI::MouseButton::RightButton);
                    } break;

                    case irr::EMOUSE_INPUT_EVENT::EMIE_MMOUSE_DOUBLE_CLICK:
                    {
                        return guiContext.injectMouseButtonDoubleClick(CEGUI::MouseButton::MiddleButton);
                    } break;

                    case irr::EMOUSE_INPUT_EVENT::EMIE_LMOUSE_TRIPLE_CLICK:
                    {
                        return guiContext.injectMouseButtonTripleClick(CEGUI::MouseButton::LeftButton);
                    } break;

                    case irr::EMOUSE_INPUT_EVENT::EMIE_RMOUSE_TRIPLE_CLICK:
                    {
                        return guiContext.injectMouseButtonTripleClick(CEGUI::MouseButton::RightButton);
                    } break;

                    case irr::EMOUSE_INPUT_EVENT::EMIE_MMOUSE_TRIPLE_CLICK:
                    {
                        return guiContext.injectMouseButtonTripleClick(CEGUI::MouseButton::MiddleButton);
                    } break;

                    default: return false;
                }
            }();
        }
        break;

        default: return false;
    }
    return false;
}

void GUIManager::createRootWindowFromLayout(const std::string& layout)
{
    guiRootWindow = WindowManager::getSingleton().loadLayoutFromString(layout);
    guiRootWindow->setMousePassThroughEnabled(true);
    System::getSingleton().getDefaultGUIContext().setRootWindow(guiRootWindow);
}

CEGUI::ColourPicker* GUIManager::createColourPicker(bool alternativeLayout, const char* parent, const char* title, const char* name, const TOnColorPicked& onColorPicked)
{
    assert(parent && name);
    static const auto defaultColor = ::CEGUI::Colour(1.0f, 1.0f, 1.0f, 1.0f);

    auto guiLayout = WindowManager::getSingleton().loadLayoutFromFile(alternativeLayout ? "CPAlternativeLayout.layout" : "CPMainLayout.layout");

    if (guiRootWindow && guiLayout) 
    {
        auto window = static_cast<DefaultWindow*>(guiRootWindow->getChild(parent));
        window->addChild(guiLayout);
        // window->setSize(USize(UDim(1.0f, 0.0f), UDim(1.0f, 0.0f)));

        if (title) 
        {
            auto bar = static_cast<DefaultWindow*>(guiLayout->getChild("TitleLabel"));
            bar->setText(title);
        }

        static std::array<char, 3> compomentMap{ 'R', 'G', 'B' };

        for (const auto& compoment : compomentMap)
        {
            /*
                CEGUI supports handling specified functions as an event is trigerred.
            */

            auto onEventForSliders = [guiLayout](void)
            {
                auto sliderR = static_cast<Slider*>(guiLayout->getChild("SliderR"))->getCurrentValue();
                auto sliderG = static_cast<Slider*>(guiLayout->getChild("SliderG"))->getCurrentValue();
                auto sliderB = static_cast<Slider*>(guiLayout->getChild("SliderB"))->getCurrentValue();

                const Colour color(sliderR / 255.0f, sliderG / 255.0f, sliderB / 255.0f, 1.0f);

                static_cast<ColourPicker*>(guiLayout->getChild("ColorPickerContainer/MyPicker"))->setColour(color);

                if (guiLayout->isChild("LabelShared")) 
                {
                    Window* label_shared = guiLayout->getChild("LabelShared");

                    std::ostringstream ss;
                    ss << '(' << "[colour='FFFFFFFF']"
                        << "[colour='FFFF0000']" << std::setprecision(3) << sliderR << ", "
                        << "[colour='FF00FF00']" << std::setprecision(3) << sliderG << ", "
                        << "[colour='FF0000FF']" << std::setprecision(3) << sliderB
                        << "[colour='FFFFFFFF']" << ')';

                    static_cast<DefaultWindow*>(label_shared)->setText(ss.str());
                }
                else 
                {
                    auto* labelR = guiLayout->getChild("SliderR/Label");
                    auto* labelG = guiLayout->getChild("SliderG/Label");
                    auto* labelB = guiLayout->getChild("SliderB/Label");

                    static_cast<DefaultWindow*>(labelR)->setText(
                        String("[colour='FFFF0000']") + toStringFloat(sliderR, 0));
                    static_cast<DefaultWindow*>(labelG)->setText(
                        String("[colour='FF00FF00']") + toStringFloat(sliderG, 0));
                    static_cast<DefaultWindow*>(labelB)->setText(
                        String("[colour='FF0000FF']") + toStringFloat(sliderB, 0));
                }
            };

            static_cast<Slider*>(guiLayout->getChild(std::string("Slider") + compoment))->subscribeEvent(Slider::EventValueChanged, onEventForSliders);
        }

        auto cpicker_window = static_cast<DefaultWindow*>(guiLayout->getChild("ColorPickerContainer"));
        ColourPicker* picker = static_cast<ColourPicker*>(WindowManager::getSingleton().createWindow("Alfisko/CPColourPicker", "MyPicker"));
        picker->setTooltipText("Left-click to open the color picker.");

        cpicker_window->addChild(picker);

        auto onEventForAcceptColors = [guiLayout, picker, onColorPicked](void)
        {
            const auto color = picker->getColour();
            onColorPicked(color);

            static_cast<Slider*>(guiLayout->getChild("SliderR"))->setCurrentValue(color.getRed() * 255.0f);
            static_cast<Slider*>(guiLayout->getChild("SliderG"))->setCurrentValue(color.getGreen() * 255.0f);
            static_cast<Slider*>(guiLayout->getChild("SliderB"))->setCurrentValue(color.getBlue() * 255.0f);
        };

        picker->subscribeEvent(ColourPicker::EventAcceptedColour, onEventForAcceptColors);

        auto onEventForOpeningPicker = [guiLayout](void)
        {
            auto sliderR = static_cast<Slider*>(guiLayout->getChild("SliderR"))->getCurrentValue();
            auto sliderG = static_cast<Slider*>(guiLayout->getChild("SliderG"))->getCurrentValue();
            auto sliderB = static_cast<Slider*>(guiLayout->getChild("SliderB"))->getCurrentValue();
        };

        picker->subscribeEvent(ColourPicker::EventOpenedPicker, onEventForOpeningPicker);

        picker->setInheritsAlpha(false);
        picker->setPosition(UVector2(UDim(0.0f, 0.0f), UDim(0.0f, 0.0f)));
        picker->setSize(USize(UDim(1.0f, 0.0f), UDim(1.0f, 0.0f)));
        picker->setColour(defaultColor);

        auto pickerWindow = static_cast<::CEGUI::ColourPicker*>(guiLayout);
        guicolorPickerWindows[name] = pickerWindow;

        return pickerWindow;
    }

    return nullptr;
}

CEGUI::Window* GUIManager::createDropDownList(const char* name, const char* title, const std::vector<const char*>& list, const TEventHandler& eventSelectionAccepted)
{
    assert(name);

    if (guiRootWindow && name) 
    {
        auto* box = static_cast<Combobox*>(WindowManager::getSingleton().createWindow("Alfisko/DropDownMenu", title));
        box->setInheritsAlpha(false);
        static const Colour WHITE = Colour(1.0f, 1.0f, 1.0f, 1.0f);

        if (!ImageManager::getSingleton().isDefined("ItemHover"))
            ImageManager::getSingleton().addFromImageFile("ItemHover", "ItemHover.png");

        auto window = static_cast<DefaultWindow*>(guiRootWindow->getChild(name));
        window->addChild(box);

        box->getDropList()->subscribeEvent(ComboDropList::EventListSelectionAccepted, eventSelectionAccepted);

        ListboxTextItem* first = nullptr;
        bool first_chosen = false;

        for (const auto& e : list) 
        {
            auto item = new ListboxTextItem(e);
            // item->setSelectionColours(WHITE, WHITE, WHITE, WHITE);
            item->setTextColours(WHITE, WHITE, WHITE, WHITE);
            item->setSelectionBrushImage("ItemHover");

            box->addItem(item);

            if (!first_chosen) 
            {
                first = item;
                first_chosen = true;
            }
        }

        box->setAutoSizeListHeightToContent(true);
        box->setItemSelectState(first, true);
        box->setProperty("NormalEditTextColour", WhiteProperty);

        return box;
    }

    return nullptr;
}

void GUIManager::registerSliderEvent(const char* name, float max, float step, const TEventHandler& onAnEvent)
{
    if (name) 
    {
        auto slider = static_cast<::CEGUI::Slider*>(guiRootWindow->getChild(name));

        if (slider) 
        {
            slider->setCurrentValue(0.0f);
            slider->setMaxValue(max);
            slider->setClickStep(step);
            slider->subscribeEvent(CEGUI::Slider::EventValueChanged, onAnEvent);
        }
    }
}

void GUIManager::setOpacity(const char* name, float opacity)
{
    auto window = static_cast<CEGUI::FrameWindow*>(guiRootWindow->getChild(name));
    window->setAlpha(opacity);

    if (opacity > 0.0f) 
    {
        auto bar = static_cast<CEGUI::DefaultWindow*>(window->getChild("__auto_titlebar__"));
        bar->setInheritsAlpha(false);
        bar->setAlpha(1.0f);
    }
}

CEGUI::Key::Scan GUIManager::toCEGUIKey(const irr::EKEY_CODE& code)
{
    switch (code) 
    {
        case irr::EKEY_CODE::KEY_ESCAPE: return ::CEGUI::Key::Scan::Escape;
        case irr::EKEY_CODE::KEY_KEY_1: return ::CEGUI::Key::Scan::One;
        case irr::EKEY_CODE::KEY_KEY_2: return ::CEGUI::Key::Scan::Two;
        case irr::EKEY_CODE::KEY_KEY_3: return ::CEGUI::Key::Scan::Three;
        case irr::EKEY_CODE::KEY_KEY_4: return ::CEGUI::Key::Scan::Four;
        case irr::EKEY_CODE::KEY_KEY_5: return ::CEGUI::Key::Scan::Five;
        case irr::EKEY_CODE::KEY_KEY_6: return ::CEGUI::Key::Scan::Six;
        case irr::EKEY_CODE::KEY_KEY_7: return ::CEGUI::Key::Scan::Seven;
        case irr::EKEY_CODE::KEY_KEY_8: return ::CEGUI::Key::Scan::Eight;
        case irr::EKEY_CODE::KEY_KEY_9: return ::CEGUI::Key::Scan::Nine;
        case irr::EKEY_CODE::KEY_KEY_0: return ::CEGUI::Key::Scan::Zero;
        case irr::EKEY_CODE::KEY_MINUS: return ::CEGUI::Key::Scan::Minus;
        // case irr::EKEY_CODE::KEY_ : return ::CEGUI::Key::Scan::Equals;
        case irr::EKEY_CODE::KEY_BACK: return ::CEGUI::Key::Scan::Backspace;
        case irr::EKEY_CODE::KEY_TAB: return ::CEGUI::Key::Scan::Tab;
        case irr::EKEY_CODE::KEY_KEY_Q: return ::CEGUI::Key::Scan::Q;
        case irr::EKEY_CODE::KEY_KEY_W: return ::CEGUI::Key::Scan::W;
        case irr::EKEY_CODE::KEY_KEY_E: return ::CEGUI::Key::Scan::E;
        case irr::EKEY_CODE::KEY_KEY_R: return ::CEGUI::Key::Scan::R;
        case irr::EKEY_CODE::KEY_KEY_T: return ::CEGUI::Key::Scan::T;
        case irr::EKEY_CODE::KEY_KEY_I: return ::CEGUI::Key::Scan::I;
        case irr::EKEY_CODE::KEY_KEY_O: return ::CEGUI::Key::Scan::O;
        case irr::EKEY_CODE::KEY_KEY_P: return ::CEGUI::Key::Scan::P;
        case irr::EKEY_CODE::KEY_OEM_4: return ::CEGUI::Key::Scan::LeftBracket;
        case irr::EKEY_CODE::KEY_OEM_6: return ::CEGUI::Key::Scan::RightBracket;
        case irr::EKEY_CODE::KEY_RETURN: return ::CEGUI::Key::Scan::Return;
        case irr::EKEY_CODE::KEY_LCONTROL: return ::CEGUI::Key::Scan::LeftControl;
        case irr::EKEY_CODE::KEY_KEY_A: return ::CEGUI::Key::Scan::A;
        case irr::EKEY_CODE::KEY_KEY_S: return ::CEGUI::Key::Scan::S;
        case irr::EKEY_CODE::KEY_KEY_D: return ::CEGUI::Key::Scan::D;
        case irr::EKEY_CODE::KEY_KEY_F: return ::CEGUI::Key::Scan::F;
        case irr::EKEY_CODE::KEY_KEY_G: return ::CEGUI::Key::Scan::G;
        case irr::EKEY_CODE::KEY_KEY_H: return ::CEGUI::Key::Scan::H;
        case irr::EKEY_CODE::KEY_KEY_J: return ::CEGUI::Key::Scan::J;
        case irr::EKEY_CODE::KEY_KEY_K: return ::CEGUI::Key::Scan::K;
        case irr::EKEY_CODE::KEY_KEY_L: return ::CEGUI::Key::Scan::L;
        case irr::EKEY_CODE::KEY_OEM_1: return ::CEGUI::Key::Scan::Semicolon;
        case irr::EKEY_CODE::KEY_OEM_7: return ::CEGUI::Key::Scan::Apostrophe;
        case irr::EKEY_CODE::KEY_OEM_3: return ::CEGUI::Key::Scan::Grave;
        case irr::EKEY_CODE::KEY_LSHIFT: return ::CEGUI::Key::Scan::LeftShift;
        case irr::EKEY_CODE::KEY_OEM_5: return ::CEGUI::Key::Scan::Backslash;
        case irr::EKEY_CODE::KEY_KEY_Z: return ::CEGUI::Key::Scan::Z;
        case irr::EKEY_CODE::KEY_KEY_X: return ::CEGUI::Key::Scan::X;
        case irr::EKEY_CODE::KEY_KEY_C: return ::CEGUI::Key::Scan::C;
        case irr::EKEY_CODE::KEY_KEY_V: return ::CEGUI::Key::Scan::V;
        case irr::EKEY_CODE::KEY_KEY_B: return ::CEGUI::Key::Scan::B;
        case irr::EKEY_CODE::KEY_KEY_N: return ::CEGUI::Key::Scan::N;
        case irr::EKEY_CODE::KEY_KEY_M: return ::CEGUI::Key::Scan::M;
        case irr::EKEY_CODE::KEY_COMMA: return ::CEGUI::Key::Scan::Comma;
        case irr::EKEY_CODE::KEY_PERIOD: return ::CEGUI::Key::Scan::Period;
        case irr::EKEY_CODE::KEY_OEM_2: return ::CEGUI::Key::Scan::Slash;
        case irr::EKEY_CODE::KEY_RSHIFT: return ::CEGUI::Key::Scan::RightShift;
        case irr::EKEY_CODE::KEY_MULTIPLY: return ::CEGUI::Key::Scan::Multiply;
        case irr::EKEY_CODE::KEY_MENU: return ::CEGUI::Key::Scan::LeftAlt;
        case irr::EKEY_CODE::KEY_SPACE: return ::CEGUI::Key::Scan::Space;
        case irr::EKEY_CODE::KEY_CAPITAL: return ::CEGUI::Key::Scan::Capital;
        case irr::EKEY_CODE::KEY_F1: return ::CEGUI::Key::Scan::F1;
        case irr::EKEY_CODE::KEY_F2: return ::CEGUI::Key::Scan::F2;
        case irr::EKEY_CODE::KEY_F3: return ::CEGUI::Key::Scan::F3;
        case irr::EKEY_CODE::KEY_F4: return ::CEGUI::Key::Scan::F4;
        case irr::EKEY_CODE::KEY_F5: return ::CEGUI::Key::Scan::F5;
        case irr::EKEY_CODE::KEY_F6: return ::CEGUI::Key::Scan::F6;
        case irr::EKEY_CODE::KEY_F7: return ::CEGUI::Key::Scan::F7;
        case irr::EKEY_CODE::KEY_F8: return ::CEGUI::Key::Scan::F8;
        case irr::EKEY_CODE::KEY_F9: return ::CEGUI::Key::Scan::F9;
        case irr::EKEY_CODE::KEY_F10: return ::CEGUI::Key::Scan::F10;
        case irr::EKEY_CODE::KEY_NUMLOCK: return ::CEGUI::Key::Scan::NumLock;
        case irr::EKEY_CODE::KEY_SCROLL: return ::CEGUI::Key::Scan::ScrollLock;
        case irr::EKEY_CODE::KEY_NUMPAD7: return ::CEGUI::Key::Scan::Numpad7;
        case irr::EKEY_CODE::KEY_NUMPAD8: return ::CEGUI::Key::Scan::Numpad8;
        case irr::EKEY_CODE::KEY_NUMPAD9: return ::CEGUI::Key::Scan::Numpad9;
        case irr::EKEY_CODE::KEY_SUBTRACT: return ::CEGUI::Key::Scan::Subtract;
        case irr::EKEY_CODE::KEY_NUMPAD4: return ::CEGUI::Key::Scan::Numpad4;
        case irr::EKEY_CODE::KEY_NUMPAD5: return ::CEGUI::Key::Scan::Numpad5;
        case irr::EKEY_CODE::KEY_NUMPAD6: return ::CEGUI::Key::Scan::Numpad6;
        case irr::EKEY_CODE::KEY_ADD: return ::CEGUI::Key::Scan::Add;
        case irr::EKEY_CODE::KEY_NUMPAD1: return ::CEGUI::Key::Scan::Numpad1;
        case irr::EKEY_CODE::KEY_NUMPAD2: return ::CEGUI::Key::Scan::Numpad2;
        case irr::EKEY_CODE::KEY_NUMPAD3: return ::CEGUI::Key::Scan::Numpad3;
        case irr::EKEY_CODE::KEY_NUMPAD0: return ::CEGUI::Key::Scan::Numpad0;
        case irr::EKEY_CODE::KEY_DECIMAL: return ::CEGUI::Key::Scan::Decimal;
        case irr::EKEY_CODE::KEY_OEM_102: return ::CEGUI::Key::Scan::OEM_102;
        case irr::EKEY_CODE::KEY_F11: return ::CEGUI::Key::Scan::F11;
        case irr::EKEY_CODE::KEY_F12: return ::CEGUI::Key::Scan::F12;
        case irr::EKEY_CODE::KEY_F13: return ::CEGUI::Key::Scan::F13;
        case irr::EKEY_CODE::KEY_F14: return ::CEGUI::Key::Scan::F14;
        case irr::EKEY_CODE::KEY_F15: return ::CEGUI::Key::Scan::F15;
        case irr::EKEY_CODE::KEY_KANA: return ::CEGUI::Key::Scan::Kana;
        // case irr::EKEY_CODE::KEY_ : return ::CEGUI::Key::Scan::ABNT_C1;
        case irr::EKEY_CODE::KEY_CONVERT: return ::CEGUI::Key::Scan::Convert;
        case irr::EKEY_CODE::KEY_NONCONVERT: return ::CEGUI::Key::Scan::NoConvert;
        // case irr::EKEY_CODE::KEY_ : return ::CEGUI::Key::Scan::Yen;
        // case irr::EKEY_CODE::KEY_ : return ::CEGUI::Key::Scan::ABNT_C2;
        // case irr::EKEY_CODE::KEY_ : return ::CEGUI::Key::Scan::NumpadEquals;
        // case irr::EKEY_CODE::KEY_ : return ::CEGUI::Key::Scan::PrevTrack;
        // case irr::EKEY_CODE::KEY_ : return ::CEGUI::Key::Scan::At;
        // case irr::EKEY_CODE::KEY_ : return ::CEGUI::Key::Scan::Colon;
        // case irr::EKEY_CODE::KEY_ : return ::CEGUI::Key::Scan::Underline;
        case irr::EKEY_CODE::KEY_KANJI: return ::CEGUI::Key::Scan::Kanji;
            // case irr::EKEY_CODE::KEY_ : return ::CEGUI::Key::Scan::Stop;
        case irr::EKEY_CODE::KEY_OEM_AX: return ::CEGUI::Key::Scan::AX;
        // case irr::EKEY_CODE::KEY_ : return ::CEGUI::Key::Scan::Unlabeled;
        // case irr::EKEY_CODE::KEY_ : return ::CEGUI::Key::Scan::NextTrack;
        case irr::EKEY_CODE::KEY_RCONTROL: return ::CEGUI::Key::Scan::RightControl;
        // case irr::EKEY_CODE::KEY_ : return ::CEGUI::Key::Scan::Mute;
        // case irr::EKEY_CODE::KEY_ : return ::CEGUI::Key::Scan::Calculator;
        case irr::EKEY_CODE::KEY_PLAY: return ::CEGUI::Key::Scan::PlayPause;
        // case irr::EKEY_CODE::KEY_ : return ::CEGUI::Key::Scan::MediaStop;
        // case irr::EKEY_CODE::KEY_ : return ::CEGUI::Key::Scan::VolumeDown;
        // case irr::EKEY_CODE::KEY_ : return ::CEGUI::Key::Scan::VolumeUp;
        // case irr::EKEY_CODE::KEY_ : return ::CEGUI::Key::Scan::WebHome;
        case irr::EKEY_CODE::KEY_DIVIDE: return ::CEGUI::Key::Scan::Divide;
        // case irr::EKEY_CODE::KEY_ : return ::CEGUI::Key::Scan::SysRq;
        case irr::EKEY_CODE::KEY_HOME: return ::CEGUI::Key::Scan::Home;
        case irr::EKEY_CODE::KEY_UP: return ::CEGUI::Key::Scan::ArrowUp;
        case irr::EKEY_CODE::KEY_PRIOR: return ::CEGUI::Key::Scan::PageUp;
        case irr::EKEY_CODE::KEY_LEFT: return ::CEGUI::Key::Scan::ArrowLeft;
        case irr::EKEY_CODE::KEY_RIGHT: return ::CEGUI::Key::Scan::ArrowRight;
        case irr::EKEY_CODE::KEY_END: return ::CEGUI::Key::Scan::End;
        case irr::EKEY_CODE::KEY_DOWN: return ::CEGUI::Key::Scan::ArrowDown;
        case irr::EKEY_CODE::KEY_NEXT: return ::CEGUI::Key::Scan::PageDown;
        case irr::EKEY_CODE::KEY_INSERT: return ::CEGUI::Key::Scan::Insert;
        case irr::EKEY_CODE::KEY_DELETE: return ::CEGUI::Key::Scan::Delete;
        case irr::EKEY_CODE::KEY_LWIN: return ::CEGUI::Key::Scan::LeftWindows;
        case irr::EKEY_CODE::KEY_RWIN: return ::CEGUI::Key::Scan::RightWindows;
        case irr::EKEY_CODE::KEY_APPS: return ::CEGUI::Key::Scan::AppMenu;
        // case irr::EKEY_CODE::KEY_ : return ::CEGUI::Key::Scan::Power;
        // case irr::EKEY_CODE::KEY_ : return ::CEGUI::Key::Scan::Sleep;
        // case irr::EKEY_CODE::KEY_ : return ::CEGUI::Key::Scan::Wake;
        // case irr::EKEY_CODE::KEY_ : return ::CEGUI::Key::Scan::WebSearch;
        // case irr::EKEY_CODE::KEY_ : return ::CEGUI::Key::Scan::WebFavorites;
        // case irr::EKEY_CODE::KEY_ : return ::CEGUI::Key::Scan::WebRefresh;
        // case irr::EKEY_CODE::KEY_ : return ::CEGUI::Key::Scan::WebStop;
        // case irr::EKEY_CODE::KEY_ : return ::CEGUI::Key::Scan::WebForward;
        // case irr::EKEY_CODE::KEY_ : return ::CEGUI::Key::Scan::WebBack;
        // case irr::EKEY_CODE::KEY_ : return ::CEGUI::Key::Scan::MyComputer;
        // case irr::EKEY_CODE::KEY_ : return ::CEGUI::Key::Scan::Mail;
        // case irr::EKEY_CODE::KEY_ : return ::CEGUI::Key::Scan::MediaSelect;
        default: return ::CEGUI::Key::Scan::Unknown;
    }
}

template <typename T>
CEGUI::String GUIManager::toStringFloat(const T rvalue, const int precision)
{
    std::ostringstream out;
    out.precision(precision);
    out << std::fixed << rvalue;
    return ::CEGUI::String(out.str());
}

std::string GUIManager::readWindowLayout(const std::string& layoutPath)
{
    std::ifstream file(layoutPath);
    std::string str((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());
    return str;
}

// Might be replaced with IrrlichtBAW's file system API.
int GUIManager::doesFileExists(const char* file)
{
    #if defined(_WIN32)
        DWORD attribute = GetFileAttributes(file);
        return (attribute != INVALID_FILE_ATTRIBUTES && !(attribute & FILE_ATTRIBUTE_DIRECTORY));
    #elif defined(__linux__)
        struct stat s;
        return stat(file, &s) == 0;
    #endif
}

void GUIManager::replace(std::string& str, const std::string& from, const std::string& to)
{
    size_t start = 0;
    while ((start = str.find(from, start)) != std::string::npos) {
        str.replace(start, from.length(), to);
        start += to.length(); // Handles case where 'to' is a substring of 'from'
    }
}

void GUIManager::showErrorMessage(const char* title, const char* message)
{
    if (!guiRootWindow->isChild("MessageBoxRoot"))
    {
        CEGUI::Window* layout = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("MessageBox.layout");
        layout->setVisible(false);
        layout->setAlwaysOnTop(true);
        layout->setSize(CEGUI::USize(CEGUI::UDim(0.5, 0.0f), CEGUI::UDim(0.2f, 0.0f)));
        layout->setHorizontalAlignment(CEGUI::HA_CENTRE);
        layout->setVerticalAlignment(CEGUI::VA_CENTRE);

        static_cast<CEGUI::PushButton*>(layout->getChild("FrameWindow/ButtonWindow/Button"))->subscribeEvent(CEGUI::PushButton::EventClicked,
            [&](const CEGUI::EventArgs&)
            {
                guiRootWindow->getChild("MessageBoxRoot")->setVisible(false);
            });

        guiRootWindow->addChild(layout);
    }

    auto header = static_cast<CEGUI::DefaultWindow*>(guiRootWindow->getChild("MessageBoxRoot"));
    header->setVisible(true);
    header->activate();

    auto frame = static_cast<CEGUI::FrameWindow*>(header->getChild("FrameWindow"));
    frame->setText(title);
    static_cast<CEGUI::DefaultWindow*>(frame->getChild("Label"))->setText(message);
}

std::vector<std::string> GUIManager::split(const std::string& s, const char delimiter)
{
    std::vector<std::string> v;
    std::istringstream f(s);
    std::string r;
    while (std::getline(f, r, delimiter))
        v.push_back(r);

    return v;
}