#ifndef _IRR_EXT_IRRLICHT_BAW_GEOMETRY_BUFFER_INCLUDED_
#define _IRR_EXT_IRRLICHT_BAW_GEOMETRY_BUFFER_INCLUDED_

#include <irrlicht.h>
#include <CEGUI/GeometryBuffer.h>

namespace irr
{
namespace ext
{
namespace cegui
{

class IrrlichtBaWGeometryBuffer final : public CEGUI::GeometryBuffer
{
	public:

		IrrlichtBaWGeometryBuffer();
		virtual ~IrrlichtBaWGeometryBuffer();

        virtual void draw() const override;

        virtual void setTranslation(const CEGUI::Vector3f& v) override;

        virtual void setRotation(const CEGUI::Quaternion& r) override;

        virtual void setPivot(const CEGUI::Vector3f& p) override;

        virtual void setClippingRegion(const CEGUI::Rectf& region) override;

        virtual void appendVertex(const CEGUI::Vertex& vertex) override;

        virtual void appendGeometry(const CEGUI::Vertex* const vbuff, CEGUI::uint vertex_count) override;

        virtual void setActiveTexture(CEGUI::Texture* texture) override;

        virtual void reset() override;

        virtual CEGUI::Texture* getActiveTexture() const override;

        virtual CEGUI::uint getVertexCount() const override;

        virtual CEGUI::uint getBatchCount() const override;

        virtual void setRenderEffect(CEGUI::RenderEffect* effect) override;

        virtual CEGUI::RenderEffect* getRenderEffect() override;

        virtual void setBlendMode(const CEGUI::BlendMode mode) override;

        virtual CEGUI::BlendMode getBlendMode() const override;

        virtual void setClippingActive(const bool active) override;

        virtual bool isClippingActive() const override;

    private:

        void createGpuPipeline();

        irr::video::IVideoDriver* driver = nullptr;
        irr::core::smart_refctd_ptr<irr::video::IGPURenderpassIndependentPipeline> gpuPipeline;
};

} // namespace cegui
} // namespace ext
} // namespace irr

#endif // _IRR_EXT_IRRLICHT_BAW_GEOMETRY_BUFFER_INCLUDED_
