#pragma once

#include "xstudio/ui/metal/shader_program_base.hpp"

namespace xstudio
{
    namespace media_reader
    {
        class OIIOPixelUnpackShader : public ui::metal::MetalShader
        {
        public:
            OIIOPixelUnpackShader(const utility::Uuid &shader_uuid)
             : ui::metal::MetalShader(shader_uuid, "") {}
        };
    }
}