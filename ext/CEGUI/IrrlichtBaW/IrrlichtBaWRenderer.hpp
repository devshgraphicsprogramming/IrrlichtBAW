#ifndef _IRR_EXT_IRRLICHT_BAW_RENDERER_INCLUDED_
#define _IRR_EXT_IRRLICHT_BAW_RENDERER_INCLUDED_

#include <irrlicht.h>
#include <CEGUI/Renderer.h>
#include "../ext/CEGUI/IrrlichtBaW/IrrlichtBaWRenderTarget.hpp"
#include "../ext/CEGUI/IrrlichtBaW/IrrlichtBaWGeometryBuffer.hpp"
#include "../ext/CEGUI/IrrlichtBaW/IrrlichtBaWTextureTarget.hpp"
#include "../ext/CEGUI/IrrlichtBaW/IrrlichtBaWTexture.hpp"

namespace irr
{
namespace ext
{
namespace cegui
{

class IrrlichtBaWRenderer final : public CEGUI::Renderer
{
	public:

		IrrlichtBaWRenderer(irr::core::smart_refctd_ptr<irr::IrrlichtDevice> _device);
		virtual ~IrrlichtBaWRenderer();

		static constexpr auto &getDefaultGLSLVertexShader() { return ceguiVertexShader; }
		static constexpr auto &getDefaultGLSLFragmentShader() { return ceguiFragmentShader; }
		static auto getGpuDesriptorSetLayout3() { return gpuDescriptorSetLayout; }

		virtual CEGUI::RenderTarget& getDefaultRenderTarget() override;

		virtual CEGUI::GeometryBuffer& createGeometryBuffer() override;

		virtual void destroyGeometryBuffer(const CEGUI::GeometryBuffer& buffer) override;

		virtual void destroyAllGeometryBuffers() override;

		virtual CEGUI::TextureTarget* createTextureTarget() override;

		virtual void destroyTextureTarget(CEGUI::TextureTarget* target) override;

		virtual void destroyAllTextureTargets() override;

		virtual CEGUI::Texture& createTexture(const CEGUI::String& name) override;

		virtual CEGUI::Texture& createTexture(const CEGUI::String& name, const CEGUI::String& filename, const CEGUI::String& resourceGroup) override;

		virtual CEGUI::Texture& createTexture(const CEGUI::String& name, const CEGUI::Sizef& size) override;

		virtual void destroyTexture(CEGUI::Texture& texture) override;

		virtual void destroyAllTextures() override;

		virtual CEGUI::Texture& getTexture(const CEGUI::String& name) const override;

		virtual bool isTextureDefined(const CEGUI::String& name) const override;

		virtual void beginRendering() override;

		virtual void endRendering() override;

		virtual void setDisplaySize(const CEGUI::Sizef& size) override;

		virtual const CEGUI::Sizef& getDisplaySize() const override;

		virtual const CEGUI::Vector2f& getDisplayDPI() const override;

		virtual CEGUI::uint getMaxTextureSize() const override;

		virtual const CEGUI::String& getIdentifierString() const override;

	private:

		void initializeDescriptorSetLayout();

		irr::core::smart_refctd_ptr<irr::IrrlichtDevice> device;
		irr::video::IVideoDriver* driver = nullptr;

		std::vector<irr::core::smart_refctd_ptr<irr::video::IGPURenderpassIndependentPipeline>> gpuPipelines;
		static irr::core::smart_refctd_ptr<irr::video::IGPUDescriptorSetLayout> gpuDescriptorSetLayout;

		static CEGUI::String d_rendererID;

		/*
			Standard shaders defined by CEGUI
		*/

		static constexpr std::string_view ceguiVertexShader = R"===(
        #version 430 core

        uniform mat4 modelViewPerspMatrix;
        in vec3 inPosition;
        in vec2 inTexCoord;
        in vec4 inColour;
        out vec2 exTexCoord;
        out vec4 exColour;

        void main(void)
        {
            exTexCoord = inTexCoord;
            exColour = inColour;
            gl_Position = modelViewPerspMatrix * vec4(inPosition, 1.0);
        }
		)===";

		static constexpr std::string_view ceguiFragmentShader = R"===(
        #version 430 core
        
        layout(set = 3, binding = 0) uniform sampler2D texture0;
        in vec2 exTexCoord;
        in vec4 exColour;
        out vec4 out0;
        
        void main(void)
        {
            out0 = texture(texture0, exTexCoord) * exColour;
        }
		)===";
};

} // namespace cegui
} // namespace ext
} // namespace irr

#endif // _IRR_EXT_CEGUI_OPENGL_STATE_INCLUDED_
