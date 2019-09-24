#include "../../ext/MitsubaLoader/ParserUtil.h"

#include "../../ext/MitsubaLoader/CElementFactory.h"

#include <functional>

namespace irr
{
namespace ext
{
namespace MitsubaLoader
{


template<>
IElement* CElementFactory::createElement<CElementFilm>(const char** _atts, ParserManager* _util)
{
	if (IElement::invalidAttributeCount(_atts, 2u))
		return nullptr;
	if (core::strcmpi(_atts[0], "type"))
		return nullptr;

	static const core::unordered_map<std::string, CElementFilm::Type, core::CaseInsensitiveHash, core::CaseInsensitiveEquals> StringToType =
	{
		{"hdrfilm",		CElementFilm::Type::HDR_FILM},
		{"tiledhdrfilm",CElementFilm::Type::TILED_HDR},
		{"ldrfilm",		CElementFilm::Type::LDR_FILM},
		{"mfilm",		CElementFilm::Type::MFILM}
	};

	auto found = StringToType.find(_atts[1]);
	if (found==StringToType.end())
	{
		ParserLog::invalidXMLFileStructure("unknown type");
		_IRR_DEBUG_BREAK_IF(false);
		return nullptr;
	}

	CElementFilm* obj = _util->objects.construct<CElementFilm>();
	if (!obj)
		return nullptr;

	obj->type = found->second;
	// defaults
	switch (obj->type)
	{
		case CElementFilm::Type::LDR_FILM:
			obj->fileFormat = CElementFilm::FileFormat::PNG;
			obj->ldrfilm = CElementFilm::LDR();
			break;
		case CElementFilm::Type::MFILM:
			obj->width = 1;
			obj->height = 1;
			obj->fileFormat = CElementFilm::FileFormat::MATLAB;
			obj->pixelFormat = CElementFilm::PixelFormat::LUMINANCE;
			obj->mfilm = CElementFilm::M();
			break;
		default:
			break;
	}
	return obj;
}


bool CElementFilm::addProperty(SPropertyElementData&& _property)
{
	bool error = type==Type::INVALID;
#define SET_PROPERTY(MEMBER,PROPERTY_TYPE)		[&]() -> void { \
		if (_property.type!=PROPERTY_TYPE) { \
			error = true; \
			return; \
		} \
		MEMBER = _property.getProperty<PROPERTY_TYPE>(); \
	}
	auto setWidth			= SET_PROPERTY(width,SPropertyElementData::Type::INTEGER);
	auto setHeight			= SET_PROPERTY(height,SPropertyElementData::Type::INTEGER);
	auto setCropOffsetX		= SET_PROPERTY(cropOffsetX,SPropertyElementData::Type::INTEGER);
	auto setCropOffsetY		= SET_PROPERTY(cropOffsetY,SPropertyElementData::Type::INTEGER);
	auto setCropWidth		= SET_PROPERTY(cropWidth,SPropertyElementData::Type::INTEGER);
	auto setCropHeight		= SET_PROPERTY(cropHeight,SPropertyElementData::Type::INTEGER);
	auto setFileFormat = [&]() -> void
	{
		if (_property.type!=SPropertyElementData::Type::STRING)
		{
			error = true;
			return;
		}
		static const core::unordered_map<std::string, FileFormat, core::CaseInsensitiveHash, core::CaseInsensitiveEquals> StringToType =
		{
			{"openexr",		OPENEXR},
			{"rgbe",		RGBE},
			{"pfm",			PFM},
			{"matlab",		MATLAB},
			{"mathematica",	MATHEMATICA},
			{"numpy",		NUMPY}
		};
		auto found = StringToType.find(_property.svalue);
		if (found!=StringToType.end())
		{
			error = true;
			return;
		}
		fileFormat = found->second;
	};
	auto setPixelFormat = [&]() -> void
	{
		if (_property.type!=SPropertyElementData::Type::STRING)
		{
			error = true;
			return;
		}
		static const core::unordered_map<std::string, PixelFormat, core::CaseInsensitiveHash, core::CaseInsensitiveEquals> StringToType =
		{
			{"luminance",		LUMINANCE},
			{"luminanceAlpha",	LUMINANCE_ALPHA},
			{"rgb",				RGB},
			{"rgba",			RGBA},
			{"xyz",				XYZ},
			{"xyza",			XYZA},
			{"spectrum",		SPECTRUM},
			{"spectrumAlpha",	SPECTRUM_ALPHA}
		};
		auto found = StringToType.find(_property.svalue);
		if (found!=StringToType.end())
		{
			error = true;
			return;
		}
		pixelFormat = found->second;
	};
	auto setComponentFormat = [&]() -> void
	{
		if (_property.type!=SPropertyElementData::Type::STRING || type==Type::LDR_FILM || type==Type::MFILM)
		{
			error = true;
			return;
		}
		static const core::unordered_map<std::string, ComponentFormat, core::CaseInsensitiveHash, core::CaseInsensitiveEquals> StringToType =
		{
			{"float16",	FLOAT16},
			{"float32",	FLOAT32},
			{"uint32",	UINT32}
		};
		auto found = StringToType.find(_property.svalue);
		if (found!=StringToType.end())
		{
			error = true;
			return;
		}
		componentFormat = found->second;
	};
	auto setBanner			= SET_PROPERTY(banner,SPropertyElementData::Type::BOOLEAN);
	auto setHighQualityEdges= SET_PROPERTY(highQualityEdges,SPropertyElementData::Type::BOOLEAN);
	

