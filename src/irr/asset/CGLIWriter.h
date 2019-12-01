// Copyright (C) 2009-2012 Gaz Davidson
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#pragma once

#include "irr/asset/IAssetLoader.h"

namespace irr
{
	namespace asset
	{
		//! Texture writer capable of saving in .ktx, .dds and .kmg file extensions
		class CGLIWriter : public asset::IAssetWriter
		{
		protected:
			virtual ~CGLIWriter() {}

		public:
			CGLIWriter() {}

			virtual const char** getAssociatedFileExtensions() const override
			{
				static const char* extensions[]{ "ktx", "dds", "kmg", nullptr };
				return extensions;
			}

			virtual uint64_t getSupportedAssetTypesBitfield() const override { return asset::IAsset::ET_IMAGE; }

			virtual uint32_t getSupportedFlags() override { return asset::EWF_NONE; }

			virtual uint32_t getForcedFlags() override { return 0u; }

			virtual bool writeAsset(io::IWriteFile* _file, const SAssetWriteParams& _params, IAssetWriterOverride* _override = nullptr) override;

		protected:
			bool writeGLIFile(io::IWriteFile* file, const asset::ICPUImage* image);

		private:
			
		};
	}
}
