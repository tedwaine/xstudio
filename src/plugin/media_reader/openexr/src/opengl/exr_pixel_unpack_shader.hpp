#pragma once

#include "xstudio/ui/opengl/shader_program_base.hpp"

namespace xstudio
{
    namespace media_reader
    {
        class EXRPixelUnpackShader : public ui::opengl::OpenGLShader
        {
        public:
            EXRPixelUnpackShader(const utility::Uuid &shader_uuid);
        };
    }
}