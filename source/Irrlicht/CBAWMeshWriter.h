#ifndef __IRR_BAW_MESH_WRITER_H_INCLUDED__
#define __IRR_BAW_MESH_WRITER_H_INCLUDED__

#include "IMeshWriter.h"

/*
Format design:
--------------
submeshes_cnt: uint32
submeshes_offsets : uint32[submeshes_cnt]
def submesh:
	v_cnt, vt_cnt, vn_cnt, fc_cnt : 4*uint32
	offsets v, vt, vn, idx, mat : 5*uint32 # offsets relative to local submesh
	blocks v(float3), vt(float2), vn(int32, INT_2_10_10_10_REV), idx(uint16|uint32)
	index_size : uint8 # must be 2 or 4
	def material
		type : int32
		colors : int32[4] #order: ambient, diffuse, emissive, specular (each color represented by 32bits)
		shininess : float
		params : float[2]
		thickness : float
		zbuffer : uint8
		color_mask : uint8
		blend_op : uint8
		polygon_offset_const_mltr : float
		polygon_offset_gradient_mltr : float
		flags : uint8 (6 bits, from lsb: wireframe, pointcloud, zwrite_enable, backface_culling, frontface_culling, rasterizer_discard)
		def texture:
			path : cstring (0-byte terminated)
			params : uint32 (20 bits, from lsb: wrapu:3, wrapv:3, wrapw:3, minfltr:3, maxfltr:1, use_mm:1, anisofltr:5, seamlesscubemap:1)
			lod_bias : float
		textures : texture[_IRR_MATERIAL_MAX_TEXTURES_]
	mat : material
submeshes : submesh[submeshes_cnt]
*/

namespace irr { 

namespace io { class IFileSystem; }

namespace scene
{

	class CBAWMeshWriter :
		public IMeshWriter
	{
	protected:
		~CBAWMeshWriter();
	public:
		CBAWMeshWriter(io::IFileSystem * _fs);

		EMESH_WRITER_TYPE getType() const override { return EMWT_BAW; }

		bool writeMesh(io::IWriteFile* file, scene::ICPUMesh* mesh, int32_t flags = EMWF_NONE) override;

	private:
		io::IFileSystem* m_fileSystem;
	};

}} // end of ns irr:scene

#endif