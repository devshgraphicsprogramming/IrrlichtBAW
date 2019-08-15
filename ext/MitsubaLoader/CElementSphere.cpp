#include "../../ext/MitsubaLoader/CElementSphere.h"

#include "../../ext/MitsubaLoader/ParserUtil.h"
#include "../../ext/MitsubaLoader/CElementTransform.h"
#include "../../ext/MitsubaLoader/CSimpleElement.h"

namespace irr { namespace ext { namespace MitsubaLoader {


bool CElementSphere::processAttributes(const char** _atts)
{
	//only type is an acceptable argument
	for (int i = 0; _atts[i]; i += 2)
	{
		if (std::strcmp(_atts[i], "type"))
		{
			ParserLog::wrongAttribute(_atts[i], getLogName());
			return false;
		}
	}

	return true;
}

bool CElementSphere::onEndTag(asset::IAssetManager& _assetManager, IElement* _parent)
{
	mesh = _assetManager.getGeometryCreator()->createSphereMesh(radius,32,32);
	if (!mesh)
		return false;

	if (flipNormalsFlag)
		flipNormals(_assetManager);

	core::vector3df_SIMD translate = transform.getTranslation3D();
	translate += center;
	transform.setTranslation(translate);

	return _parent->processChildData(this);
}

bool CElementSphere::processChildData(IElement* _child)
{
	switch (_child->getType())
	{
	case IElement::Type::TRANSFORM:
	{
		CElementTransform* transformElement = static_cast<CElementTransform*>(_child);

		if (transformElement->getName() == "toWorld")
			this->transform = static_cast<CElementTransform*>(_child)->getMatrix();
		else
			ParserLog::mitsubaLoaderError("Unqueried attribute '" + transformElement->getName() + "' in element 'shape'");

		return true;
	}
	case IElement::Type::BOOLEAN:
	{
		CElementBoolean* boolElement = static_cast<CElementBoolean*>(_child);
		const std::string  elementName = boolElement->getNameAttribute();

		if (elementName == "flipNormals")
		{
			flipNormalsFlag = boolElement->getValueAttribute();
		}
		else
		{
			//warning
			ParserLog::mitsubaLoaderError("Unqueried attribute " + elementName + " in element \"shape\"");
		}

		return true;
	}
	case IElement::Type::POINT:
	{
		CElementPoint* pointElement = static_cast<CElementPoint*>(_child);
		const std::string  elementName = pointElement->getNameAttribute();

		if (elementName == "center")
		{
			center = pointElement->getValueAttribute();
		}
		else
		{
			//warning
			ParserLog::mitsubaLoaderError("Unqueried attribute " + elementName + " in element \"shape\"");
		}

		return true;
	}
	case IElement::Type::FLOAT:
	{
		CElementFloat* floatElement = static_cast<CElementFloat*>(_child);
		const std::string  elementName = floatElement->getNameAttribute();

		if (elementName == "radius")
		{
			radius = floatElement->getValueAttribute();
		}
		else
		{
			//warning
			ParserLog::mitsubaLoaderError("Unqueried attribute " + elementName + " in element \"shape\"");
		}

		return true;
	}
	default:
		ParserLog::wrongChildElement(getLogName(), _child->getLogName());
		return false;
	}
}

}
}
}