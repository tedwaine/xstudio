#pragma once

#include "xstudio/ui/metal/shader_program_base.hpp"

namespace xstudio
{
    namespace demo_plugin
    {

        class DemoPixelUnpackShader : public ui::metal::MetalShader
        {
        public:
            DemoPixelUnpackShader(
                const utility::Uuid &shader_uuid) {}
            virtual ~DemoPixelUnpackShader() = default;
        };
    }
}