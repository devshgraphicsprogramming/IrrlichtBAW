#include <irrlicht.h>
#include <array>
#include <fstream>

using IRRDevice = irr::core::smart_refctd_ptr<irr::IrrlichtDevice>;

using IRRGPUImage = irr::core::smart_refctd_ptr<irr::video::IGPUImage>;
using IRRGPUImageView = irr::core::smart_refctd_ptr<irr::video::IGPUImageView>;

struct IRRGPUImagePair
{
	IRRGPUImage gpuImage;
	IRRGPUImageView gpuImageView;
};

using IRRGPUDescriptorSet = irr::core::smart_refctd_ptr<irr::video::IGPUDescriptorSet>;
using IRRGPUDescriptorSetLayout = irr::core::smart_refctd_ptr<irr::video::IGPUDescriptorSetLayout>;

struct IRRGPUDescriptorSetPair
{
	IRRGPUDescriptorSet gpuDescriptorSet;
	IRRGPUDescriptorSetLayout gpuDescriptorSetLayout;
};

using IRRGPUComputePipeline = irr::core::smart_refctd_ptr<irr::video::IGPUComputePipeline>;

inline irr::asset::E_FORMAT SRGBFromatToUnorm(irr::asset::E_FORMAT format)
{
	using namespace irr::asset;

	switch (format)
	{
	case EF_R8_SRGB:
		return EF_R8_UNORM;
	case EF_R8G8_SRGB:
		return EF_R8G8_UNORM;
	case EF_R8G8B8_SRGB:
		return EF_R8G8B8_UNORM;
	case EF_R8G8B8A8_SRGB:
		return EF_R8G8B8A8_UNORM;
	case EF_R8_UNORM:
	case EF_R8G8_UNORM:
	case EF_R8G8B8_UNORM:
	case EF_R8G8B8A8_UNORM:
		return format;
	default:
		return EF_UNKNOWN;
	}
}

inline irr::asset::E_FORMAT UNORMFormatToSFLOAT(irr::asset::E_FORMAT format)
{
	using namespace irr::asset;

	switch (format)
	{
	case EF_R8_UNORM:
		return EF_R32_SFLOAT;
	case EF_R8G8_UNORM:
		return EF_R32G32_SFLOAT;
	case EF_R8G8B8_UNORM:
		return EF_R32G32B32_SFLOAT;
	case EF_R8G8B8A8_UNORM:
		return EF_R32G32B32A32_SFLOAT;
	default:
		return EF_UNKNOWN;
	}
}

inline irr::asset::VkExtent3D getImageDimensions(IRRGPUImage img)
{
	return img->getCreationParameters().extent;
}

inline IRRGPUImageView gpuImageViewFromGpuImage(IRRDevice device, IRRGPUImage gpuImage, 
	irr::asset::E_FORMAT imageFormatOverride = irr::asset::EF_UNKNOWN)
{
	auto& gpuParams = gpuImage->getCreationParameters();

	irr::asset::IImageView<irr::video::IGPUImage>::SCreationParams gpuImageViewParams =
	{
		static_cast<irr::video::IGPUImageView::E_CREATE_FLAGS>(0),
		gpuImage,
		irr::asset::IImageView<irr::video::IGPUImage>::ET_2D,
		(imageFormatOverride != irr::asset::EF_UNKNOWN) ? imageFormatOverride : gpuParams.format,
		{},
			{
				static_cast<irr::asset::IImage::E_ASPECT_FLAGS>(0),
				0,
				gpuParams.mipLevels,
				0,
				gpuParams.arrayLayers
			}
	};

	auto gpuImageView = device->getVideoDriver()->createGPUImageView(std::move(gpuImageViewParams));

	assert(gpuImageView);

	return gpuImageView;
}

inline IRRGPUImagePair loadImage(IRRDevice device, const char* path, bool flipImage = false)
{
	auto assetManager = device->getAssetManager();
	auto driver = device->getVideoDriver();

	irr::asset::IAssetLoader::SAssetLoadParams loadingParams;
	auto images_bundle = assetManager->getAsset(path, loadingParams);

	assert(!images_bundle.isEmpty());

	auto image = irr::core::smart_refctd_ptr_dynamic_cast<irr::asset::ICPUImage>(images_bundle.getContents().begin()[0]);

	if (flipImage)
	{
		irr::asset::IImageAssetHandlerBase::performImageFlip(image);
	}

	auto image_raw = image.get();

	auto gpuImage = driver->getGPUObjectsFromAssets(&image_raw, &image_raw + 1)->front();

	assert(gpuImage);

	return { gpuImage, gpuImageViewFromGpuImage(device, gpuImage, SRGBFromatToUnorm(gpuImage->getCreationParameters().format)) };
}

