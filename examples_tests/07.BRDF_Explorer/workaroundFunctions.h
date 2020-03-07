#include <cstdint>
#include <CEGUI/Size.h>

namespace CEGUI
{
	class OpenGL3Renderer;
}

::CEGUI::Texture& irrTex2ceguiTex(uint32_t _GLname, const ::CEGUI::Sizef& _sz, const std::string& _name, CEGUI::OpenGL3Renderer& _renderer);

uint32_t getTextureGLname(irr::video::IGPUImageView* _texture);