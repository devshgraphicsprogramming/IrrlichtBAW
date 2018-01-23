#include "CBAWMeshWriter.h"

#include <utility>

#include "IFileSystem.h"
#include "IWriteFile.h"
#include "irrArray.h"
#include "IMesh.h"

namespace irr { namespace scene 
{

	CBAWMeshWriter::~CBAWMeshWriter()
	{
		if (m_fileSystem)
			m_fileSystem->drop();
	}

	CBAWMeshWriter::CBAWMeshWriter(io::IFileSystem * _fs) : m_fileSystem(_fs)
	{
#ifdef _DEBUG
		setDebugName("CBAWMeshWriter");
#endif
		if (m_fileSystem)
			m_fileSystem->grab();
	}

	bool CBAWMeshWriter::writeMesh(io::IWriteFile * _file, scene::ICPUMesh * _mesh, int32_t _flags)
	{
		//! @todo refactor (divide into few functions) writeMesh()
		const uint32_t headerByteSize = (1 + _mesh->getMeshBufferCount()) * (32/8); // mesh_cnt + mesh_cnt*mesh_offset
		core::array<int32_t> header;
		header.reallocate(headerByteSize/(32/8));
		header.push_back(_mesh->getMeshBufferCount());
		header.push_back(headerByteSize); // offset of 0 submesh

		for (uint32_t mbi = 0u; mbi < _mesh->getMeshBufferCount() - 1; ++mbi) // offsets of the rest of submeshes
		{
			const ICPUMeshBuffer* meshBuf = _mesh->getMeshBuffer(mbi);
			const IMeshDataFormatDesc<core::ICPUBuffer>* desc = meshBuf->getMeshDataAndFormat();
			
			const core::ICPUBuffer *pbuf = desc->getMappedBuffer(meshBuf->getPositionAttributeIx()),
				*tbuf = desc->getMappedBuffer(meshBuf->getTcoordAttributeIx()),
				*nbuf = desc->getMappedBuffer(meshBuf->getNormalAttributeIx());
			uint32_t submeshOffset = header[header.size() - 1] + pbuf->getSize();
			if (tbuf != pbuf)
				submeshOffset += tbuf->getSize();
			if (nbuf != tbuf && nbuf != pbuf)
				submeshOffset += nbuf->getSize();
			submeshOffset += meshBuf->getIndexCount() * (meshBuf->getIndexType() == video::EIT_16BIT ? 2 : 4);
			header.push_back(submeshOffset);
		}
		// end of header
		_file->write(header.const_pointer(), header.size() * sizeof(*header.pointer()));

		// prepare data of every submesh
		for (uint32_t mbi = 0u; mbi < _mesh->getMeshBufferCount(); ++mbi)
		{
			const ICPUMeshBuffer* meshBuf = _mesh->getMeshBuffer(mbi);
			const IMeshDataFormatDesc<core::ICPUBuffer>* desc = meshBuf->getMeshDataAndFormat();
			core::array<core::vector3df> pos;
			core::array<core::vector2df> uvs;
			core::array<core::vector4df_SIMD> normals; // or <int32_t>? INT_2_10_10_10_REV
			const std::pair<void *, size_t> indices = // .first: ptr to indices; .second: size of indices in bytes
				std::make_pair(
					(uint8_t *)desc->getIndexBuffer()->getPointer() + meshBuf->getIndexBufferOffset(),
					meshBuf->getIndexCount() * (meshBuf->getIndexType() == video::EIT_16BIT ? 2 : 4)
			);

			size_t ix = 0u;
			while (1) // fill pos array
			{
				core::vectorSIMDf v;
				if (meshBuf->getAttribute(v, meshBuf->getPositionAttributeIx(), ix++)) // if `ix` is out of buffer range
					// @todo debug: check whether `ix` was surely out of range and print some log if it was within (means some other error occured and `false` shall be returned)
					break;
				pos.push_back(v.getAsVector3df());
			}
			ix = 0u;
			while (1) // fill uvs array
			{
				core::vectorSIMDf v;
				if (meshBuf->getAttribute(v, meshBuf->getTcoordAttributeIx(), ix++)) // if `ix` is out of buffer range
					break;
				uvs.push_back(v.getAsVector2df());
			}
			ix = 0u;
			while (1) // fill normals array
			{
				core::vectorSIMDf v;
				if (meshBuf->getAttribute(v, meshBuf->getNormalAttributeIx(), ix++)) // if `ix` is out of buffer range
					break;
				normals.push_back(v);
			}

			const uint32_t cnt[4]{ pos.size(), uvs.size(), normals.size(), meshBuf->getIndexCount() };
			const uint32_t v_offset = 4*sizeof(*cnt),
				uv_offset = v_offset + cnt[0] * sizeof(pos[0]),
				vn_offset = uv_offset + cnt[1] * sizeof(uvs[0]),
				idx_offset = vn_offset + cnt[2] * sizeof(normals);
			const uint32_t offsets[4]{ v_offset, uv_offset, vn_offset, idx_offset }; // submesh-local offsets

			const video::SMaterial & mat = meshBuf->getMaterial();
			uint8_t matFlags = 0u;
			matFlags |= mat.Wireframe << 0;
			matFlags |= mat.PointCloud << 1;
			matFlags |= mat.ZWriteEnable << 2;
			matFlags |= mat.BackfaceCulling << 3;
			matFlags |= mat.FrontfaceCulling << 4;
			matFlags |= mat.RasterizerDiscard << 5;
			const uint32_t matColor[4] {
				mat.AmbientColor.color,
				mat.DiffuseColor.color,
				mat.EmissiveColor.color,
				mat.SpecularColor.color
			};

			_file->write(cnt, sizeof(cnt));
			_file->write(offsets, sizeof(offsets));

			_file->write(pos.const_pointer(), pos.size() * sizeof(pos[0]));
			_file->write(uvs.const_pointer(), uvs.size() * sizeof(uvs[0]));
			_file->write(normals.const_pointer(), normals.size() * sizeof(normals[0]));
			_file->write(indices.first, indices.second);

			// get material texture mat.getTexture(n), dynamic_cast it to ITexture, .getName() and possibly use NamedPath::PathToName on that name (will get path object)
		}
	}

}} // end ns irr::scene