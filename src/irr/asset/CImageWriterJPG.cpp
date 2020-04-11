// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#include "CImageWriterJPG.h"

#ifdef _IRR_COMPILE_WITH_JPG_WRITER_

#include "IWriteFile.h"
#include "irr/asset/format/convertColor.h"
#include "irr/asset/ICPUImageView.h"
#include "os.h"

#ifdef _IRR_COMPILE_WITH_LIBJPEG_
#include <stdio.h> // required for jpeglib.h
extern "C"
{
	#include <jpeglib.h>
	#include <jerror.h>
}

// The writer uses a 4k buffer and flushes to disk each time it's filled
#define OUTPUT_BUF_SIZE 4096

using namespace irr;
using namespace asset;

namespace 
{
typedef struct
{
	struct jpeg_destination_mgr pub;/* public fields */

	io::IWriteFile* file;		/* target file */
	JOCTET buffer[OUTPUT_BUF_SIZE];	/* image buffer */
} mem_destination_mgr;


typedef mem_destination_mgr * mem_dest_ptr;
}

// init
static void jpeg_init_destination(j_compress_ptr cinfo)
{
	mem_dest_ptr dest = (mem_dest_ptr) cinfo->dest;
	dest->pub.next_output_byte = dest->buffer;
	dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;
}


// flush to disk and reset buffer
static boolean jpeg_empty_output_buffer(j_compress_ptr cinfo)
{
	mem_dest_ptr dest = (mem_dest_ptr) cinfo->dest;

	// for now just exit upon file error
	if (dest->file->write(dest->buffer, OUTPUT_BUF_SIZE) != OUTPUT_BUF_SIZE)
		ERREXIT (cinfo, JERR_FILE_WRITE);

	dest->pub.next_output_byte = dest->buffer;
	dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;

	return TRUE;
}


static void jpeg_term_destination(j_compress_ptr cinfo)
{
	mem_dest_ptr dest = (mem_dest_ptr) cinfo->dest;
	const int32_t datacount = (int32_t)(OUTPUT_BUF_SIZE - dest->pub.free_in_buffer);
	// for now just exit upon file error
	if (dest->file->write(dest->buffer, datacount) != datacount)
		ERREXIT (cinfo, JERR_FILE_WRITE);
}


// set up buffer data
static void jpeg_file_dest(j_compress_ptr cinfo, io::IWriteFile* file)
{
	if (cinfo->dest == nullptr)
	{ /* first time for this JPEG object? */
		cinfo->dest = (struct jpeg_destination_mgr *)
			(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo,
						JPOOL_PERMANENT,
						sizeof(mem_destination_mgr));
	}

	mem_dest_ptr dest = (mem_dest_ptr) cinfo->dest;  /* for casting */

	/* Initialize method pointers */
	dest->pub.init_destination = jpeg_init_destination;
	dest->pub.empty_output_buffer = jpeg_empty_output_buffer;
	dest->pub.term_destination = jpeg_term_destination;

	/* Initialize private member */
	dest->file = file;
}

