#ifndef _APPLICATION_HANDLER_HPP_INCLUDED_
#define _APPLICATION_HANDLER_HPP_INCLUDED_

#include "SceneManager.hpp"
#include "GUIManager.hpp"
#include "../../common/QToQuitEventReceiver.h"

class ApplicationHandler
{
	public:

		ApplicationHandler(irr::core::smart_refctd_ptr<irr::IrrlichtDevice> _device);
		virtual ~ApplicationHandler();

		bool getStatus();
		void work();		

	private:

		void setupManagers();
		bool status;

		GUIManager guiManager;		                                            
		SceneManager sceneManager;	                                          
		irr::core::smart_refctd_ptr<irr::IrrlichtDevice> device;
		QToQuitEventReceiver quitEventReceiver;

        /*
            Commond data used for serialization by either 
            GUI and scene, contatins some default constants,
            some variables that will be changed after GUI or scene
            interactions.
        */

		struct CommondData
		{
            /*
                 Variables data updated by the user using GUI.
            */

            struct SGUIState
            {
                struct
                {
                    irr::core::vector3df color;
                } emissive;

                struct
                {
                    E_DROPDOWN_STATE sourceDropdown = EDS_CONSTANT;
                    irr::core::vector3df constantColor;
                } albedo;

                struct
                {
                    bool isIsotropic = false;
                    E_DROPDOWN_STATE sourceDropdown = EDS_CONSTANT;
                    float constValue1 = 0.f;
                    float constValue2 = 0.f;
                } roughness;

                struct
                {
                    E_DROPDOWN_STATE sourceDropdown = EDS_CONSTANT;
                    irr::core::vector3df constantReal{ 1.33f };
                    irr::core::vector3df constantImag;
                } refractionIndex;

                struct
                {
                    E_DROPDOWN_STATE sourceDropdown = EDS_CONSTANT;
                    float constValue = 0.f;
                } metallic;

                struct
                {
                    float height = 0.01f;
                    bool enabled = false;
                } bumpMapping;

                struct
                {
                    bool enabled = false;
                } ambientOcclusion;

                struct
                {
                    irr::core::vector3df color{ 1.f, 1.f, 1.f };
                    irr::core::vector3df constantPosition;
                    bool animated = true;
                    float intensity = 800.f;
                } light;
            } guiState;

            struct SLightAnimData
            {
                float radius;
                irr::core::vector2df center;
                irr::core::vector3df position;
                TimePoint startTime = Clock::now();

                inline void update()
                {
                    auto now = Clock::now();
                    const float time = std::chrono::duration_cast<Duration>(now - startTime).count();

                    position.X = center.X + std::sin(time) * radius;
                    position.Z = center.Y + std::cos(time) * radius;
                }
            } lightAnimatedData;

            struct {
                TimePoint TimePointLastHeightFactorChange = Clock::now();
                float HeightFactorChanged = false;
            } derivMapGeneration;

            /*
                Some default constant values used for slider settings in GUI 
            */

            static constexpr std::string_view DROPDOWN_ALBEDO_NAME = "MaterialParamsWindow/AlbedoDropDownList/DropDown_Albedo";
            static constexpr std::string_view DROPDOWN_ROUGHNESS_NAME = "MaterialParamsWindow/RoughnessDropDownList/DropDown_Roughness";
            static constexpr std::string_view DROPDOWN_RI_NAME = "MaterialParamsWindow/RIDropDownList/DropDown_RI";
            static constexpr std::string_view DROPDOWN_METALLIC_NAME = "MaterialParamsWindow/MetallicDropDownList/DropDown_Metallic";

            //static constexpr float sliderRIRange = 2.0f;
            static constexpr float sliderRealRIRange = 4.0f;
            static constexpr float sliderImagRIRange = 10.0f;
            static constexpr float sliderMetallicRange = 1.0f;
            static constexpr float sliderRoughness1Range = 1.0f;
            static constexpr float sliderRoughness2Range = 1.0f;
            static constexpr float sliderBumpHeightRange = 2.0f;
            static constexpr float sliderLightIntensityRange = 9999.f; // "turns out i cant set min value on cegui slider XD"
            static constexpr float defaultOpacity = 0.85f;

            /*
                Default title & filtering for the choose-your-file dialog
            */

            static constexpr std::string_view imageFileDialogTitle = "Select Texture";
            static constexpr std::string_view meshFileDialogTitle = "Select Mesh";

            const std::vector<std::string> imageFileDialogFilters = 
            {
                "Image (*.jpg, *.jpeg, *.png, *.bmp, *.tga, *.dds, *.gif)",
                "*.jpg *.jpeg *.png *.bmp *.tga *.dds *.gif"
            };

            const std::vector<std::string> meshFileDialogFilters = 
            {
                "Mesh (*.ply *.stl *.baw *.x *.obj)",
                "*.ply *.stl *.baw *.x *.obj"
            };

		} commonData;

        void initializeDropdown();
        void initializeTooltip();
        void setLightPosition(const irr::core::vector3df& _lightPos);
        void setGUIForConstantIoR();
        void resetGUIAfterConstantIoR();

        void onEventForAOTextureBrowse(const CEGUI::EventArgs& event);
        void onEventForAOTextureBrowse_EditBox(const CEGUI::EventArgs& event);
        void onEventForBumpTextureBrowse(const CEGUI::EventArgs& event);
        void onEventForBumpTextureBrowse_EditBox(const CEGUI::EventArgs& event);
        void onEventForTextureBrowse(const CEGUI::EventArgs& event);
        void onEventForMeshBrowse(const CEGUI::EventArgs& event);
};

#endif // _APPLICATION_HANDLER_HPP_INCLUDED_
