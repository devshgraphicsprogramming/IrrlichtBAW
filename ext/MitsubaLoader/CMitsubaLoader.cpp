#include "../../ext/MitsubaLoader/CMitsubaLoader.h"
#include "../3rdparty/libexpat/expat/lib/expat.h"
#include "irr/asset/SCPUMesh.h"
#include "irr/asset/IAssetManager.h"

#include "../../ext/MitsubaLoader/IElement.h"
#include "../../ext/MitsubaLoader/CMitsubaScene.h"

#include "IrrlichtDevice.h"
#include <stack>
#include <chrono>

#include "ParserUtil.h"

namespace irr { namespace ext { namespace MitsubaLoader {

//TODO: 
/*
 - proper handling of incorrect .xml file structure (in some situations parsing should be stoped and nullptr should be returned
   from CMitsubaLoader::loadAsset, and in other situations only warning should be shown.)
 - handle 'version' attribute

 - make sure that default values for <float .. /> <boolean .. /> etc. are correct
 - test atof function (CSimpleElement.h) or use ostringstream operator >> instead
 
 - how should loader treat upper/lower case letters in xml?

 - clean up memory dynamically allocated by ParserData::scene

 - resolve header include conflict (IElement.h needs to include ParserUtil.h (only for ParserLog), and ParserUtil.h needs to include IElement.h)

 - check, if transform name is indeed "toWorld"

 - create an issue about isALoadableFileFormat in CBAWMeshFileLoader

 - disable asset cacheing (use ECF_DUPLICATE_TOP_LEVEL)

 - LOG!

 - celementshapeobj todos

 - idk if I use flipsurfaces correctly.. 

 - acording CElementColor: 
		- is single value allowed in "value" attribute for sure?
		- spectrum?

 - acording cylinder: close top leaves one top still closed, also this:
	"Note that the cylinder does not have endcaps � also,
	it�s interior has inward-facing normals, which most scattering models in Mitsuba will treat as fully
	absorbing. If this is not desirable, consider using the twosided plugin." ...

*/


CMitsubaLoader::CMitsubaLoader(IrrlichtDevice* device)
	:m_device(device),
	m_assetManager(device->getAssetManager())
{
#ifdef _IRR_DEBUG
	setDebugName("CMitsubaLoader");
#endif
}

bool CMitsubaLoader::isALoadableFileFormat(io::IReadFile* _file) const
{
	//not implemented
	//_IRR_DEBUG_BREAK_IF(true);
	return true;
}

		//SHAPE CUBE ON BEGIN TAG printed twice

const char** CMitsubaLoader::getAssociatedFileExtensions() const
{
	static const char* ext[]{ "xml", nullptr };
	return ext;
}

asset::IAsset* CMitsubaLoader::loadAsset(io::IReadFile* _file, const SAssetLoadParams& _params, IAssetLoaderOverride* _override, uint32_t _hierarchyLevel)
{
	//TODO: error handling
	XML_Parser parser = XML_ParserCreate(nullptr);
	XML_SetElementHandler(parser, elementHandlerStart, elementHandlerEnd);

	ParserData data(m_assetManager, parser);
	
	//from now data (instance of ParserData struct) will be visible to expat handlers
	XML_SetUserData(parser, &data);

	const char* buff = new char[_file->getSize()];

	//will replace it with something like safeRead from CBAWMeshLoader
	_file->read((void*)buff, _file->getSize());

	XML_Status parseStatus = XML_Parse(parser, buff, _file->getSize(), 0);

	switch (parseStatus)
	{
	case XML_STATUS_ERROR:
		std::cout << "Parse status: XML_STATUS_ERROR\n";
		break;

	case XML_STATUS_OK:
		std::cout << "Parse status: XML_STATUS_OK\n";
		break;

	case XML_STATUS_SUSPENDED:
		std::cout << "Parse status: XML_STATUS_SUSPENDED\n";
		break;

	default:
		std::cout << "Parse status: XML_STATUS_SUSPENDED\n";
		break;

	}

	XML_ParserFree(parser);

	if (!data.scene)
		return nullptr;
	
	return data.scene->releaseMesh();
}

}
}
}