inline IRRGPUComputePipeline createComputePipeline(IRRDevice device, const char* shaderPath,
	std::array<IRRGPUDescriptorSetLayout, 4> layouts, std::size_t pushConstantSize = 0)
{
	auto driver = device->getVideoDriver();

	irr::asset::SPushConstantRange range;
	range.offset = 0;
	range.size = pushConstantSize;
	range.stageFlags = irr::asset::ISpecializedShader::ESS_COMPUTE;
	auto layout = driver->createGPUPipelineLayout(&range, &range + 1,
		std::move(layouts[0]),
		std::move(layouts[1]),
		std::move(layouts[2]),
		std::move(layouts[3]));

	assert(layout);

	auto am = device->getAssetManager();

	irr::asset::IAssetLoader::SAssetLoadParams lp;
	auto cs_bundle = am->getAsset(shaderPath, lp);

	assert(!cs_bundle.isEmpty());

	auto cs = irr::core::smart_refctd_ptr_static_cast<irr::asset::ICPUSpecializedShader>(*cs_bundle.getContents().begin());

	assert(cs);

	auto cs_rawptr = cs.get();
	auto shader = driver->getGPUObjectsFromAssets(&cs_rawptr, &cs_rawptr + 1)->front();

	assert(shader);

	auto compPipeline = driver->createGPUComputePipeline(nullptr, std::move(layout), std::move(shader));

	assert(compPipeline);

	return compPipeline;
}

inline IRRGPUDescriptorSetPair createDescriptorSetForImage(IRRDevice device, IRRGPUImageView gpuImageView,
	uint32_t binding = 0,
	irr::video::IGPUSpecializedShader::E_SHADER_STAGE shader_stage = irr::video::IGPUSpecializedShader::ESS_COMPUTE)
{
	auto driver = device->getVideoDriver();

	irr::video::IGPUDescriptorSetLayout::SBinding gpuImageBinding;
	gpuImageBinding.binding = binding;
	gpuImageBinding.type = irr::asset::EDT_STORAGE_IMAGE;
	gpuImageBinding.count = 1;
	gpuImageBinding.stageFlags = shader_stage;
	gpuImageBinding.samplers = nullptr;

	auto gpu_ds0l = driver->createGPUDescriptorSetLayout(&gpuImageBinding, &gpuImageBinding + 1);

	assert(gpu_ds0l);

	auto gpu_ds0 = driver->createGPUDescriptorSet(gpu_ds0l);

	assert(gpu_ds0);

	irr::video::IGPUDescriptorSet::SWriteDescriptorSet write;
	write.dstSet = gpu_ds0.get();
	write.binding = binding;
	write.count = 1;
	write.arrayElement = 0;
	write.descriptorType = irr::asset::EDT_STORAGE_IMAGE;

	irr::video::IGPUDescriptorSet::SDescriptorInfo info = {};
	info.desc = std::move(gpuImageView);

	write.info = &info;
	driver->updateDescriptorSets(1, &write, 0, nullptr);

	assert(gpu_ds0);

	return { gpu_ds0, gpu_ds0l };
}

inline IRRGPUImagePair createGpuImagePair(IRRDevice device, irr::asset::VkExtent3D resolution,
	irr::asset::E_FORMAT imageFormat = irr::asset::EF_R32G32B32A32_SFLOAT)
{
	auto driver = device->getVideoDriver();

	irr::video::IGPUImage::SCreationParams params = {};

	params.arrayLayers = 1;
	params.extent = resolution;
	params.format = imageFormat;
	params.mipLevels = 1;
	params.samples = irr::asset::IImage::E_SAMPLE_COUNT_FLAGS::ESCF_1_BIT;
	params.type = irr::asset::IImage::E_TYPE::ET_2D;

	auto gpuImage = driver->createDeviceLocalGPUImageOnDedMem(std::move(params));

	assert(gpuImage);

	return { gpuImage, gpuImageViewFromGpuImage(device, gpuImage) };
}

