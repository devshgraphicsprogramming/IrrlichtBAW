#ifndef __IRR_BAW_MESH_WRITER_H_INCLUDED__
#define __IRR_BAW_MESH_WRITER_H_INCLUDED__

#include "IMeshWriter.h"

/*
Format design:
--------------
submeshes_cnt: int32
submeshes_offsets: int32
submeshes:
	def submesh:
		v_cnt, vt_cnt, vn_cnt, fc_cnt : int32
		offsets v, vt, vn, idx : int32 #offsets relative to local submesh
		blocks v(float3), vt(float2), vn(INT_2_10_10_10_REV/float3), idx(uint8/uint16)
#todo materials
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