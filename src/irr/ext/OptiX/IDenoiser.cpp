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
    CUdeviceptr inputBuffer,
    CUdeviceptr outputBuffer,
    size_t                inputWidth,
    size_t                inputHeight,
    OptixPixelFormat   pixelFormat,
    size_t                overlap,
    size_t                tileWidth,
    size_t                tileHeight,
    std::vector<Tile>& tiles)
{
    int pixelSize = get_pixel_size(pixelFormat);
    int rowStride = inputWidth * pixelSize;

    int  pos_y= 0;
    do {
        int inputOffsetY = pos_y==0? 0 : overlap;
        auto avaible_height = inputHeight - pos_y;
        int actualInputTileHeight = std::min(avaible_height, overlap + tileHeight) + inputOffsetY;

        int pos_x = 0;
        do 
        {
            int inputOffsetX = pos_x == 0 ? 0 : overlap;
            auto avaible_width = inputWidth - pos_x;
            int actualInputTileWidth = std::min(avaible_width, overlap + tileWidth) + inputOffsetX;

            Tile tile{};
            {
                auto in_posx = pos_x - inputOffsetX;
                auto in_posy = pos_y - inputOffsetY;

                tile.input.data = inputBuffer
                        + in_posy * rowStride
                        + in_posx * pixelSize;
                tile.input.width = actualInputTileWidth;
                tile.input.height = actualInputTileHeight;
                tile.input.rowStrideInBytes = rowStride;
                tile.input.format = pixelFormat;
                tile.output.data = outputBuffer
                        + pos_y * rowStride
                        + pos_x * pixelSize;
                tile.output.width = std::min(avaible_width, tileWidth);
                tile.output.height = std::min(avaible_height, tileHeight);
                tile.output.rowStrideInBytes = rowStride;
                tile.output.format = pixelFormat;
                tile.inputOffsetX = inputOffsetX;
                tile.inputOffsetY = inputOffsetY;
                tiles.push_back(tile);
            }
           /* input_x += input_x == 0 ? tileWidth + overlap : tileWidth;
            copied_x += copy_x;*/
            pos_x += tileWidth;
        } while (pos_x < inputWidth);
       /* input_y += input_y == 0 ? tileHeight + overlap : tileHeight;
        copied_y += copy_y;*/
        pos_y += tileHeight;
        pos_x = 0;
    } while (pos_y < inputHeight);    
}