#include <Metal/Metal.h>
#include <AppKit/AppKit.h>

#include "xstudio/ui/metal/metal_shader_program.hpp"
#include "xstudio/media_reader/media_reader.hpp"
#include "xstudio/utility/logging.hpp"
#include "xstudio/utility/uuid.hpp"

using namespace xstudio;
using namespace xstudio::ui::viewport;
using namespace xstudio::ui::metal;
using namespace xstudio::media_reader;
using namespace xstudio::colour_pipeline;
using namespace xstudio::utility;

id<MTLFunction> MetalShaderProgram::compileShaderFromSource(const std::string &src, const std::string &entryPoint)
{

    NSString *srcstr = [NSString stringWithCString: src.c_str() encoding:[NSString defaultCStringEncoding]];
    MTLCompileOptions *opts = [[MTLCompileOptions alloc] init];
    opts.languageVersion = MTLLanguageVersion1_2;
    NSError *err = nullptr;
    id<MTLLibrary> lib = [device_ newLibraryWithSource: srcstr options: opts error: &err];
    // srcstr is autoreleased, opts is managed by ARC

    if (err) {
        NSAlert *anAlert = [NSAlert alertWithError:err];
        [anAlert runModal];
        return nullptr;
    }

    NSString *name = [NSString stringWithCString: entryPoint.c_str() encoding:[NSString defaultCStringEncoding]];
    id<MTLFunction> fn = [lib newFunctionWithName: name];
    // [name release]; // NSString created with stringWithCString is autoreleased

    return fn;
}

MetalShaderProgram::MetalShaderProgram(
    id<MTLDevice> device,
    const std::string &vertex_shader,
    const std::string &fragment_shader,
    const bool do_compile)
    : device_(device), vertex_shaders_({vertex_shader}), fragment_shaders_({fragment_shader})
{
    if (do_compile) {
        compile();
    }
}

MetalShaderProgram::~MetalShaderProgram() {
    // Destructor implementation here
}

void MetalShaderProgram::compile() {
    // Method implementation here
    vertex_shader_ = compileShaderFromSource(vertex_shaders_.back(), "main0");
    fragment_shader_ = compileShaderFromSource(fragment_shaders_.back(), "main0");
}

void MetalShaderProgram::inject_colour_op_shader(const std::string &colour_op_shader) {
    // Method implementation here
}

void MetalShaderProgram::set_shader_parameters(const utility::JsonStore &shader_params) {
    // Method implementation here
}

void MetalShaderProgram::set_shader_parameters(const media_reader::ImageBufPtr &image) {
    // Method implementation here
}