/* write_JPEG_memory: store JPEG compressed image into memory.
*/
static bool writeJPEGFile(io::IWriteFile* file, const const asset::ICPUImage* image, uint32_t quality)
{
	const auto& imageParams = image->getCreationParameters();
	const auto& region = image->getRegions().begin();
	auto format = imageParams.format;

	IImage::SBufferCopy::TexelBlockInfo blockInfo(format);
	core::vector3du32_SIMD trueExtent = IImage::SBufferCopy::TexelsToBlocks(region->getTexelStrides(), blockInfo);

	bool grayscale = (format == asset::EF_R8_SRGB) || (format == asset::EF_R8_UNORM);
	
	core::vector3d<uint32_t> dim;
	dim.X = trueExtent.X;
	dim.Y = trueExtent.Y;
	dim.Z = trueExtent.Z;

	const auto texelOrBlockByteSize = asset::getTexelOrBlockBytesize(format) * trueExtent.X;

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);

	jpeg_create_compress(&cinfo);
	jpeg_file_dest(&cinfo, file);
	cinfo.image_width = dim.X;
	cinfo.image_height = dim.Y;
	cinfo.input_components = grayscale ? 1 : 3;
	cinfo.in_color_space = grayscale ? JCS_GRAYSCALE : JCS_RGB;

	jpeg_set_defaults(&cinfo);

	if ( 0 == quality )
		quality = 85;

	jpeg_set_quality(&cinfo, quality, TRUE);
	jpeg_start_compress(&cinfo, TRUE);

	core::smart_refctd_ptr<ICPUImage> newConvertedImage;
	{
		auto copyImageForConverting = core::smart_refctd_ptr_static_cast<ICPUImage>(image->clone());
		auto copyImageParams = copyImageForConverting->getCreationParameters();
		auto copyBuffer = copyImageForConverting->getBuffer();
		auto copyRegion = copyImageForConverting->getRegions().begin();

		auto newCpuBuffer = core::make_smart_refctd_ptr<ICPUBuffer>(copyBuffer->getSize());
		memcpy(newCpuBuffer->getPointer(), copyBuffer->getPointer(), newCpuBuffer->getSize());

		auto newRegions = core::make_refctd_dynamic_array<core::smart_refctd_dynamic_array<ICPUImage::SBufferCopy>>(1u);
		ICPUImage::SBufferCopy& region = newRegions->front();
		region.imageSubresource.mipLevel = copyRegion->imageSubresource.mipLevel;
		region.imageSubresource.baseArrayLayer = copyRegion->imageSubresource.baseArrayLayer;
		region.imageSubresource.layerCount = copyRegion->imageSubresource.layerCount;
		region.bufferOffset = copyRegion->bufferOffset;
		region.bufferRowLength = copyRegion->bufferRowLength;
		region.bufferImageHeight = copyRegion->bufferImageHeight;
		region.imageOffset = copyRegion->imageOffset;
		region.imageExtent = copyRegion->imageExtent;

		CMatchedSizeInOutImageFilterCommon::state_type state;
		state.extent = imageParams.extent;

		switch (imageParams.format)
		{
			case asset::EF_R8G8B8_UNORM:
			{
				copyImageParams.format = EF_R8G8B8_SRGB;
				newConvertedImage = ICPUImage::create(std::move(copyImageParams));
				newConvertedImage->setBufferAndRegions(std::move(newCpuBuffer), newRegions);
				state.inImage = newConvertedImage.get();

				CConvertFormatImageFilter<EF_R8G8B8_UNORM, EF_R8G8B8_SRGB> convertFiler;
				convertFiler.execute(&state);
			}
			break;

			case asset::EF_B5G6R5_UNORM_PACK16:
			{
				copyImageParams.format = EF_R8G8B8_SRGB;
				newConvertedImage = ICPUImage::create(std::move(copyImageParams));
				newConvertedImage->setBufferAndRegions(std::move(newCpuBuffer), newRegions);
				state.inImage = newConvertedImage.get();

				CConvertFormatImageFilter<EF_B5G6R5_UNORM_PACK16, EF_R8G8B8_SRGB> convertFiler;
				convertFiler.execute(&state);
			}
			break;

			case asset::EF_A1R5G5B5_UNORM_PACK16:
			{
				copyImageParams.format = EF_R8G8B8_SRGB;
				newConvertedImage = ICPUImage::create(std::move(copyImageParams));
				newConvertedImage->setBufferAndRegions(std::move(newCpuBuffer), newRegions);
				state.inImage = newConvertedImage.get();

				CConvertFormatImageFilter<EF_A1R5G5B5_UNORM_PACK16, EF_R8G8B8_SRGB> convertFiler;
				convertFiler.execute(&state);
			}
			break;

			case asset::EF_R8_UNORM:
			{
				copyImageParams.format = EF_R8_SRGB;
				newConvertedImage = ICPUImage::create(std::move(copyImageParams));
				newConvertedImage->setBufferAndRegions(std::move(newCpuBuffer), newRegions);
				state.inImage = newConvertedImage.get();

				CConvertFormatImageFilter<EF_R8_UNORM, EF_R8_SRGB> convertFiler;
				convertFiler.execute(&state);
			}
			break;

			default:
			{
				newConvertedImage = std::move(copyImageForConverting);
			}
			break;
		}
	}

	auto convertedImageParams = newConvertedImage->getCreationParameters();

	const auto JPG_BYTE_PITCH = texelOrBlockByteSize;
	uint8_t * dest = new uint8_t[JPG_BYTE_PITCH];

	if (dest)
	{
		const uint32_t pitch = JPG_BYTE_PITCH;
		JSAMPROW row_pointer[1];      /* pointer to JSAMPLE row[s] */
		row_pointer[0] = dest;
		
		uint8_t* src = (uint8_t*)newConvertedImage->getBuffer()->getPointer();
		
		/* Switch up, write from bottom -> top because the texture is flipped from OpenGL side */
		uint32_t eof = cinfo.image_height * cinfo.image_width * cinfo.input_components;
		
		while (cinfo.next_scanline < cinfo.image_height)
		{
			switch (convertedImageParams.format) 
			{
				case asset::EF_R8_SRGB:
					_IRR_FALLTHROUGH;
				case asset::EF_R8G8B8_SRGB:
					memcpy(dest, src, pitch);
					break;
				default:
				{
					os::Printer::log("Unsupported color format, operation aborted.", ELL_ERROR);
					delete [] dest;
					return false;
				}
			}
			
			src += pitch;
			jpeg_write_scanlines(&cinfo, row_pointer, 1);
		}

		delete [] dest;

		/* Step 6: Finish compression */
		jpeg_finish_compress(&cinfo);
	}

	/* Step 7: Destroy */
	jpeg_destroy_compress(&cinfo);

	return (dest != 0);
}
#endif // _IRR_COMPILE_WITH_LIBJPEG_

CImageWriterJPG::CImageWriterJPG()
{
#ifdef _IRR_DEBUG
	setDebugName("CImageWriterJPG");
#endif
}

bool CImageWriterJPG::writeAsset(io::IWriteFile* _file, const SAssetWriteParams& _params, IAssetWriterOverride* _override)
{
#if !defined(_IRR_COMPILE_WITH_LIBJPEG_ )
	return false;
#else
	SAssetWriteContext ctx{ _params, _file };

	const asset::ICPUImageView* imageView = IAsset::castDown<ICPUImageView>(_params.rootAsset);
	const auto smartImage = IImageAssetHandlerBase::getTopImageDataForCommonWriting(imageView);
	const auto image = smartImage.get();

    io::IWriteFile* file = _override->getOutputFile(_file, ctx, {image, 0u});
    const asset::E_WRITER_FLAGS flags = _override->getAssetWritingFlags(ctx, image, 0u);
    const float comprLvl = _override->getAssetCompressionLevel(ctx, image, 0u);

	return writeJPEGFile(file, image, (!!(flags & asset::EWF_COMPRESSED)) * static_cast<uint32_t>((1.f-comprLvl)*100.f)); // if quality==0, then it defaults to 75

#endif//!defined(_IRR_COMPILE_WITH_LIBJPEG_ )
}

#undef OUTPUT_BUF_SIZE
#endif
