#ifndef _IRR_ASSET_C_GLSL_VIRTUAL_TEXTURING_BUILTIN_INCLUDE_LOADER_H_INCLUDED__
#define _IRR_ASSET_C_GLSL_VIRTUAL_TEXTURING_BUILTIN_INCLUDE_LOADER_H_INCLUDED__

#include "irr/asset/IGLSLEmbeddedIncludeLoader.h"

#include "irr/asset/CGraphicsPipelineLoaderMTL.h"
#include "irr/asset/ICPUVirtualTexture.h"


namespace irr
{
namespace asset
{

class CGLSLVirtualTexturingBuiltinIncludeLoader : public IGLSLEmbeddedIncludeLoader
{
    public:
        const char* getVirtualDirectoryName() const override { return "glsl/virtual_texturing/"; }

    private:
	    static core::vector<std::string> parseArgumentsFromPath(const std::string& _path)
	    {
		    core::vector<std::string> args;

		    std::stringstream ss{ _path };
		    std::string arg;
		    while (std::getline(ss, arg, '/'))
			    args.push_back(std::move(arg));

		    return args;
	    }

		static std::string getVTfunctions(const std::string& _path)
		{
			auto args = parseArgumentsFromPath(_path.substr(_path.find_first_of('/')+1, _path.npos));
			if (args.size()<2u)
				return {};

			constexpr uint32_t
				ix_pg_sz_log2 = 0u,
				ix_tile_padding = 1u;

			const uint32_t pg_sz_log2 = std::atoi(args[ix_pg_sz_log2].c_str());
			const uint32_t tile_padding = std::atoi(args[ix_tile_padding].c_str());

			ICPUVirtualTexture::SMiptailPacker::rect tilePacking[ICPUVirtualTexture::MAX_PHYSICAL_PAGE_SIZE_LOG2];
			//this could be cached..
			ICPUVirtualTexture::SMiptailPacker::computeMiptailOffsets(tilePacking, pg_sz_log2, tile_padding);

			auto tilePackingOffsetsStr = [&] {
				std::string offsets;
				for (uint32_t i = 0u; i < pg_sz_log2; ++i)
					offsets += "vec2(" + std::to_string(tilePacking[i].x+tile_padding) + "," + std::to_string(tilePacking[i].y+tile_padding) + ")" + (i == (pg_sz_log2 - 1u) ? "" : ",");
				return offsets;
			};

			using namespace std::string_literals;
			std::string s = R"(
#ifndef _IRR_BUILTIN_GLSL_VIRTUAL_TEXTURING_FUNCTIONS_INCLUDED_
#define _IRR_BUILTIN_GLSL_VIRTUAL_TEXTURING_FUNCTIONS_INCLUDED_
)";
			s += "\n\n#define PAGE_SZ " + std::to_string(1u<<pg_sz_log2) + "u" +
				"\n#define PAGE_SZ_LOG2 " + args[ix_pg_sz_log2] + "u" +
				"\n#define TILE_PADDING " + args[ix_tile_padding] + "u" +
				"\n#define PADDED_TILE_SIZE uint(PAGE_SZ+2*TILE_PADDING)" +
				"\n\nconst vec2 packingOffsets[] = vec2[PAGE_SZ_LOG2+1]( vec2(" + std::to_string(tile_padding) + ")," + tilePackingOffsetsStr() + ");";
			s += s = R"(
#include "irr/builtin/glsl/virtual_texturing/impl_functions.glsl"

#endif
)";
			return s;
		}

	protected:
		core::vector<std::pair<std::regex, HandleFunc_t>> getBuiltinNamesToFunctionMapping() const override
		{
			auto retval = IGLSLEmbeddedIncludeLoader::getBuiltinNamesToFunctionMapping();

			const std::string num = "[0-9]+";
			retval.insert(retval.begin(),
				{ 
					std::regex{"functions\\.glsl/"+num+"/"+num},
					&getVTfunctions
				}
			);
			return retval;
		}
};

}}
#endif