#pragma once

#include "xstudio/ui/metal/shader_program_base.hpp"

namespace xstudio
{
    namespace media_reader
    {

        class FFMPegPixelUnpackShader : public ui::metal::MetalShader
        {
        public:
            enum ShaderType {
                BLANK,
                YUV,
                RGB
            };
            FFMPegPixelUnpackShader(
                const utility::Uuid &shader_uuid,
                const ShaderType type) {}
            virtual ~FFMPegPixelUnpackShader() = default;
        };
    }
}