	auto dispatch = [&](auto func) -> void
	{
		switch (type)
		{
			case CElementFilm::Type::HDR_FILM:
				func(hdrfilm);
				break;
			case CElementFilm::Type::LDR_FILM:
				func(ldrfilm);
				break;
			case CElementFilm::Type::MFILM:
				func(mfilm);
				break;
			default:
				error = true;
				break;
		}
	};
#define SET_PROPERTY_TEMPLATE(MEMBER,PROPERTY_TYPE, ... )		[&]() -> void { \
		dispatch([&](auto& state) -> void { \
			IRR_PSEUDO_IF_CONSTEXPR_BEGIN(is_any_of<std::remove_reference<decltype(state)>::type,__VA_ARGS__>::value) \
			{ \
				if (_property.type!=PROPERTY_TYPE) { \
					error = true; \
					return; \
				} \
				state. ## MEMBER = _property.getProperty<PROPERTY_TYPE>(); \
			} \
			IRR_PSEUDO_IF_CONSTEXPR_END \
		}); \
	}

	auto setAttachLog = SET_PROPERTY_TEMPLATE(attachLog, SPropertyElementData::Type::BOOLEAN, HDR);
	auto setTonemapMethod = [&]() -> void
	{
		if (_property.type != SPropertyElementData::Type::STRING || type == Type::LDR_FILM)
		{
			error = true;
			return;
		}
		static const core::unordered_map<std::string, LDR::TonemapMethod, core::CaseInsensitiveHash, core::CaseInsensitiveEquals> StringToType =
		{
			{"gamma",	LDR::GAMMA},
			{"reinhard",LDR::REINHARD}
		};
		auto found = StringToType.find(_property.svalue);
		if (found != StringToType.end())
		{
			error = true;
			return;
		}
		ldrfilm.tonemapMethod = found->second;
	};
	auto setGamma = SET_PROPERTY_TEMPLATE(gamma, SPropertyElementData::Type::FLOAT, LDR);
	auto setExposure = SET_PROPERTY_TEMPLATE(exposure, SPropertyElementData::Type::FLOAT, LDR);
	auto setKey = SET_PROPERTY_TEMPLATE(key, SPropertyElementData::Type::FLOAT, LDR);
	auto setBurn = SET_PROPERTY_TEMPLATE(burn, SPropertyElementData::Type::FLOAT, LDR);
	auto setDigits = SET_PROPERTY_TEMPLATE(digits, SPropertyElementData::Type::INTEGER, M);
	auto setVariable = [&]() -> void
	{
		if (_property.type != SPropertyElementData::Type::STRING || type == Type::MFILM)
		{
			error = true;
			return;
		}
		size_t len = std::min(strlen(_property.svalue),M::MaxVarNameLen);
		memcpy(mfilm.variable,_property.svalue,len);
		mfilm.variable[len] = 0;
	};


