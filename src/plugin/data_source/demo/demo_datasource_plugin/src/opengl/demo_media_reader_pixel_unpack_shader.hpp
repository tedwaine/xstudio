#pragma once

#include "xstudio/ui/opengl/shader_program_base.hpp"

namespace xstudio
{
    namespace demo_plugin
    {

        class DemoPixelUnpackShader : public ui::opengl::OpenGLShader
        {
        public:
            DemoPixelUnpackShader(
                const utility::Uuid &shader_uuid);
        };
    }
}