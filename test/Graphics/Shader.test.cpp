#include <SFML/Graphics/Shader.hpp>

// Other 1st party headers
#include <SFML/Window/GraphicsContext.hpp>

#include <SFML/System/FileInputStream.hpp>
#include <SFML/System/Path.hpp>

#include <SFML/Base/Macros.hpp>

#include <Doctest.hpp>

#include <string_view>
#include <type_traits>

namespace
{
constexpr auto vertexSource = R"glsl(

#ifdef GL_ES
precision mediump float;
#endif

uniform vec2  storm_position;
uniform float storm_total_radius;
uniform float storm_inner_radius;

uniform mat4 sf_u_projectionMatrix;
uniform mat4 sf_u_textureMatrix;
uniform mat4 sf_u_modelViewMatrix;

attribute vec4 sf_a_color;
attribute vec2 sf_a_position;
attribute vec2 sf_a_texCoord;

varying vec4 sf_v_color;
varying vec2 sf_v_texCoord;

void main()
{
    vec2 newPosition = sf_a_position;

    vec2 offset = newPosition.xy - storm_position;

    float len = length(offset);
    if (len < storm_total_radius)
    {
        float push_distance = storm_inner_radius + len / storm_total_radius * (storm_total_radius - storm_inner_radius);
        newPosition.xy      = storm_position + normalize(offset) * push_distance;
    }

    gl_Position   = sf_u_projectionMatrix * sf_u_modelViewMatrix * vec4(newPosition, 0.0, 1.0);
    sf_v_texCoord = (sf_u_textureMatrix * vec4(sf_a_texCoord, 0.0, 1.0)).xy;
    sf_v_color    = sf_a_color;
}

)glsl";

constexpr auto geometrySource = R"glsl(

#version 150

// The render target's resolution (used for scaling)
uniform vec2 resolution;

// The billboards' size
uniform vec2 size;

// Input is the passed point cloud
layout(points) in;

// The output will consist of triangle strips with four vertices each
layout(triangle_strip, max_vertices = 4) out;

// Output texture coordinates
varying vec2 sf_v_texCoord;

in gl_PerVertex
{
    vec4 gl_Position;
}
gl_in[];

uniform mat4 sf_u_textureMatrix;

// Main entry point
void main()
{
    // Calculate the half width/height of the billboards
    vec2 half_size = size / 2.f;

    // Scale the size based on resolution (1 would be full width/height)
    half_size /= resolution;

    // Iterate over all vertices
    for (int i = 0; i < gl_in.length(); ++i)
    {
        // Retrieve the passed vertex position
        vec2 pos = gl_in[i].gl_Position.xy;

        // Bottom left vertex
        gl_Position   = vec4(pos - half_size, 0.f, 1.f);
        sf_v_texCoord = vec2(1.f, 1.f);
        EmitVertex();

        // Bottom right vertex
        gl_Position   = vec4(pos.x + half_size.x, pos.y - half_size.y, 0.f, 1.f);
        sf_v_texCoord = vec2(0.f, 1.f);
        EmitVertex();

        // Top left vertex
        gl_Position   = vec4(pos.x - half_size.x, pos.y + half_size.y, 0.f, 1.f);
        sf_v_texCoord = vec2(1.f, 0.f);
        EmitVertex();

        // Top right vertex
        gl_Position   = vec4(pos + half_size, 0.f, 1.f);
        sf_v_texCoord = vec2(0.f, 0.f);
        EmitVertex();

        // And finalize the primitive
        EndPrimitive();
    }
}

)glsl";

constexpr auto fragmentSource = R"glsl(

#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D sf_u_texture;
uniform float     blink_alpha;

varying vec4 sf_v_color;
varying vec2 sf_v_texCoord;

void main()
{
    vec4 pixel = sf_v_color;
    pixel.a    = blink_alpha;

    gl_FragColor = pixel;
}

)glsl";

#ifdef SFML_RUN_DISPLAY_TESTS
constexpr bool skipShaderFullTest = false;
#else
constexpr bool skipShaderFullTest = true;
#endif

} // namespace

