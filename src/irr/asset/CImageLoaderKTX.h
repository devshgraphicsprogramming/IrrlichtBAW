/*
MIT License
Copyright (c) 2019 Achal Pandey
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef __C_IMAGE_LOADER_KTX_H_INCLUDED__
#define __C_IMAGE_LOADER_KTX_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_KTX_LOADER_

#include "irr/asset/IAssetLoader.h"

namespace irr
{
namespace asset
{


/*!
    Surface Loader for KTX images
*/
class CImageLoaderKTX : public asset::IAssetLoader
{
public:
    virtual bool isALoadableFileFormat(io::IReadFile* _file) const override;

    virtual const char** getAssociatedFileExtensions() const override
    {
        static const char* ext[]{ "ktx", nullptr };
        return ext;
    }

    virtual uint64_t getSupportedAssetTypesBitfield() const override { return asset::IAsset::ET_IMAGE; }

    virtual asset::SAssetBundle loadAsset(
        io::IReadFile*                                  _file,
        const asset::IAssetLoader::SAssetLoadParams&    _params,
        asset::IAssetLoader::IAssetLoaderOverride*      _override       =   nullptr,
        uint32_t                                        _hierarchyLevel =   0u) override;
};


} // end namespace asset
} // end namespace irr

#endif // _IRR_COMPILE_WITH_KTX_LOADER_
#endif
