#pragma once

#include "xstudio/ui/metal/shader_program_base.hpp"

namespace xstudio
{
    namespace media_reader
    {
        class PDFPixelUnpackShader : public ui::metal::MetalShader
        {
        public:
            PDFPixelUnpackShader(const utility::Uuid &shader_uuid, const bool transparent = false) {}
            virtual ~PDFPixelUnpackShader() = default;
        };
    }
}