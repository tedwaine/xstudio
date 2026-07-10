#pragma once

#include "xstudio/ui/metal/shader_program_base.hpp"

namespace xstudio
{
    namespace media_reader
    {
        class EXRPixelUnpackShader : public ui::metal::MetalShader
        {
        public:
            EXRPixelUnpackShader(const utility::Uuid &shader_uuid) {}
            virtual ~EXRPixelUnpackShader() = default;
        };
    }
}