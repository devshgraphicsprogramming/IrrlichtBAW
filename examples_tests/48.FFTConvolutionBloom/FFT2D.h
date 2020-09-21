#pragma once
#include "FFTConvolutionBloomFunctions.h"
#include <algorithm>

class FFT2D
{
public:

	FFT2D() = delete;

	FFT2D(const FFT2D&) = delete;
	FFT2D& operator=(const FFT2D&) = delete;

	FFT2D(FFT2D&&) = default;
	FFT2D& operator=(FFT2D&&) = default;

	FFT2D(IRRDevice device, IRRGPUImagePair inputImage)
		: device(device), inputImage(inputImage)
	{
		auto inputRes = getImageDimensions(inputImage.gpuImage);
		auto bufferRes = decltype(inputRes){irr::core::roundUpToPoT(inputRes.width), irr::core::roundUpToPoT(inputRes.height), 1};
	
		workgroupX = bufferRes.width;
		workgroupY = bufferRes.height;

		inputImageDescriptorSet = createDescriptorSetForImage(device, inputImage.gpuImageView);

		auto tempBufferFormat = UNORMFormatToSFLOAT(inputImage.gpuImageView->getCreationParameters().format);

		realPart = createGpuImagePair(device, bufferRes, tempBufferFormat);

		realPartDescriptorSet = createDescriptorSetForImage(device, realPart.gpuImageView);

		imaginaryPart = createGpuImagePair(device, bufferRes, tempBufferFormat);

		imaginaryPartDescriptorSet = createDescriptorSetForImage(device, imaginaryPart.gpuImageView);

		ShaderConfiguration config;

		config.workgroup_size_x = 256;
		config.shared_buffer_size = std::max(bufferRes.width, bufferRes.height);
		config.input_image_format1 = imageFromatToFormatString(inputImage.gpuImageView);
		config.output_image_format1 = imageFromatToFormatString(realPart.gpuImageView);
		config.output_image_format2 = config.output_image_format1;

		writeShaderConfigurationToFile(config, fft2d_shader_config_path);

		computePipeline = createComputePipeline(
			device, 
			fft2d_shader_path,
			{ 
				inputImageDescriptorSet.gpuDescriptorSetLayout, 
				realPartDescriptorSet.gpuDescriptorSetLayout,
				imaginaryPartDescriptorSet.gpuDescriptorSetLayout 
			},
			sizeof(FFT2DPushConstant)
		);

		std::remove(fft2d_shader_config_path);

		pushconst.channel_no = std::min(getFormatChannelCount(inputImage.gpuImageView->getCreationParameters().format), 
			getFormatChannelCount(realPart.gpuImageView->getCreationParameters().format));

		pushconst.input_width = inputRes.width;
		pushconst.input_height = inputRes.height;
		pushconst.output_width = bufferRes.width;
		pushconst.output_height = bufferRes.height;
		pushconst.clz_width = fft2d_clz(pushconst.output_width) + 1;
		pushconst.clz_height = fft2d_clz(pushconst.output_height) + 1;
		pushconst.logtwo_width = 32 - pushconst.clz_width;
		pushconst.logtwo_height = 32 - pushconst.clz_height;
	}

	void perfromFFT()
	{
		pushconst.stage = 0;

		invokeComputeShader(
			device,
			computePipeline,
			{
				inputImageDescriptorSet.gpuDescriptorSet,
				realPartDescriptorSet.gpuDescriptorSet,
				imaginaryPartDescriptorSet.gpuDescriptorSet
			},
			workgroupX, 1, 1,
			sizeof(FFT2DPushConstant),
			&pushconst);

		//return;

		pushconst.stage = 1;

		invokeComputeShader(
			device,
			computePipeline,
			{
				inputImageDescriptorSet.gpuDescriptorSet,
				realPartDescriptorSet.gpuDescriptorSet,
				imaginaryPartDescriptorSet.gpuDescriptorSet
			},
			workgroupY, 1, 1,
			sizeof(FFT2DPushConstant),
			&pushconst);
	}

