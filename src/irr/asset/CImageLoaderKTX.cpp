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

#include "CImageLoaderKTX.h"

#ifdef _IRR_COMPILE_WITH_KTX_LOADER_

#ifdef _IRR_COMPILE_WITH_GLI_
#include "gli/gli.hpp"
#endif

#include "irr/asset/ICPUTexture.h"
#include "irr/asset/CImageData.h"
#include "COpenGLTexture.h"
#include "CReadFile.h"
#include "os.h"

namespace irr
{
namespace asset
{

static bool checkKTXSignature(unsigned char* buffer)
{
    // Unique KTX identifier as per the Khronos KTX specification
    // https://www.khronos.org/opengles/sdk/tools/KTX/file_format_spec/#2.1
    unsigned char signature[12] = { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };
    
    for (unsigned int i = 0; i < 12; i++)
    {
        if (buffer[i] != signature[i])
            return false;
    }
    return true;
}

//! returns true if the file maybe is able to be loaded by this class
bool CImageLoaderKTX::isALoadableFileFormat(io::IReadFile* _file) const
{
#ifdef _IRR_COMPILE_WITH_GLI_
    if (!_file)
        return false;

    const size_t prevPos = _file->getPos();
    
    unsigned char buffer[12];

    // Read the first few bytes of the KTX _file
    if (_file->read(buffer, 12) != 12)
    {
        _file->seek(prevPos);
        return false;
    }

    _file->seek(prevPos);

    // Check if it really is a KTX _file
    return checkKTXSignature(buffer);
#else
    return false;
#endif // _IRR_COMPILE_WITH_GLI_
}


// load in the image data
asset::SAssetBundle CImageLoaderKTX::loadAsset(
    io::IReadFile*                                  _file,
    const asset::IAssetLoader::SAssetLoadParams&    _params,
    asset::IAssetLoader::IAssetLoaderOverride*      _override,
    uint32_t                                        _hierarchyLevel)
{
    core::vector<asset::CImageData*> images;
#ifdef _IRR_COMPILE_WITH_GLI_
    if (!_file)
        return {};

    unsigned char buffer[12];

    // Read the first few bytes of the KTX _file
    if (_file->read(buffer, 12) != 12)
    {
        os::Printer::log("LOAD KTX: can't read _file\n", _file->getFileName().c_str(), ELL_ERROR);
        return {};
    }

    // Check if it really is a KTX _file
    if (!checkKTXSignature(buffer))
    {
        os::Printer::log("LOAD KTX: not really a ktx\n", _file->getFileName().c_str(), ELL_ERROR);
        return {};
    }

    const char* filename = _file->getFileName().c_str();
    gli::texture texture = gli::load_ktx(filename);
    if (texture.empty())
    {
        os::Printer::log("Failed to load texture at path ", filename, ELL_ERROR);
        return {};
    }

    // Instantiate translation class to convert GLI enums into OpenGL values
    gli::gl GL(gli::gl::PROFILE_GL33);
    const gli::gl::format textureFormat = GL.translate(texture.format(), texture.swizzles());
    GLenum textureTarget = GL.translate(texture.target());

    uint32_t nullOffset[3] = { 0, 0, 0 };
    for (unsigned int mipLevel = 0; mipLevel < texture.levels(); mipLevel++)
    {
        uint32_t imageSize[3] = { texture.extent(mipLevel)[0], texture.extent(mipLevel)[1], texture.extent(mipLevel)[2] };
        for (unsigned int face = 0; face < texture.faces(); face++)
        {
            asset::CImageData* image = nullptr;
            if (gli::is_compressed(texture.format()))
            {
                image = new asset::CImageData(texture.data(0, face, mipLevel), nullOffset, imageSize, mipLevel, irr::video::COpenGLTexture::getColorFormatFromSizedOpenGLFormat(textureFormat.Internal));
            }
            else
            {
                // From the KTX Specification, "Uncompressed texture data matches a GL_UNPACK_ALIGNMENT of 4."
                image = new asset::CImageData(texture.data(0, face, mipLevel), nullOffset, imageSize, mipLevel, irr::video::COpenGLTexture::getColorFormatFromSizedOpenGLFormat(textureFormat.Internal), 4u);
            }
            images.push_back(image);
        }
    }
    
    ICPUTexture* cpuTexture = ICPUTexture::create(images, _file->getFileName().c_str());
    for (auto& image : images)
        image->drop();
    return SAssetBundle({ core::smart_refctd_ptr<IAsset>(cpuTexture, core::dont_grab) });

#endif
    return {};
}

}// end namespace asset
}//end namespace irr

#endif  // _IRR_COMPILE_WITH_KTX_LOADER_