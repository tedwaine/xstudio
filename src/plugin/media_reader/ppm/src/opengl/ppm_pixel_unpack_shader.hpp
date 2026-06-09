#pragma once

#include "xstudio/ui/opengl/shader_program_base.hpp"

namespace xstudio
{
    namespace media_reader
    {
        class PPMPixelUnpackShader : public ui::opengl::OpenGLShader
        {
        public:
            PPMPixelUnpackShader(const utility::Uuid &shader_uuid);
        };
    }
}