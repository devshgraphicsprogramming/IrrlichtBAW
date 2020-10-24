#include "irr/ext/OptiX/IDenoiser.h"

using namespace irr;
using namespace ext;
using namespace OptiX;

unsigned int get_pixel_size(OptixPixelFormat pixelFormat)
{
    switch (pixelFormat) {
    case OPTIX_PIXEL_FORMAT_HALF3:
        return 3 * sizeof(unsigned short);
    case OPTIX_PIXEL_FORMAT_HALF4:
        return 4 * sizeof(unsigned short);
    case OPTIX_PIXEL_FORMAT_FLOAT3:
        return 3 * sizeof(float);
    case OPTIX_PIXEL_FORMAT_FLOAT4:
        return 4 * sizeof(float);
    default:
        return OPTIX_ERROR_INVALID_VALUE;
    }
}
void IDenoiser::createTilesForDenoising(
    void* inputBuffer,
    void* outputBuffer,
    size_t                inputWidth,
    size_t                inputHeight,
    OptixPixelFormat   pixelFormat,
    size_t                overlap,
    size_t                tileWidth,
    size_t                tileHeight,
    std::vector<Tile>& tiles)
{
    uint32_t pixelSize = get_pixel_size(pixelFormat);
    uint32_t rowStride = inputWidth * pixelSize;

    int input_width = std::min(tileWidth + 2 * overlap, inputWidth);
    int input_height = std::min(tileHeight + 2 * overlap, inputHeight);

    int input_y = 0, copied_y = 0;
    do {
        int inputOffsetY = 0;
        int copy_y = tileHeight + overlap;

        if (input_y != 0) {
            inputOffsetY =
                std::max(overlap,
                    input_height - (inputHeight - input_y));
            copy_y = std::min(tileHeight, inputHeight - copied_y);
        }
        int input_x = 0, copied_x = 0;
        do {
            int inputOffsetX = 0;
            int copy_x = tileWidth + overlap;
            if (input_x != 0) {
                inputOffsetX =
                    std::max(overlap,
                        input_width - (inputWidth - input_x));
                copy_x = std::min(tileWidth, inputWidth - copied_x);
            }

            Tile tile{};
            {
                tile.input.data =
                    (CUdeviceptr)(
                        (char*)inputBuffer
                        + (input_y - inputOffsetY) * rowStride
                        + (input_x - inputOffsetX) * pixelSize);
                tile.input.width = input_width;
                tile.input.height = input_height;
                tile.input.rowStrideInBytes = rowStride;
                tile.input.format = pixelFormat;
                tile.output.data =
                    (CUdeviceptr)(
                        (char*)outputBuffer
                        + input_y * rowStride
                        + input_x * pixelSize);
                tile.output.width = copy_x;
                tile.output.height = copy_y;
                tile.output.rowStrideInBytes = rowStride;
                tile.output.format = pixelFormat;
                tile.inputOffsetX = inputOffsetX;
                tile.inputOffsetY = inputOffsetY;
                tiles.push_back(tile);
            }
            input_x += input_x == 0 ? tileWidth + overlap : tileWidth;
            copied_x += copy_x;

        } while (input_x < inputWidth);
        input_y += input_y == 0 ? tileHeight + overlap : tileHeight;
        copied_y += copy_y;
    } while (input_y < inputHeight);    
}