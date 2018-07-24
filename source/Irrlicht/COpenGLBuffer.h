#ifndef __C_OPEN_GL_BUFFER_H_INCLUDED__
#define __C_OPEN_GL_BUFFER_H_INCLUDED__

#include "IGPUBuffer.h"
#include "IrrCompileConfig.h"
#include "FW_Mutex.h"

#ifdef _IRR_COMPILE_WITH_OPENGL_
#include "CNullDriver.h"
#include "COpenGLExtensionHandler.h"
#include <assert.h>

namespace irr
{
namespace video
{



//! get the amount of Bits per Pixel of the given color format
inline uint32_t getBitsPerPixelFromGLenum(const GLenum& format)
{
    switch(format)
    {
        case GL_R8:
        case GL_R8I:
        case GL_R8UI:
            return 8;
        case GL_R16:
        case GL_R16F:
        case GL_R16I:
        case GL_R16UI:
            return 16;
        case GL_R32F:
        case GL_R32I:
        case GL_R32UI:
            return 32;
        case GL_RG8:
        case GL_RG8I:
        case GL_RG8UI:
            return 16;
        case GL_RG16:
        case GL_RG16F:
        case GL_RG16I:
        case GL_RG16UI:
            return 32;
        case GL_RG32F:
        case GL_RG32I:
        case GL_RG32UI:
            return 64;
        case GL_RGB32F:
        case GL_RGB32I:
        case GL_RGB32UI:
            return 96;
        case GL_RGBA8:
        case GL_RGBA8I:
        case GL_RGBA8UI:
            return 32;
        case GL_RGBA16:
        case GL_RGBA16F:
        case GL_RGBA16I:
        case GL_RGBA16UI:
            return 64;
        case GL_RGBA32F:
        case GL_RGBA32I:
        case GL_RGBA32UI:
            return 128;
        default:
            return 0;
    }
}


class COpenGLBuffer : public IGPUBuffer, public IDriverMemoryAllocation
{
    protected:
        virtual ~COpenGLBuffer()
        {
#ifdef OPENGL_LEAK_DEBUG
            assert(concurrentAccessGuard[0]==0);
            FW_AtomicCounterIncr(concurrentAccessGuard[0]);
#endif // OPENGL_LEAK_DEBUG
            if (BufferName)
                COpenGLExtensionHandler::extGlDeleteBuffers(1,&BufferName);

#ifdef OPENGL_LEAK_DEBUG
            assert(concurrentAccessGuard[0]==1);
            FW_AtomicCounterDecr(concurrentAccessGuard[0]);
#endif // OPENGL_LEAK_DEBUG
        }

    public:
        COpenGLBuffer(const IDriverMemoryBacked::SDriverMemoryRequirements &mreqs, const bool& canModifySubData) : IGPUBuffer(mreqs), BufferName(0), cachedFlags(0)
        {
			lastTimeReallocated = 0;
            COpenGLExtensionHandler::extGlCreateBuffers(1,&BufferName);
            if (BufferName==0)
                return;

            cachedFlags =   (canModifySubData ? GL_DYNAMIC_STORAGE_BIT:0)||
                            (mreqs.memoryHeapLocation==IDriverMemoryAllocation::ESMT_NOT_DEVICE_LOCAL ? GL_CLIENT_STORAGE_BIT:0)||
                            ((mreqs.mappingCapability&IDriverMemoryAllocation::EMCF_CAN_MAP_FOR_READ)!=0u ? (GL_MAP_PERSISTENT_BIT|GL_MAP_READ_BIT):0)||
                            ((mreqs.mappingCapability&IDriverMemoryAllocation::EMCF_CAN_MAP_FOR_WRITE)!=0u ? (GL_MAP_PERSISTENT_BIT|GL_MAP_WRITE_BIT):0)||
                            ((mreqs.mappingCapability&IDriverMemoryAllocation::EMCF_COHERENT)!=0u ? GL_MAP_COHERENT_BIT:GL_MAP_FLUSH_EXPLICIT_BIT);
            COpenGLExtensionHandler::extGlNamedBufferStorage(BufferName,cachedMemoryReqs.vulkanReqs.size,nullptr,cachedFlags);

#ifdef OPENGL_LEAK_DEBUG
            for (size_t i=0; i<3; i++)
                concurrentAccessGuard[i] = 0;
#endif // OPENGL_LEAK_DEBUG
        }

        //!
        inline const GLuint& getOpenGLName() const {return BufferName;}


        //!
        virtual bool canUpdateSubRange() const {return cachedFlags&GL_DYNAMIC_STORAGE_BIT;}

        //!
        virtual void updateSubRange(const size_t& offset, const size_t& size, const void* data)
        {
            if (canUpdateSubRange())
                COpenGLExtensionHandler::extGlNamedBufferSubData(BufferName,offset,size,data);
        }


        //! Returns the allocation which is bound to the resource
        virtual IDriverMemoryAllocation* getBoundMemory() {return this;}

        //! Constant version
        virtual const IDriverMemoryAllocation* getBoundMemory() const {return this;}

        //! Returns the offset in the allocation at which it is bound to the resource
        virtual size_t getBoundMemoryOffset() const {return 0ull;}