inline void invokeComputeShader(IRRDevice device, IRRGPUComputePipeline compPipeline,
	std::array<IRRGPUDescriptorSet, 4> descriptorSets,
	uint32_t workGroupsX = 1, uint32_t workGroupsY = 1, uint32_t workGroupsZ = 1,
	uint32_t pushConstantSize = 0, void* pushConstantData = nullptr,
	bool blocking = true)
{
	auto driver = device->getVideoDriver();

	driver->bindComputePipeline(compPipeline.get());

	if (pushConstantData)
		driver->pushConstants(compPipeline->getLayout(), irr::asset::ISpecializedShader::ESS_COMPUTE, 0, pushConstantSize, pushConstantData);

	uint32_t noOfDescriptors = 0;
	std::array<irr::video::IGPUDescriptorSet*, 4> descriptorsRaw = {};

	for (int i = 0; i < descriptorSets.size(); i++)
	{
		if (descriptorSets[i])
		{
			++noOfDescriptors;
			descriptorsRaw[i] = descriptorSets[i].get();
		}
		else break;
	}

	driver->bindDescriptorSets(irr::video::EPBP_COMPUTE, compPipeline->getLayout(), 0, noOfDescriptors, descriptorsRaw.data(), nullptr);

	driver->dispatch(workGroupsX, workGroupsY, workGroupsZ);

	if (blocking)
	{
		irr::video::COpenGLExtensionHandler::extGlMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
	}
}

struct ShaderConfiguration
{
	uint32_t workgroup_size_x = 1;
	uint32_t workgroup_size_y = 1;
	uint32_t workgroup_size_z = 1;
	uint32_t shared_buffer_size = 0;
	std::string input_image_format1;
	std::string input_image_format2;

	std::string output_image_format1;
	std::string output_image_format2;

	std::string serialize() const
	{
		std::ostringstream oss;
		oss << "#define WORKGROUP_SIZE_X " << workgroup_size_x << '\n';
		oss << "#define WORKGROUP_SIZE_Y " << workgroup_size_y << '\n';
		oss << "#define WORKGROUP_SIZE_Z " << workgroup_size_z << '\n';
		oss << "#define SHARED_BUFFER_SIZE " << shared_buffer_size << '\n';

		oss << "#define INPUT_IMAGE_FORMAT1 " << input_image_format1 << '\n';
		oss << "#define INPUT_IMAGE_FORMAT2 " << input_image_format2 << '\n';

		oss << "#define OUTPUT_IMAGE_FORMAT1 " << output_image_format1 << '\n';
		oss << "#define OUTPUT_IMAGE_FORMAT2 " << output_image_format2 << '\n';

		return oss.str();
	}
};

const char* imageFromatToFormatString(IRRGPUImageView imageView)
{
	using namespace irr::asset;

	switch (imageView->getCreationParameters().format)
	{
	case EF_R8_SRGB:
	case EF_R8_UNORM:
		return "r8";
	case EF_R8G8_SRGB:
	case EF_R8G8_UNORM:
		return "rg8";
	case EF_R8G8B8_SRGB:
	case EF_R8G8B8_UNORM:
		return "rgb8";
	case EF_R8G8B8A8_SRGB:
	case EF_R8G8B8A8_UNORM:
		return "rgba8";
	case EF_R32_SFLOAT:
		return "r32f";
	case EF_R32G32_SFLOAT:
		return "rg32f";
	case EF_R32G32B32_SFLOAT:
		return "rgb32f";
	case EF_R32G32B32A32_SFLOAT:
		return "rgba32f";
	default:
		return "";
	}
}

inline bool writeShaderConfigurationToFile(const ShaderConfiguration& configuration, const char* path)
{
	std::ofstream file(path);
	if (!file) return false;

	file << configuration.serialize();
	file.close();

	return true;
}

struct FFT2DPushConstant
{
	int input_width;
	int input_height;
	int output_width;
	int output_height;
	int logtwo_width;
	int logtwo_height;
	int clz_width;
	int clz_height;
	int channel_no;
	int stage;
};

struct ConvolutionPushConstant
{
	int original_width;
	int original_height;
	int kernel_width;
	int kernel_height;
};

inline uint32_t fft2d_clz(uint32_t x)
{
	static constexpr uint32_t lut[32] = {
		0, 31, 9, 30, 3, 8, 13, 29, 2, 5, 7, 21, 12, 24, 28, 19,
		1, 10, 4, 14, 6, 22, 25, 20, 11, 15, 23, 26, 16, 27, 17, 18
	};

	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	x++;

	return lut[x * 0x076be629U >> 27U];
}
