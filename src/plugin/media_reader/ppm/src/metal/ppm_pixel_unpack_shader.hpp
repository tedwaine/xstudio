#pragma once

#include "xstudio/ui/metal/shader_program_base.hpp"

namespace xstudio
{
    namespace media_reader
    {
        class PPMPixelUnpackShader : public ui::metal::MetalShader
        {
        public:
            PPMPixelUnpackShader(const utility::Uuid &shader_uuid) {}
        };
    }
}