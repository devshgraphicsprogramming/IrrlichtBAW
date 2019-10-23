#include "CBufferLoaderBIN.h"

namespace irr
{
	namespace asset
	{
		asset::SAssetBundle CBufferLoaderBIN::loadAsset(io::IReadFile* _file, const asset::IAssetLoader::SAssetLoadParams& _params, asset::IAssetLoader::IAssetLoaderOverride* _override, uint32_t _hierarchyLevel)
		{
			if (!_file)
				return {};

			SContext ctx;

			ctx.files = _file;
			ctx.files->grab();

			uint16_t index{};

			for (io::IReadFile* file = ctx.files; file; ++file)
			{
				ctx.sourceCodeBuffersCache.emplace_back(core::make_smart_refctd_ptr<char>(_IRR_NEW_ARRAY(char, file->getSize())));
				file->read(ctx.sourceCodeBuffersCache[index++].get(), file->getSize());
			}

			return SAssetBundle(ctx.sourceCodeBuffersCache); // is it valid actually?
		}

		bool CBufferLoaderBIN::isALoadableFileFormat(io::IReadFile* _file) const
		{
			std::string filePath(_file->getFileName().c_str());
			uint16_t beggingOfFileExtIndex{};

			auto getIndexInArrayForFileExtension = [&](auto& index)
			{
				for (uint16_t i = filePath.size() - 1; i > 0u; --i)
					if (filePath[i] == '.')
					{
						index = i;
						return true;
					}
			};

			if (!getIndexInArrayForFileExtension(beggingOfFileExtIndex))
				return false;

			std::string fileExtension;
			for (uint16_t i = beggingOfFileExtIndex; i < filePath.size(); ++i)
				fileExtension += filePath[i];

			auto availableExtensions(getAssociatedFileExtensions());
			for (const char* extension = const_cast<char*>(availableExtensions[0]); extension; ++extension)
			{
				auto isFormatValid = [&](const char* _ext)
				{
					std::string checkedExtension(_ext);
					if (checkedExtension == fileExtension)
						return true;
				};

				if (isFormatValid(extension))
					return true;
			}
			return false;

		}
	}
}