TEST_CASE("[Graphics] sf::Shader" * doctest::skip(skipShaderFullTest))
{
    sf::GraphicsContext graphicsContext;

    SECTION("Type traits")
    {
        STATIC_CHECK(!std::is_default_constructible_v<sf::Shader>);
        STATIC_CHECK(!std::is_copy_constructible_v<sf::Shader>);
        STATIC_CHECK(!std::is_copy_assignable_v<sf::Shader>);
        STATIC_CHECK(std::is_nothrow_move_constructible_v<sf::Shader>);
        STATIC_CHECK(std::is_nothrow_move_assignable_v<sf::Shader>);
    }

    SECTION("Move semantics")
    {
        SECTION("Construction")
        {
            sf::Shader movedShader = sf::Shader::loadFromFile(graphicsContext, "Graphics/shader.vert", sf::Shader::Type::Vertex)
                                         .value();
            const sf::Shader shader = SFML_BASE_MOVE(movedShader);
            CHECK(shader.getNativeHandle() != 0);
        }

        SECTION("Assignment")
        {
            sf::Shader movedShader = sf::Shader::loadFromFile(graphicsContext, "Graphics/shader.vert", sf::Shader::Type::Vertex)
                                         .value();
            sf::Shader shader = sf::Shader::loadFromFile(graphicsContext, "Graphics/shader.frag", sf::Shader::Type::Fragment)
                                    .value();
            shader = SFML_BASE_MOVE(movedShader);
            CHECK(shader.getNativeHandle() != 0);
        }
    }

    SECTION("loadFromFile()")
    {
        SECTION("One shader")
        {
            CHECK(!sf::Shader::loadFromFile(graphicsContext, "does-not-exist.vert", sf::Shader::Type::Vertex));

            const auto vertexShader = sf::Shader::loadFromFile(graphicsContext, "Graphics/shader.vert", sf::Shader::Type::Vertex);
            CHECK(vertexShader.hasValue());
            if (vertexShader)
                CHECK(static_cast<bool>(vertexShader->getNativeHandle()));

            const auto fragmentShader = sf::Shader::loadFromFile(graphicsContext,
                                                                 "Graphics/shader.frag",
                                                                 sf::Shader::Type::Fragment);
            CHECK(fragmentShader.hasValue());
            if (fragmentShader)
                CHECK(static_cast<bool>(fragmentShader->getNativeHandle()));
        }

        SECTION("Two shaders")
        {
            CHECK(!sf::Shader::loadFromFile(graphicsContext, "does-not-exist.vert", "Graphics/shader.frag"));
            CHECK(!sf::Shader::loadFromFile(graphicsContext, "Graphics/shader.vert", "does-not-exist.frag"));

            const auto shader = sf::Shader::loadFromFile(graphicsContext,
                                                         "Graphics/shader.vert",
                                                         "Graphics/shader.frag");
            CHECK(shader.hasValue());
            if (shader)
                CHECK(static_cast<bool>(shader->getNativeHandle()));
        }

        SECTION("Three shaders")
        {
            CHECK(!sf::Shader::loadFromFile(graphicsContext,
                                            "does-not-exist.vert",
                                            "Graphics/shader.geom",
                                            "Graphics/shader.frag"));
            CHECK(!sf::Shader::loadFromFile(graphicsContext,
                                            "Graphics/shader.vert",
                                            "does-not-exist.geom",
                                            "Graphics/shader.frag"));
            CHECK(!sf::Shader::loadFromFile(graphicsContext,
                                            "Graphics/shader.vert",
                                            "Graphics/shader.geom",
                                            "does-not-exist.frag"));

            const auto shader = sf::Shader::loadFromFile(graphicsContext,
                                                         "Graphics/shader.vert",
                                                         "Graphics/shader.geom",
                                                         "Graphics/shader.frag");
            CHECK(shader.hasValue() == sf::Shader::isGeometryAvailable(graphicsContext));
            if (shader)
                CHECK(static_cast<bool>(shader->getNativeHandle()) == sf::Shader::isGeometryAvailable(graphicsContext));
        }
    }

    SECTION("loadFromMemory()")
    {
        CHECK(sf::Shader::loadFromMemory(graphicsContext, vertexSource, sf::Shader::Type::Vertex).hasValue());
        CHECK(!sf::Shader::loadFromMemory(graphicsContext, geometrySource, sf::Shader::Type::Geometry));
        CHECK(sf::Shader::loadFromMemory(graphicsContext, fragmentSource, sf::Shader::Type::Fragment).hasValue());
        CHECK(sf::Shader::loadFromMemory(graphicsContext, vertexSource, fragmentSource).hasValue());

        const auto shader = sf::Shader::loadFromMemory(graphicsContext, vertexSource, geometrySource, fragmentSource);
        CHECK(shader.hasValue() == sf::Shader::isGeometryAvailable(graphicsContext));
        if (shader)
            CHECK(static_cast<bool>(shader->getNativeHandle()));
    }

    SECTION("loadFromStream()")
    {
        auto vertexShaderStream   = sf::FileInputStream::open("Graphics/shader.vert").value();
        auto fragmentShaderStream = sf::FileInputStream::open("Graphics/shader.frag").value();
        auto geometryShaderStream = sf::FileInputStream::open("Graphics/shader.geom").value();

        auto emptyStream = sf::FileInputStream::open("Graphics/invalid_shader.vert").value();

        SECTION("One shader")
        {
            CHECK(!sf::Shader::loadFromStream(graphicsContext, emptyStream, sf::Shader::Type::Vertex));
            CHECK(sf::Shader::loadFromStream(graphicsContext, vertexShaderStream, sf::Shader::Type::Vertex).hasValue());
            CHECK(sf::Shader::loadFromStream(graphicsContext, fragmentShaderStream, sf::Shader::Type::Fragment).hasValue());
        }

        SECTION("Two shaders")
        {
            CHECK(!sf::Shader::loadFromStream(graphicsContext, emptyStream, fragmentShaderStream));
            CHECK(!sf::Shader::loadFromStream(graphicsContext, vertexShaderStream, emptyStream));
            CHECK(sf::Shader::loadFromStream(graphicsContext, vertexShaderStream, fragmentShaderStream).hasValue());
        }

        SECTION("Three shaders")
        {
            CHECK(!sf::Shader::loadFromStream(graphicsContext, emptyStream, geometryShaderStream, fragmentShaderStream));
            CHECK(!sf::Shader::loadFromStream(graphicsContext, vertexShaderStream, emptyStream, fragmentShaderStream));
            CHECK(!sf::Shader::loadFromStream(graphicsContext, vertexShaderStream, geometryShaderStream, emptyStream));

            const auto shader = sf::Shader::loadFromStream(graphicsContext,
                                                           vertexShaderStream,
                                                           geometryShaderStream,
                                                           fragmentShaderStream);

            CHECK(shader.hasValue() == sf::Shader::isGeometryAvailable(graphicsContext));
            if (shader)
                CHECK(static_cast<bool>(shader->getNativeHandle()) == sf::Shader::isGeometryAvailable(graphicsContext));
        }
    }
}
