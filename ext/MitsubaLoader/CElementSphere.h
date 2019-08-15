#ifndef __C_ELEMENT_SHAPE_SPHERE_H_INCLUDED__
#define __C_ELEMENT_SHAPE_SPHERE_H_INCLUDED__

#include "../../ext/MitsubaLoader/IElement.h"
#include "../../ext/MitsubaLoader/IShape.h"
#include "irrlicht.h"

namespace irr { namespace ext { namespace MitsubaLoader {


class CElementSphere : public IElement, public IShape
{
public:
	CElementSphere()
		:radius(1.0f) {};

	virtual bool processAttributes(const char** _args) override;
	virtual bool onEndTag(asset::IAssetManager& _assetManager, IElement* _parent) override;
	virtual IElement::Type getType() const override { return IElement::Type::SHAPE; };
	virtual std::string getLogName() const override { return "shape cube"; };
	virtual bool processChildData(IElement* child) override;

private:
	core::vector3df_SIMD center;
	float radius;

};

}
}
}

#endif