#pragma once

#include <string>
#include <vector>

#include <core/Types.h>

namespace frostwind {

struct GraphicsConfig {
    bool vsync = true;
};

struct GraphicsLimits {
    int maxTextureSize = 0;
    usize maxBufferSize = 0;
    usize maxParameterBytes = 0;
    u32 maxVertexAttributes = 0;
};

enum class FrameStatus { Ready, Unavailable, Failed };
enum class ColorFormat { RGBA8 };
enum class ShaderStage { Vertex, Fragment };
enum class ShaderFormat { GLSL };
enum class VertexFormat { Float2 };
enum class PrimitiveTopology { TriangleList };
enum class BlendMode { Opaque };
enum class DepthMode { Disabled };
enum class CullMode { None };

struct FrameTarget {
    int width = 0;
    int height = 0;
    ColorFormat format = ColorFormat::RGBA8;
};

struct Color {
    f32 r = 0;
    f32 g = 0;
    f32 b = 0;
    f32 a = 1;
};

struct ShaderDescription {
    ShaderStage stage = ShaderStage::Vertex;
    ShaderFormat format = ShaderFormat::GLSL;
    std::string entryPoint = "main";
};

struct VertexAttribute {
    u32 location = 0;
    VertexFormat format = VertexFormat::Float2;
    usize offset = 0;
};

// The initial contract has one optional vertex parameter block and one optional
// fragment texture, each at engine slot 0. Names identify shader declarations;
// native locations and binding points are resolved privately by the backend.
struct ParameterBinding {
    std::string name;
    usize byteSize = 0;
};

struct TextureBinding {
    std::string name;
};

struct PipelineDescription {
    usize vertexStride = 0;
    std::vector<VertexAttribute> attributes;
    ParameterBinding parameters;
    TextureBinding texture;
    ColorFormat targetFormat = ColorFormat::RGBA8;
    PrimitiveTopology topology = PrimitiveTopology::TriangleList;
    BlendMode blend = BlendMode::Opaque;
    DepthMode depth = DepthMode::Disabled;
    CullMode cull = CullMode::None;
};

} // namespace frostwind
