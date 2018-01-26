#include "CBAWMeshWriter.h"

#include <utility>

#include "IFileSystem.h"
#include "IWriteFile.h"
#include "irrArray.h"
#include "IMesh.h"
#include "SVertexManipulator.h"
#include "ITexture.h"

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
		const uint32_t materialWithoutTexturesByteSize = 4 + 4*4 + 4 + 2*4 + 4 + 1 + 1 + 1 + 4 + 4 + 1;
		const uint32_t materialTextureWithoutStringByteSize = 4 + 4;
		const uint32_t headerByteSize = (1 + _mesh->getMeshBufferCount()) * (32/8); // mesh_cnt + mesh_cnt*mesh_offset
		core::array<uint32_t> header;
		header.reallocate(headerByteSize/(32/8));
		header.push_back(_mesh->getMeshBufferCount());
		header.push_back(headerByteSize); // offset of 0 submesh

		// TODO: make place in file for header which will be written in the end of the function

		// prepare data of every submesh
		for (uint32_t mbi = 0u; mbi < _mesh->getMeshBufferCount(); ++mbi)
		{
			const ICPUMeshBuffer* meshBuf = _mesh->getMeshBuffer(mbi);
			const IMeshDataFormatDesc<core::ICPUBuffer>* desc = meshBuf->getMeshDataAndFormat();
			const uint8_t indexSize = (meshBuf->getIndexType() == video::EIT_16BIT ? 2 : 4);
			core::array<core::vector3df> pos;
			core::array<core::vector2df> uvs;
			core::array<int32_t> normals; // INT_2_10_10_10_REV format
			const std::pair<void *, size_t> indices = // .first: ptr to indices; .second: size of indices in bytes
				std::make_pair(
					(uint8_t *)desc->getIndexBuffer()->getPointer() + meshBuf->getIndexBufferOffset(),
					meshBuf->getIndexCount() * indexSize
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
				normals.push_back(quantizeNormal2_10_10_10(v));
			}

			const uint32_t cnt[4]{ pos.size(), uvs.size(), normals.size(), meshBuf->getIndexCount() };
			const uint32_t v_offset = 4*sizeof(*cnt),
				uv_offset = v_offset + cnt[0] * sizeof(pos[0]),
				vn_offset = uv_offset + cnt[1] * sizeof(uvs[0]),
				idx_offset = vn_offset + cnt[2] * sizeof(normals[0]),
				mat_offset = idx_offset + indices.second;
			const uint32_t offsets[5]{ v_offset, uv_offset, vn_offset, idx_offset, mat_offset }; // submesh-local offsets

			const video::SMaterial & mat = meshBuf->getMaterial();
			const uint32_t matColor[4] {
				mat.AmbientColor.color,
				mat.DiffuseColor.color,
				mat.EmissiveColor.color,
				mat.SpecularColor.color
			};
			uint8_t matFlags = 0u;
			matFlags |= (mat.Wireframe << 0);
			matFlags |= (mat.PointCloud << 1);
			matFlags |= (mat.ZWriteEnable << 2);
			matFlags |= (mat.BackfaceCulling << 3);
			matFlags |= (mat.FrontfaceCulling << 4);
			matFlags |= (mat.RasterizerDiscard << 5);

			_file->write(cnt, sizeof(cnt));
			_file->write(offsets, sizeof(offsets));

			_file->write(pos.const_pointer(), pos.size() * sizeof(pos[0]));
			_file->write(uvs.const_pointer(), uvs.size() * sizeof(uvs[0]));
			_file->write(normals.const_pointer(), normals.size() * sizeof(normals[0]));
			_file->write(indices.first, indices.second);
			_file->write(&indexSize, 1);

			_file->write(&mat.MaterialType, sizeof(video::E_MATERIAL_TYPE));
			_file->write(matColor, sizeof(matColor));
			_file->write(&mat.Shininess, 4);
			_file->write(&mat.MaterialTypeParam, 4);
			_file->write(&mat.MaterialTypeParam2, 4);
			_file->write(&mat.Thickness, 4);
			_file->write(&mat.ZBuffer, 1);
			const uint8_t colorMask = mat.ColorMask;
			_file->write(&colorMask, 1);
			const uint8_t blendOp = mat.BlendOperation;
			_file->write(&blendOp, 1);
			_file->write(&mat.PolygonOffsetConstantMultiplier, 4);
			_file->write(&mat.PolygonOffsetGradientMultiplier, 4);
			_file->write(&matFlags, 1);

			uint32_t submeshOffset = header.getLast();
			submeshOffset += materialWithoutTexturesByteSize;

			for (int i = 0; i < video::MATERIAL_MAX_TEXTURES; ++i) {
				io::path path;
				if (video::ITexture * texture = dynamic_cast<video::ITexture *>(mat.getTexture(i))) {
					path = texture->getName().getInternalName();
					const size_t len = std::strlen(path.c_str()) + 1;
					_file->write(path.c_str(), len);
					submeshOffset += len;
				}
				else {
					int8_t zero = 0;
					_file->write(&zero, 1);
					++submeshOffset;
				}
				uint32_t params = 0u;
				params |= mat.TextureLayer[i].SamplingParams.SeamlessCubeMap;
				(params <<= 5) |= mat.TextureLayer[i].SamplingParams.AnisotropicFilter;
				(params <<= 1) |= mat.TextureLayer[i].SamplingParams.UseMipmaps;
				(params <<= 1) |= mat.TextureLayer[i].SamplingParams.MaxFilter;
				(params <<= 3) |= mat.TextureLayer[i].SamplingParams.MinFilter;
				(params <<= 3) |= mat.TextureLayer[i].SamplingParams.TextureWrapW;
				(params <<= 3) |= mat.TextureLayer[i].SamplingParams.TextureWrapV;
				(params <<= 3) |= mat.TextureLayer[i].SamplingParams.TextureWrapU;

				_file->write(&params, 4);
				_file->write(&mat.TextureLayer[i].SamplingParams.LODBias, 4);

				submeshOffset += materialTextureWithoutStringByteSize;
			}

			if (header.size() < headerByteSize/(32 / 8))
				header.push_back(submeshOffset);
		}
		// TODO write header on position 0 in file (previosly reserved place)
	}

}} // end ns irr::scene