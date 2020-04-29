#ifndef _GUI_MANAGER_HPP_INCLUDED_
#define _GUI_MANAGER_HPP_INCLUDED_

#include <irrlicht.h>
#include <CEGUI/CEGUI.h>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include "portable-file-dialogs/portable-file-dialogs.h"
#include <CEGUI/RendererModules/OpenGL/GL3Renderer.h>
#include <CEGUI/System.h>
#include <CEGUI/widgets/Slider.h>
#include <CEGUI/CommonDialogs/ColourPicker/ColourPicker.h>
#include <map>
#include <functional>

/*
    CEGUI uses an alias name of resources and paths to them.
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
    ResourceProviderAlias("schemes", "../../media/cegui_alfisko/schemes/"),
    ResourceProviderAlias("imagesets", "../../media/cegui_alfisko/imagesets/"),
    ResourceProviderAlias("fonts", "../../media/cegui_alfisko/fonts/"),
    ResourceProviderAlias("layouts", "../../media/cegui_alfisko/layouts/"),
    ResourceProviderAlias("looknfeels", "../../media/cegui_alfisko/looknfeel/"),
    ResourceProviderAlias("lua_scripts", "../../media/cegui_alfisko/lua_scripts/"),
    ResourceProviderAlias("schemas", "../../media/cegui_alfisko/xml_schemas/")
};

enum E_DROPDOWN_STATE
{
    EDS_CONSTANT,
    EDS_TEX0,
    EDS_TEX1,
    EDS_TEX2,
    EDS_TEX3
};

using Clock = std::chrono::high_resolution_clock;
using TimePoint = Clock::time_point;
using Duration = std::chrono::duration<float, std::ratio<1, 1>>;

/*
    A simple manager to take control over the input Alfisko schemes, layouts, resources
    etc. Provided to make the GUI with Alfisko stuff to be displayed on the screen 
    with some events where some of them may be registered by specifing and passing 
    extra lamba functions.
*/

class GUIManager: public irr::core::IReferenceCounted, public irr::IEventReceiver
{
    public:

        /*
            Aliases for lambda style functions that will be passed
            to funtions required those lambdas defined bellow. You can
            handle various data thanks to labda expression.
        */

        using TEventHandler = std::function<void(const CEGUI::EventArgs&)>;
        using TOnColorPicked = std::function<void(const CEGUI::Colour&)>;

        GUIManager(irr::core::smart_refctd_ptr<irr::IrrlichtDevice> device);
        virtual ~GUIManager() { destroy(); }

        void render();
        bool OnEvent(const irr::SEvent& event) override;

        void setOpacity(const char* name, float opacity);

        auto getRootWindow() const { return guiRootWindow; }
        auto getRenderer() const { return guiRenderer; }
        E_DROPDOWN_STATE getDropdownState(const char* _dropdownName) const;

        std::pair<bool, std::string> openFileDialog(const char* title, const std::vector<std::string>& filters);
        void createRootWindowFromLayout(const std::string& layout);
        CEGUI::ColourPicker* createColourPicker(bool alternativeLayout, const char* parent, const char* title, const char* name, const TOnColorPicked& onColorPicked = [](const CEGUI::Colour&) {});
        CEGUI::Window* createDropDownList(const char* name, const char* title, const std::vector<const char*>& list, const TEventHandler& eventSelectionAccepted = [](const ::CEGUI::EventArgs&) {});

        void registerSliderEvent(const char* name, float max, float step, const TEventHandler& func);

        /*
            The function is basically std::to_string(float), but with customizable floating point precision
            used mainly for text setup in GUI
        */

        template <typename T>
        static inline CEGUI::String toStringFloat(const T rvalue, const int precision = 6);
        std::string readWindowLayout(const std::string& layoutPath);

        /*
            Some extra utilities
        */

        int doesFileExists(const char* file); //!< Might be replaced with IrrlichtBAW's file system API.
        void replace(std::string& str, const std::string& from, const std::string& to);
        void showErrorMessage(const char* title, const char* message);

        template <typename... Args>
        static inline CEGUI::String ssprintf(const std::string& format, Args... args)  //!< sprintf() for std::string
        {
            size_t size = snprintf(nullptr, 0, format.c_str(), args...) + 1; // Extra space for '\0'
            std::unique_ptr<char[]> buf(new char[size]);
            snprintf(buf.get(), size, format.c_str(), args...);
            return ::CEGUI::String(std::string(buf.get(), buf.get() + size - 1)); // We don't want the '\0' inside
        }

        std::vector<std::string> split(const std::string& s, const char delimiter);

        /*
            White (1.0f, 1.0f, 1.0f) color, but it's a CEGUI::String property
            which is the editable property seen in CEED. Needed for setProperty().
        */

        const CEGUI::String WhiteProperty = CEGUI::PropertyHelper<::CEGUI::ColourRect>::toString(CEGUI::ColourRect(CEGUI::Colour(1.0f, 1.0f, 1.0f, 1.0f)));

    private:

        void init();
        void destroy();

        CEGUI::Key::Scan toCEGUIKey(const irr::EKEY_CODE& code);

        /*
            CEGUI works with singleton system, what means everything
            may be accessed by static methods if entire initialization
            process passed successfully. Because of those proporties,
            only a few main widgets are tracked by members.
        */

        irr::core::smart_refctd_ptr<irr::IrrlichtDevice> device;
        irr::video::IVideoDriver* driver = nullptr;
        CEGUI::OpenGL3Renderer* guiRenderer = nullptr; // it will be changed soon to IrrlichtBaWRenderer! The renderer will be placed as extension to IrrlichtBaW engine
        CEGUI::Window* guiRootWindow = nullptr;
        std::map<const char*, CEGUI::ColourPicker*> guicolorPickerWindows;
};

#endif // _GUI_MANAGER_HPP_INCLUDED_
