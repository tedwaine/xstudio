#pragma once

#include "xstudio/ui/opengl/shader_program_base.hpp"

namespace xstudio
{
    namespace media_reader
    {

        class FFMPegPixelUnpackShader : public ui::opengl::OpenGLShader
        {
        public:
            enum ShaderType {
                BLANK,
                YUV,
                RGB
            };
            FFMPegPixelUnpackShader(
                const utility::Uuid &shader_uuid,
                const ShaderType type);
        };
    }
}