        //!
        virtual E_SOURCE_MEMORY_TYPE getType() const {return ESMT_DONT_KNOW;}

        //!
        virtual E_MAPPING_CAPABILITY_FLAGS getMappingCaps() const {return EMCF_CANNOT_MAP;}

        //!
        virtual void* mapMemoryRange(const E_MAPPING_CPU_ACCESS_FLAG& accessType, const size_t& offset, const size_t& size)
        {
            GLbitfield flags = ((accessType&IDriverMemoryAllocation::EMCF_CAN_MAP_FOR_READ)!=0u ? GL_MAP_READ_BIT:0)||((accessType&IDriverMemoryAllocation::EMCF_CAN_MAP_FOR_WRITE)!=0u ? GL_MAP_WRITE_BIT:0);
            flags |= cachedFlags&(GL_MAP_PERSISTENT_BIT|GL_MAP_COHERENT_BIT|GL_MAP_FLUSH_EXPLICIT_BIT);
            mappedPtr = reinterpret_cast<uint8_t*>(COpenGLExtensionHandler::extGlMapNamedBufferRange(BufferName,offset,size,flags))-offset;
        }

        //!
        virtual void unmapMemory()
        {
            if (mappedPtr)
                COpenGLExtensionHandler::extGlUnmapNamedBuffer(BufferName);
        }

        //! Whether the allocation was made for a specific resource and is supposed to only be bound to that resource.
        virtual bool isDedicated() const {return true;}
    protected:
        GLbitfield cachedFlags;
        GLuint BufferName;
        /*
        virtual bool reallocate(const size_t &newSize, const bool& forceRetentionOfData, const bool &reallocateIfShrink, const size_t& wraparoundStart)
        {
#ifdef OPENGL_LEAK_DEBUG
            assert(concurrentAccessGuard[2]==0);
            FW_AtomicCounterIncr(concurrentAccessGuard[2]);
#endif // OPENGL_LEAK_DEBUG
            if (newSize==BufferSize)
            {
#ifdef OPENGL_LEAK_DEBUG
                assert(concurrentAccessGuard[2]==1);
                FW_AtomicCounterDecr(concurrentAccessGuard[2]);
#endif // OPENGL_LEAK_DEBUG
                return true;
            }

            if (newSize<BufferSize&&(!reallocateIfShrink))
            {
#ifdef OPENGL_LEAK_DEBUG
                assert(concurrentAccessGuard[2]==1);
                FW_AtomicCounterDecr(concurrentAccessGuard[2]);
#endif // OPENGL_LEAK_DEBUG
                return true;
            }

            if (forceRetentionOfData)
            {
                GLuint newBufferHandle = 0;
                COpenGLExtensionHandler::extGlCreateBuffers(1,&newBufferHandle);
                if (newBufferHandle==0)
                {
    #ifdef OPENGL_LEAK_DEBUG
                    assert(concurrentAccessGuard[2]==1);
                    FW_AtomicCounterDecr(concurrentAccessGuard[2]);
    #endif // OPENGL_LEAK_DEBUG
                    return false;
                }

                COpenGLExtensionHandler::extGlNamedBufferStorage(newBufferHandle,newSize,NULL,cachedFlags);
                if (wraparoundStart&&newSize>BufferSize)
                {
                    size_t wrap = wraparoundStart%BufferSize;
                    COpenGLExtensionHandler::extGlCopyNamedBufferSubData(BufferName,newBufferHandle,wrap,wrap,BufferSize-wrap);
                    COpenGLExtensionHandler::extGlCopyNamedBufferSubData(BufferName,newBufferHandle,0,BufferSize,wrap);
                }
                else
                    COpenGLExtensionHandler::extGlCopyNamedBufferSubData(BufferName,newBufferHandle,0,0,core::min_(newSize,BufferSize));
                BufferSize = newSize;

                COpenGLExtensionHandler::extGlDeleteBuffers(1,&BufferName);
                BufferName = newBufferHandle;
            }
            else
            {
                COpenGLExtensionHandler::extGlDeleteBuffers(1,&BufferName);
                COpenGLExtensionHandler::extGlCreateBuffers(1,&BufferName);
                if (BufferName==0)
                {
#ifdef OPENGL_LEAK_DEBUG
                    assert(concurrentAccessGuard[2]==1);
                    FW_AtomicCounterDecr(concurrentAccessGuard[2]);
#endif // OPENGL_LEAK_DEBUG
                    return false;
                }

                COpenGLExtensionHandler::extGlNamedBufferStorage(BufferName,newSize,NULL,cachedFlags);
                BufferSize = newSize;
            }
            lastTimeReallocated = CNullDriver::incrementAndFetchReallocCounter();

#ifdef OPENGL_LEAK_DEBUG
            assert(concurrentAccessGuard[2]==1);
            FW_AtomicCounterDecr(concurrentAccessGuard[2]);
#endif // OPENGL_LEAK_DEBUG
            return true;
        }
        */
    private:
#ifdef OPENGL_LEAK_DEBUG
        FW_AtomicCounter concurrentAccessGuard[3];
#endif // OPENGL_LEAK_DEBUG
};

} // end namespace video
} // end namespace irr

#endif
#endif