	void perfromIFFT()
	{
		pushconst.stage = 2;

		invokeComputeShader(
			device,
			computePipeline,
			{
				inputImageDescriptorSet.gpuDescriptorSet,
				realPartDescriptorSet.gpuDescriptorSet,
				imaginaryPartDescriptorSet.gpuDescriptorSet
			},
			workgroupY, 1, 1,
			sizeof(FFT2DPushConstant),
			&pushconst);
	
		pushconst.stage = 3;

		invokeComputeShader(
			device,
			computePipeline,
			{
				inputImageDescriptorSet.gpuDescriptorSet,
				realPartDescriptorSet.gpuDescriptorSet,
				imaginaryPartDescriptorSet.gpuDescriptorSet
			},
			workgroupX, 1, 1,
			sizeof(FFT2DPushConstant),
			&pushconst);
	}

	void multiply(FFT2D& kernel)
	{
		assert(device == kernel.device);

		auto realBufferRes = getImageDimensions(realPart.gpuImage);

		ShaderConfiguration config = {};

		config.input_image_format1 = imageFromatToFormatString(realPart.gpuImageView);
		config.input_image_format2 = imageFromatToFormatString(imaginaryPart.gpuImageView);
		config.output_image_format1 = imageFromatToFormatString(kernel.realPart.gpuImageView);
		config.output_image_format2 = imageFromatToFormatString(kernel.imaginaryPart.gpuImageView);

		writeShaderConfigurationToFile(config, convolution_shader_config_path);

		auto convolutionPipeline = createComputePipeline(
			device,
			convolution_shader_path,
			{
				realPartDescriptorSet.gpuDescriptorSetLayout,
				imaginaryPartDescriptorSet.gpuDescriptorSetLayout,
				kernel.realPartDescriptorSet.gpuDescriptorSetLayout,
				kernel.imaginaryPartDescriptorSet.gpuDescriptorSetLayout
			},
			sizeof(ConvolutionPushConstant)
		);

		std::remove(convolution_shader_config_path);

		ConvolutionPushConstant convolutionPushConst;

		convolutionPushConst.original_width = realBufferRes.width;
		convolutionPushConst.original_height = realBufferRes.height;

		auto kernelRes = getImageDimensions(kernel.realPart.gpuImage);

		convolutionPushConst.kernel_width = kernelRes.width;
		convolutionPushConst.kernel_height = kernelRes.height;

		invokeComputeShader(
			device,
			convolutionPipeline,
			{
				realPartDescriptorSet.gpuDescriptorSet,
				imaginaryPartDescriptorSet.gpuDescriptorSet,
				kernel.realPartDescriptorSet.gpuDescriptorSet,
				kernel.imaginaryPartDescriptorSet.gpuDescriptorSet
			},
			realBufferRes.width, realBufferRes.height, 1,
			sizeof(ConvolutionPushConstant),
			&convolutionPushConst);

	}

	IRRGPUImagePair getInputImage() const { return inputImage; }
	IRRGPUImagePair getRealPart() const { return realPart; }
	IRRGPUImagePair getImaginaryPart() const { return imaginaryPart; }

private:
	IRRDevice device;

	IRRGPUImagePair inputImage;
	IRRGPUDescriptorSetPair inputImageDescriptorSet;

	IRRGPUImagePair realPart;
	IRRGPUDescriptorSetPair realPartDescriptorSet;

	IRRGPUImagePair imaginaryPart;
	IRRGPUDescriptorSetPair imaginaryPartDescriptorSet;

	FFT2DPushConstant pushconst;

	IRRGPUComputePipeline computePipeline;

	std::size_t workgroupX;
	std::size_t workgroupY;

	static constexpr const char* fft2d_shader_path = "../shaders/FFT2D.comp";
	static constexpr const char* fft2d_shader_config_path = "../shaders/FFT2DConfig.h";

	static constexpr const char* convolution_shader_path = "../shaders/Convolution2D.comp";
	static constexpr const char* convolution_shader_config_path = "../shaders/Convolution2DConfig.h";
};