	static const core::unordered_map<std::string, std::function<void()>, core::CaseInsensitiveHash, core::CaseInsensitiveEquals> SetPropertyMap =
	{
		{"width",				setWidth},
		{"height",				setHeight},
		{"cropOffsetX",			setCropOffsetX},
		{"cropOffsetY",			setCropOffsetY},
		{"cropWidth",			setCropWidth},
		{"cropHeight",			setCropHeight},
		{"fileFormat",			setFileFormat},
		{"pixelFormat",			setPixelFormat},
		{"componentFormat",		setComponentFormat},
		{"banner",				setBanner},
		{"highQualityEdges",	setHighQualityEdges},
		{"attachLog",			setAttachLog},
		{"tonemapMethod",		setTonemapMethod},
		{"gamma",				setGamma},
		{"exposure",			setExposure},
		{"key",					setKey},
		{"burn",				setBurn},
		{"digits",				setDigits},
		{"variable",			setVariable}
	};

	auto found = SetPropertyMap.find(_property.name);
	if (found != SetPropertyMap.end())
	{
		_IRR_DEBUG_BREAK_IF(true);
		ParserLog::invalidXMLFileStructure("No Film can have such property set with name: " + _property.name+"\nRemember we don't support \"render-time annotations\"");
		return false;
	}

	found->second();
	return !error;
}

bool CElementFilm::onEndTag(asset::IAssetLoader::IAssetLoaderOverride* _override, CGlobalMitsubaMetadata* globalMetadata)
{
	cropOffsetX = std::max(cropOffsetX,0);
	cropOffsetY = std::max(cropOffsetY,0);
	cropWidth = std::min(cropWidth,width-cropOffsetX);
	cropHeight = std::min(cropHeight,height-cropOffsetY);

	switch (type)
	{
		case Type::HDR_FILM:
			switch (fileFormat)
			{
				case OPENEXR:
					_IRR_FALLTHROUGH;
				case RGBE:
					_IRR_FALLTHROUGH;
				case PFM:
					break;
				default:
					ParserLog::invalidXMLFileStructure(getLogName() + ": film type does not support this file format");
					_IRR_DEBUG_BREAK_IF(true);
					return false;
			};
			break;
		case Type::TILED_HDR:
			switch (fileFormat)
			{
				case OPENEXR:
					break;
				default:
					ParserLog::invalidXMLFileStructure(getLogName() + ": film type does not support this file format");
					_IRR_DEBUG_BREAK_IF(true);
					return false;
			};
			break;
		case Type::LDR_FILM:
			switch (fileFormat)
			{
				case PNG:
					_IRR_FALLTHROUGH;
				case JPEG:
					_IRR_FALLTHROUGH;
				default:
					ParserLog::invalidXMLFileStructure(getLogName() + ": film type does not support this file format");
					_IRR_DEBUG_BREAK_IF(true);
					return false;
			};
			switch (pixelFormat)
			{
				case LUMINANCE_ALPHA:
					_IRR_FALLTHROUGH;
				case RGBA:
					if (type==PNG)
						break;
					_IRR_FALLTHROUGH;
				case XYZ:
					_IRR_FALLTHROUGH;
				case XYZA:
					ParserLog::invalidXMLFileStructure(getLogName() + ": film type does not support this pixel format");
					_IRR_DEBUG_BREAK_IF(true);
					return false;
					break;
				default:
					break;
			};
			break;
		case Type::MFILM:
			switch (fileFormat)
			{
				case MATLAB:
					_IRR_FALLTHROUGH;
				case MATHEMATICA:
					_IRR_FALLTHROUGH;
				case NUMPY:
					break;
				default:
					ParserLog::invalidXMLFileStructure(getLogName() + ": film type does not support this file format");
					_IRR_DEBUG_BREAK_IF(true);
					return false;
			};
			switch (pixelFormat)
			{
				case XYZ:
					_IRR_FALLTHROUGH;
				case XYZA:
					ParserLog::invalidXMLFileStructure(getLogName() + ": film type does not support this pixel format");
					_IRR_DEBUG_BREAK_IF(true);
					return false;
					break;
				default:
					break;
			};
			switch (componentFormat)
			{
				case FLOAT32:
					break;
				default:
					ParserLog::invalidXMLFileStructure(getLogName() + ": film type does not support this component format");
					_IRR_DEBUG_BREAK_IF(true);
					return false;
			};
			break;
		default:
			ParserLog::invalidXMLFileStructure(getLogName() + ": type not specified");
			_IRR_DEBUG_BREAK_IF(true);
			return false;
	}

	return true;
}


}
}
}