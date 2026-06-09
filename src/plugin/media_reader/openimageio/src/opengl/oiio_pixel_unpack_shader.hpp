#pragma once

#include "xstudio/ui/opengl/shader_program_base.hpp"

namespace xstudio
{
    namespace media_reader
    {
        class OIIOPixelUnpackShader : public ui::opengl::OpenGLShader
        {
        public:
            OIIOPixelUnpackShader(const utility::Uuid &shader_uuid);
        };
    }
}