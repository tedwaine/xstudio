#pragma once

#include "xstudio/ui/opengl/shader_program_base.hpp"

namespace xstudio
{
    namespace media_reader
    {
        class PDFPixelUnpackShader : public ui::opengl::OpenGLShader
        {
        public:
            PDFPixelUnpackShader(const utility::Uuid &shader_uuid, const bool transparent = false);
        };
    }
}