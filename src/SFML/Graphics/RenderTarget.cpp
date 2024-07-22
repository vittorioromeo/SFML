////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/BlendMode.hpp>
#include <SFML/Graphics/CoordinateType.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Shape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/StencilMode.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/VertexBuffer.hpp>
#include <SFML/Graphics/View.hpp>

#include <SFML/Window/GLCheck.hpp>
#include <SFML/Window/GLExtensions.hpp>
#include <SFML/Window/GlContextTypeImpl.hpp>
#include <SFML/Window/GraphicsContext.hpp>

#include <SFML/System/Err.hpp>
#include <SFML/System/Rect.hpp>

#include <SFML/Base/Algorithm.hpp>
#include <SFML/Base/Assert.hpp>
#include <SFML/Base/Math.hpp>
#include <SFML/Base/Optional.hpp>

#include <mutex>
#include <unordered_map>

#include <cstddef>
#include <cstdint>


namespace
{
// A nested named namespace is used here to allow unity builds of SFML.
namespace RenderTargetImpl
{
// Mutex to protect ID generation and our context-RenderTarget-map
[[nodiscard]] std::recursive_mutex& getMutex()
{
    static std::recursive_mutex mutex;
    return mutex;
}

// Unique identifier, used for identifying RenderTargets when
// tracking the currently active RenderTarget within a given context
[[nodiscard]] std::uint64_t getUniqueId()
{
    const std::lock_guard lock(getMutex());
    static std::uint64_t  id = 1; // start at 1, zero is "no RenderTarget"
    return id++;
}

// Map to help us detect whether a different RenderTarget
// has been activated within a single context
using ContextRenderTargetMap = std::unordered_map<std::uint64_t, std::uint64_t>;
[[nodiscard]] ContextRenderTargetMap& getContextRenderTargetMap()
{
    static ContextRenderTargetMap contextRenderTargetMap;
    return contextRenderTargetMap;
}

// Check if a RenderTarget with the given ID is active in the current context
[[nodiscard]] bool isActive(sf::GraphicsContext& graphicsContext, std::uint64_t id)
{
    const auto& contextRenderTargetMap = getContextRenderTargetMap();

    const auto it = contextRenderTargetMap.find(graphicsContext.getActiveThreadLocalGlContextId());
    return (it != contextRenderTargetMap.end()) && (it->second == id);
}

// Convert an sf::BlendMode::Factor constant to the corresponding OpenGL constant.
[[nodiscard]] std::uint32_t factorToGlConstant(sf::BlendMode::Factor blendFactor)
{
    // clang-format off
    switch (blendFactor)
    {
        case sf::BlendMode::Factor::Zero:             return GL_ZERO;
        case sf::BlendMode::Factor::One:              return GL_ONE;
        case sf::BlendMode::Factor::SrcColor:         return GL_SRC_COLOR;
        case sf::BlendMode::Factor::OneMinusSrcColor: return GL_ONE_MINUS_SRC_COLOR;
        case sf::BlendMode::Factor::DstColor:         return GL_DST_COLOR;
        case sf::BlendMode::Factor::OneMinusDstColor: return GL_ONE_MINUS_DST_COLOR;
        case sf::BlendMode::Factor::SrcAlpha:         return GL_SRC_ALPHA;
        case sf::BlendMode::Factor::OneMinusSrcAlpha: return GL_ONE_MINUS_SRC_ALPHA;
        case sf::BlendMode::Factor::DstAlpha:         return GL_DST_ALPHA;
        case sf::BlendMode::Factor::OneMinusDstAlpha: return GL_ONE_MINUS_DST_ALPHA;
    }
    // clang-format on

    sf::priv::err() << "Invalid value for sf::BlendMode::Factor! Fallback to sf::BlendMode::Factor::Zero.";
    SFML_BASE_ASSERT(false);
    return GL_ZERO;
}


// Convert an sf::BlendMode::Equation constant to the corresponding OpenGL constant.
[[nodiscard]] std::uint32_t equationToGlConstant(sf::BlendMode::Equation blendEquation)
{
    switch (blendEquation)
    {
        case sf::BlendMode::Equation::Add:
            return GLEXT_GL_FUNC_ADD;
        case sf::BlendMode::Equation::Subtract:
            if (GLEXT_blend_subtract)
                return GLEXT_GL_FUNC_SUBTRACT;
            break;
        case sf::BlendMode::Equation::ReverseSubtract:
            if (GLEXT_blend_subtract)
                return GLEXT_GL_FUNC_REVERSE_SUBTRACT;
            break;
        case sf::BlendMode::Equation::Min:
            if (GLEXT_blend_minmax)
                return GLEXT_GL_MIN;
            break;
        case sf::BlendMode::Equation::Max:
            if (GLEXT_blend_minmax)
                return GLEXT_GL_MAX;
            break;
    }

    static bool warned = false;
    if (!warned)
    {
        sf::priv::err() << "OpenGL extension EXT_blend_minmax or EXT_blend_subtract unavailable" << '\n'
                        << "Some blending equations will fallback to sf::BlendMode::Equation::Add" << '\n'
                        << "Ensure that hardware acceleration is enabled if available";

        warned = true;
    }

    return GLEXT_GL_FUNC_ADD;
}


// Convert an UpdateOperation constant to the corresponding OpenGL constant.
[[nodiscard]] std::uint32_t stencilOperationToGlConstant(sf::StencilUpdateOperation operation)
{
    // clang-format off
    switch (operation)
    {
        case sf::StencilUpdateOperation::Keep:      return GL_KEEP;
        case sf::StencilUpdateOperation::Zero:      return GL_ZERO;
        case sf::StencilUpdateOperation::Replace:   return GL_REPLACE;
        case sf::StencilUpdateOperation::Increment: return GL_INCR;
        case sf::StencilUpdateOperation::Decrement: return GL_DECR;
        case sf::StencilUpdateOperation::Invert:    return GL_INVERT;
    }
    // clang-format on

    sf::priv::err() << "Invalid value for sf::StencilUpdateOperation! Fallback to sf::StencilMode::Keep.";
    SFML_BASE_ASSERT(false);
    return GL_KEEP;
}


// Convert a Comparison constant to the corresponding OpenGL constant.
[[nodiscard]] std::uint32_t stencilFunctionToGlConstant(sf::StencilComparison comparison)
{
    // clang-format off
    switch (comparison)
    {
        case sf::StencilComparison::Never:        return GL_NEVER;
        case sf::StencilComparison::Less:         return GL_LESS;
        case sf::StencilComparison::LessEqual:    return GL_LEQUAL;
        case sf::StencilComparison::Greater:      return GL_GREATER;
        case sf::StencilComparison::GreaterEqual: return GL_GEQUAL;
        case sf::StencilComparison::Equal:        return GL_EQUAL;
        case sf::StencilComparison::NotEqual:     return GL_NOTEQUAL;
        case sf::StencilComparison::Always:       return GL_ALWAYS;
    }
    // clang-format on

    sf::priv::err() << "Invalid value for sf::StencilComparison! Fallback to sf::StencilMode::Always.";
    SFML_BASE_ASSERT(false);
    return GL_ALWAYS;
}


////////////////////////////////////////////////////////////
constexpr const char* defaultTexturedShaderVertexSrc = R"glsl(

#ifdef GL_ES
precision mediump float;
#endif

uniform mat4 sf_u_projectionMatrix;
uniform mat4 sf_u_modelViewMatrix;
uniform mat4 sf_u_textureMatrix;

attribute vec2 sf_a_position;
attribute vec4 sf_a_color;
attribute vec2 sf_a_texCoord;

varying vec4 sf_v_color;
varying vec2 sf_v_texCoord;

void main()
{
    gl_Position = sf_u_projectionMatrix * sf_u_modelViewMatrix * vec4(sf_a_position, 0.0, 1.0);
    sf_v_color = sf_a_color;
    sf_v_texCoord = (sf_u_textureMatrix * vec4(sf_a_texCoord, 0.0, 1.0)).xy;
}

)glsl";


////////////////////////////////////////////////////////////
constexpr const char* defaultTexturedShaderFragmentSrc = R"glsl(

#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D sf_u_texture;

varying vec4 sf_v_color;
varying vec2 sf_v_texCoord;

void main()
{
    gl_FragColor = sf_v_color * texture2D(sf_u_texture, sf_v_texCoord.st);
}

)glsl";


////////////////////////////////////////////////////////////
constexpr const char* defaultUntexturedShaderVertexSrc = R"glsl(

#ifdef GL_ES
precision mediump float;
#endif

uniform mat4 sf_u_projectionMatrix;
uniform mat4 sf_u_modelViewMatrix;

attribute vec2 sf_a_position;
attribute vec4 sf_a_color;

varying vec4 sf_v_color;

void main()
{
    gl_Position = sf_u_projectionMatrix * sf_u_modelViewMatrix * vec4(sf_a_position, 0.0, 1.0);
    sf_v_color = sf_a_color;
}

)glsl";


////////////////////////////////////////////////////////////
constexpr const char* defaultUntexturedShaderFragmentSrc = R"glsl(

#ifdef GL_ES
precision mediump float;
#endif

varying vec4 sf_v_color;

void main()
{
    gl_FragColor = sf_v_color;
}

)glsl";


////////////////////////////////////////////////////////////
[[nodiscard]] sf::Shader createBuiltInShader(sf::GraphicsContext& graphicsContext, const char* vertexSrc, const char* fragmentSrc)
{
    auto shader = sf::Shader::loadFromMemory(graphicsContext, vertexSrc, fragmentSrc).value();
    SFML_BASE_ASSERT(glIsProgram(shader.getNativeHandle()));

    if (const sf::base::Optional ulTexture = shader.getUniformLocation("sf_u_texture"))
        shader.setUniform(*ulTexture, sf::Shader::CurrentTexture);

    return shader;
}


////////////////////////////////////////////////////////////
struct [[nodiscard]] BuiltInShaders
{
    sf::Shader texturedShader;
    sf::Shader untexturedShader;
};


////////////////////////////////////////////////////////////
[[nodiscard]] sf::base::Optional<BuiltInShaders>& getBuiltInShaders(sf::GraphicsContext& graphicsContext)
{
    // TODO P0: rewrite to WindowContext and GraphicsContext:WindowContext

    SFML_BASE_ASSERT(graphicsContext.hasActiveThreadLocalOrSharedGlContext());

    static sf::base::Optional<BuiltInShaders> builtInShaders;

    if (graphicsContext.builtInShaderState == 0)
    {
        builtInShaders
            .emplace(createBuiltInShader(graphicsContext, defaultTexturedShaderVertexSrc, defaultTexturedShaderFragmentSrc),
                     createBuiltInShader(graphicsContext, defaultUntexturedShaderVertexSrc, defaultUntexturedShaderFragmentSrc));

        graphicsContext.builtInShaderState = 1;

        graphicsContext.builtInShaderDestroyFn = [] { builtInShaders.reset(); };
    }

    if (graphicsContext.builtInShaderState == 1)
        return builtInShaders;

    SFML_BASE_ASSERT(graphicsContext.builtInShaderState == 2);
    throw 100; // TODO P0: ...
}


////////////////////////////////////////////////////////////
sf::Shader& getShader(sf::GraphicsContext& graphicsContext, const sf::Shader* statesShader, const sf::Texture* statesTexture)
{
    if (statesShader != nullptr)
        return *const_cast<sf::Shader*>(statesShader); // TODO P0: nasty cast...

    if (statesShader == nullptr && statesTexture == nullptr)
        return getBuiltInShaders(graphicsContext)->untexturedShader;

    SFML_BASE_ASSERT(statesShader == nullptr && statesTexture != nullptr);
    return getBuiltInShaders(graphicsContext)->texturedShader;
}

} // namespace RenderTargetImpl
} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Render states cache
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] StatesCache
{
    bool           enable{};                //!< Is the cache enabled?
    bool           glStatesSet{};           //!< Are our internal GL states set yet?
    bool           viewChanged{};           //!< Has the current view changed since last draw?
    bool           scissorEnabled{};        //!< Is scissor testing enabled?
    bool           stencilEnabled{};        //!< Is stencil testing enabled?
    BlendMode      lastBlendMode;           //!< Cached blending mode
    StencilMode    lastStencilMode;         //!< Cached stencil
    std::uint64_t  lastTextureId{};         //!< Cached texture
    CoordinateType lastCoordinateType{};    //!< Texture coordinate type
    bool           texCoordsArrayEnabled{}; //!< Is GL_TEXTURE_COORD_ARRAY client state enabled?
    bool           useVertexCache{};        //!< Did we previously use the vertex cache?
    Vertex         vertexCache[4]{};        //!< Pre-transformed vertices cache
    GLuint         lastUsedProgramId;       //!< GL id of the last used shader program
    GLint          sfAttribPositionIdx;     //!< Index of the "sf_a_position" attribute
    GLint          sfAttribColorIdx;        //!< Index of the "sf_a_color" attribute
    GLint          sfAttribTexCoordIdx;     //!< Index of the "sf_a_texCoord" attribute
};


////////////////////////////////////////////////////////////
template <auto FnGen, auto FnBind, auto FnGet, auto FnDelete>
class OpenGLRAII
{
public:
    [[nodiscard]] explicit OpenGLRAII(GraphicsContext&)
    {
        SFML_BASE_ASSERT(m_id == 0u);
        FnGen(m_id);
        SFML_BASE_ASSERT(m_id != 0u);
    }

    [[nodiscard]] bool isBound() const
    {
        int out{};
        FnGet(out);
        return out != 0u;
    }

    void bind() const
    {
        SFML_BASE_ASSERT(m_id != 0u);
        FnBind(m_id);

        SFML_BASE_ASSERT(isBound());
    }

    ~OpenGLRAII()
    {
        if (m_id != 0u)
            FnDelete(m_id);
    }

    OpenGLRAII(const OpenGLRAII&)            = delete;
    OpenGLRAII& operator=(const OpenGLRAII&) = delete;

    OpenGLRAII(OpenGLRAII&& rhs) noexcept : m_id(base::exchange(rhs.m_id, 0u))
    {
    }

    OpenGLRAII& operator=(OpenGLRAII&& rhs) noexcept
    {
        if (&rhs == this)
            return *this;

        m_id = base::exchange(rhs.m_id, 0u);
        return *this;
    }

private:
    unsigned int m_id{};
};


////////////////////////////////////////////////////////////
using VAO = OpenGLRAII<[](auto& id) { glCheck(glGenVertexArrays(1, &id)); },
                       [](auto id) { glCheck(glBindVertexArray(id)); },
                       [](auto& id) { glCheck(glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &id)); },
                       [](auto& id) { glCheck(glDeleteVertexArrays(1, &id)); }>;


////////////////////////////////////////////////////////////
using VBO = OpenGLRAII<[](auto& id) { glCheck(glGenBuffers(1, &id)); },
                       [](auto id) { glCheck(glBindBuffer(GL_ARRAY_BUFFER, id)); },
                       [](auto& id) { glCheck(glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &id)); },
                       [](auto& id) { glCheck(glDeleteBuffers(1, &id)); }>;


////////////////////////////////////////////////////////////
void doVertexStuff(bool        enableTexCoordsArray,
                   const GLint posAttribIdx,
                   const GLint colorAttribIdx,
                   const GLint texCoordAttribIdx) // TODO P0: better name for func and params
{
#define SFML_PRIV_OFFSETOF(...) reinterpret_cast<const void*>(offsetof(__VA_ARGS__))

    // TODO P0: "actually get the layout indices" -- not sure what that means, we do that...?
    SFML_BASE_ASSERT(posAttribIdx >= 0);

    glCheck(glEnableVertexAttribArray(static_cast<GLuint>(posAttribIdx)));
    glCheck(glVertexAttribPointer(/*      index */ static_cast<GLuint>(posAttribIdx),
                                  /*       size */ 2,
                                  /*       type */ GL_FLOAT,
                                  /* normalized */ GL_FALSE,
                                  /*     stride */ sizeof(Vertex),
                                  /*     offset */ SFML_PRIV_OFFSETOF(Vertex, position)));

    if (colorAttribIdx >= 0)
    {
        glCheck(glEnableVertexAttribArray(static_cast<GLuint>(colorAttribIdx)));
        glCheck(glVertexAttribPointer(/*      index */ static_cast<GLuint>(colorAttribIdx),
                                      /*       size */ 4,
                                      /*       type */ GL_UNSIGNED_BYTE,
                                      /* normalized */ GL_TRUE,
                                      /*     stride */ sizeof(Vertex),
                                      /*     offset */ SFML_PRIV_OFFSETOF(Vertex, color)));
    }

    if (enableTexCoordsArray && texCoordAttribIdx >= 0)
    {
        glCheck(glEnableVertexAttribArray(static_cast<GLuint>(texCoordAttribIdx)));
        glCheck(glVertexAttribPointer(/*      index */ static_cast<GLuint>(texCoordAttribIdx),
                                      /*       size */ 2,
                                      /*       type */ GL_FLOAT,
                                      /* normalized */ GL_FALSE,
                                      /*     stride */ sizeof(Vertex),
                                      /*     offset */ SFML_PRIV_OFFSETOF(Vertex, texCoords)));
    }

#undef SFML_PRIV_OFFSETOF
}


////////////////////////////////////////////////////////////
struct RenderTarget::Impl
{
    explicit Impl(GraphicsContext& theGraphicsContext) :
    graphicsContext(&theGraphicsContext),
    vao(theGraphicsContext),
    vbo(theGraphicsContext)
    {
    }

    GraphicsContext* graphicsContext; //!< The graphics context
    View             defaultView;     //!< Default view
    View             view;            //!< Current view
    StatesCache      cache{};         //!< Render states cache
    std::uint64_t    id{};            //!< Unique number that identifies the render target
    VAO              vao;             //!< Vertex array object associated with the render target
    VBO              vbo;             //!< Vertex buffer object associated with the render target
};


////////////////////////////////////////////////////////////
RenderTarget::RenderTarget(GraphicsContext& graphicsContext) : m_impl(graphicsContext)
{
}


////////////////////////////////////////////////////////////
RenderTarget::~RenderTarget() = default;


////////////////////////////////////////////////////////////
RenderTarget::RenderTarget(RenderTarget&&) noexcept = default;


////////////////////////////////////////////////////////////
RenderTarget& RenderTarget::operator=(RenderTarget&&) noexcept = default;


////////////////////////////////////////////////////////////
[[nodiscard]] bool RenderTarget::clearImpl()
{
    if (!RenderTargetImpl::isActive(*m_impl->graphicsContext, m_impl->id) && !setActive(true))
        return false;

    // Unbind texture to fix RenderTexture preventing clear
    applyTexture(nullptr);

    // Apply the view (scissor testing can affect clearing)
    if (!m_impl->cache.enable || m_impl->cache.viewChanged)
        applyCurrentView(nullptr /* statesShader */, nullptr /* statesTexture */);

    return true;
}


////////////////////////////////////////////////////////////
void RenderTarget::clear(const Color& color)
{
    if (!clearImpl())
        return;

    glCheck(glClearColor(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f));
    glCheck(glClear(GL_COLOR_BUFFER_BIT));
}


////////////////////////////////////////////////////////////
void RenderTarget::clearStencil(StencilValue stencilValue)
{
    if (!clearImpl())
        return;

    glCheck(glClearStencil(static_cast<int>(stencilValue.value)));
    glCheck(glClear(GL_STENCIL_BUFFER_BIT));
}


////////////////////////////////////////////////////////////
void RenderTarget::clear(const Color& color, StencilValue stencilValue)
{
    if (!clearImpl())
        return;

    glCheck(glClearColor(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f));
    glCheck(glClearStencil(static_cast<int>(stencilValue.value)));
    glCheck(glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
}


////////////////////////////////////////////////////////////
void RenderTarget::setView(const View& view)
{
    m_impl->view              = view;
    m_impl->cache.viewChanged = true;
}


////////////////////////////////////////////////////////////
const View& RenderTarget::getView() const
{
    return m_impl->view;
}


////////////////////////////////////////////////////////////
const View& RenderTarget::getDefaultView() const
{
    return m_impl->defaultView;
}


////////////////////////////////////////////////////////////
IntRect RenderTarget::getViewport(const View& view) const
{
    const auto [width, height] = getSize().to<Vector2f>();
    const FloatRect& viewport  = view.getViewport();

    return Rect<long>({base::lround(width * viewport.position.x), base::lround(height * viewport.position.y)},
                      {base::lround(width * viewport.size.x), base::lround(height * viewport.size.y)})
        .to<IntRect>();
}


////////////////////////////////////////////////////////////
IntRect RenderTarget::getScissor(const View& view) const
{
    const auto [width, height] = getSize().to<Vector2f>();
    const FloatRect& scissor   = view.getScissor();

    return Rect<long>({base::lround(width * scissor.position.x), base::lround(height * scissor.position.y)},
                      {base::lround(width * scissor.size.x), base::lround(height * scissor.size.y)})
        .to<IntRect>();
}


////////////////////////////////////////////////////////////
Vector2f RenderTarget::mapPixelToCoords(Vector2i point) const
{
    return mapPixelToCoords(point, getView());
}


////////////////////////////////////////////////////////////
Vector2f RenderTarget::mapPixelToCoords(Vector2i point, const View& view) const
{
    // First, convert from viewport coordinates to homogeneous coordinates
    const auto     viewport   = getViewport(view).to<FloatRect>();
    const Vector2f normalized = Vector2f(-1, 1) +
                                Vector2f(2, -2).cwiseMul(point.to<Vector2f>() - viewport.position).cwiseDiv(viewport.size);

    // Then transform by the inverse of the view matrix
    return view.getInverseTransform().transformPoint(normalized);
}


////////////////////////////////////////////////////////////
Vector2i RenderTarget::mapCoordsToPixel(Vector2f point) const
{
    return mapCoordsToPixel(point, getView());
}


////////////////////////////////////////////////////////////
Vector2i RenderTarget::mapCoordsToPixel(Vector2f point, const View& view) const
{
    // First, transform the point by the view matrix
    const Vector2f normalized = view.getTransform().transformPoint(point);

    // Then convert to viewport coordinates
    const auto viewport = getViewport(view).to<FloatRect>();
    return ((normalized.cwiseMul({1, -1}) + sf::Vector2f{1, 1}).cwiseDiv({2, 2}).cwiseMul(viewport.size) + viewport.position)
        .to<Vector2i>();
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const Sprite& sprite, const Texture& texture, const RenderStates& states)
{
    auto statesCopy = states;

    statesCopy.texture = &texture;
    statesCopy.transform *= sprite.getTransform();
    statesCopy.coordinateType = CoordinateType::Pixels;

    draw(sprite.m_vertices, PrimitiveType::TriangleStrip, statesCopy);
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const Shape& shape, const Texture* texture, const RenderStates& states)
{
    shape.drawOnto(*this, texture, states);
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const Vertex* vertices, std::size_t vertexCount, PrimitiveType type, const RenderStates& states)
{
    // Nothing to draw?
    if (vertices == nullptr || (vertexCount == 0))
        return;

    if (RenderTargetImpl::isActive(*m_impl->graphicsContext, m_impl->id) || setActive(true))
    {
        // Check if the vertex count is low enough so that we can pre-transform them
        const bool useVertexCache = vertexCount <= base::getArraySize(m_impl->cache.vertexCache);

        if (useVertexCache)
        {
            // Pre-transform the vertices and store them into the vertex cache
            for (std::size_t i = 0; i < vertexCount; ++i)
            {
                Vertex& vertex   = m_impl->cache.vertexCache[i];
                vertex.position  = states.transform * vertices[i].position;
                vertex.color     = vertices[i].color;
                vertex.texCoords = vertices[i].texCoords;
            }
        }

        setupDraw(useVertexCache, states);

        // Check if texture coordinates array is needed, and update client state accordingly
        const bool enableTexCoordsArray = states.texture || states.shader;

        // If we pre-transform the vertices, we must use our internal vertex cache
        const auto* data = reinterpret_cast<const char*>(useVertexCache ? m_impl->cache.vertexCache : vertices);

        glCheck(glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeof(Vertex) * vertexCount), data, GL_STATIC_DRAW));
        doVertexStuff(enableTexCoordsArray,
                      m_impl->cache.sfAttribPositionIdx,
                      m_impl->cache.sfAttribColorIdx,
                      m_impl->cache.sfAttribTexCoordIdx);

        drawPrimitives(type, 0, vertexCount);
        cleanupDraw(states);

        // Update the cache
        m_impl->cache.useVertexCache        = useVertexCache;
        m_impl->cache.texCoordsArrayEnabled = enableTexCoordsArray;
    }
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const VertexBuffer& vertexBuffer, const RenderStates& states)
{
    draw(vertexBuffer, 0, vertexBuffer.getVertexCount(), states);
}


////////////////////////////////////////////////////////////
void RenderTarget::draw(const VertexBuffer& vertexBuffer, std::size_t firstVertex, std::size_t vertexCount, const RenderStates& states)
{
    // VertexBuffer not supported?
    if (!VertexBuffer::isAvailable(*m_impl->graphicsContext))
    {
        priv::err() << "sf::VertexBuffer is not available, drawing skipped";
        return;
    }

    // Sanity check
    if (firstVertex > vertexBuffer.getVertexCount())
        return;

    // Clamp vertexCount to something that makes sense
    vertexCount = base::min(vertexCount, vertexBuffer.getVertexCount() - firstVertex);

    // Nothing to draw?
    if (!vertexCount || !vertexBuffer.getNativeHandle())
        return;

    if (RenderTargetImpl::isActive(*m_impl->graphicsContext, m_impl->id) || setActive(true))
    {
        setupDraw(false, states);

        // Bind vertex buffer
        VertexBuffer::bind(*m_impl->graphicsContext, &vertexBuffer);

        // Always enable texture coordinates
        doVertexStuff(true /* enableTexCoordsArray */,
                      m_impl->cache.sfAttribPositionIdx,
                      m_impl->cache.sfAttribColorIdx,
                      m_impl->cache.sfAttribTexCoordIdx);
        drawPrimitives(vertexBuffer.getPrimitiveType(), firstVertex, vertexCount);

        // Unbind vertex buffer
        VertexBuffer::bind(*vertexBuffer.m_graphicsContext, nullptr);

        cleanupDraw(states);

        // Update the cache
        m_impl->cache.useVertexCache        = false;
        m_impl->cache.texCoordsArrayEnabled = true;
    }
}


////////////////////////////////////////////////////////////
bool RenderTarget::isSrgb() const
{
    // By default sRGB encoding is not enabled for an arbitrary RenderTarget
    return false;
}


////////////////////////////////////////////////////////////
bool RenderTarget::setActive(bool active)
{
    // If this RenderTarget is already active on the current GL context, do nothing
    if (const bool isAlreadyActive = RenderTargetImpl::isActive(*m_impl->graphicsContext, m_impl->id);
        (active && isAlreadyActive) || (!active && !isAlreadyActive))
        return true;

    // Mark this RenderTarget as active or no longer active in the tracking map
    const std::lock_guard lock(RenderTargetImpl::getMutex());

    const std::uint64_t contextId = m_impl->graphicsContext->getActiveThreadLocalGlContextId();

    auto& contextRenderTargetMap = RenderTargetImpl::getContextRenderTargetMap();

    const auto it = contextRenderTargetMap.find(contextId);

    // Deactivation
    if (!active)
    {
        SFML_BASE_ASSERT(it != contextRenderTargetMap.end());
        contextRenderTargetMap.erase(it);

        m_impl->cache.enable = false;
        return true;
    }

    // First ever activation
    if (it == contextRenderTargetMap.end())
    {
        contextRenderTargetMap[contextId] = m_impl->id;

        m_impl->cache.glStatesSet = false;
        m_impl->cache.enable      = false;
        return true;
    }

    // Activation on a different context
    SFML_BASE_ASSERT(it->second != m_impl->id);
    it->second = m_impl->id;

    m_impl->cache.enable = false;
    return true;
}


////////////////////////////////////////////////////////////
void RenderTarget::pushGLStates() // TODO P0: remove?
{
    if (RenderTargetImpl::isActive(*m_impl->graphicsContext, m_impl->id) || setActive(true))
    {
#ifdef SFML_DEBUG
        // make sure that the user didn't leave an unchecked OpenGL error
        const GLenum error = glGetError();
        if (error != GL_NO_ERROR)
        {
            priv::err() << "OpenGL error (" << error << ") detected in user code, "
                        << "you should check for errors with glGetError()";
        }
#endif
    }

    resetGLStates();
}


////////////////////////////////////////////////////////////
void RenderTarget::popGLStates() // TODO P0: remove?
{
    (void)(RenderTargetImpl::isActive(*m_impl->graphicsContext, m_impl->id) || setActive(true));
}


////////////////////////////////////////////////////////////
void RenderTarget::resetGLStates()
{
    // Check here to make sure a context change does not happen after activate(true)
    const bool vertexBufferAvailable = VertexBuffer::isAvailable(*m_impl->graphicsContext);

// Workaround for states not being properly reset on
// macOS unless a context switch really takes place
#if defined(SFML_SYSTEM_MACOS)
    if (!setActive(false))
    {
        priv::err() << "Failed to set render target inactive";
    }
#endif

    if (RenderTargetImpl::isActive(*m_impl->graphicsContext, m_impl->id) || setActive(true))
    {
        // Make sure that the texture unit which is active is the number 0
        if (GLEXT_multitexture)
        {
            glCheck(GLEXT_glClientActiveTexture(GLEXT_GL_TEXTURE0));
            glCheck(GLEXT_glActiveTexture(GLEXT_GL_TEXTURE0));
        }

        // Define the default OpenGL states
        glCheck(glDisable(GL_CULL_FACE));
        glCheck(glDisable(GL_STENCIL_TEST));
        glCheck(glDisable(GL_DEPTH_TEST));
        glCheck(glDisable(GL_SCISSOR_TEST));
        glCheck(glEnable(GL_BLEND));
        glCheck(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));

        const auto disableCacheAttrib = [&](const GLint cacheAttrib)
        {
            if (cacheAttrib >= 0)
                glCheck(glDisableVertexAttribArray(static_cast<GLuint>(cacheAttrib)));
        };

        disableCacheAttrib(m_impl->cache.sfAttribPositionIdx);
        disableCacheAttrib(m_impl->cache.sfAttribColorIdx);
        disableCacheAttrib(m_impl->cache.sfAttribTexCoordIdx);

        m_impl->cache.scissorEnabled = false;
        m_impl->cache.stencilEnabled = false;
        m_impl->cache.glStatesSet    = true;

        // Apply the default SFML states
        applyBlendMode(BlendAlpha);
        applyStencilMode(StencilMode());
        applyTexture(nullptr);
        applyShader(nullptr);

        if (vertexBufferAvailable)
            glCheck(VertexBuffer::bind(*m_impl->graphicsContext, nullptr));

        m_impl->cache.texCoordsArrayEnabled = true;

        m_impl->cache.useVertexCache = false;

        // Set the default view
        setView(getView());

        m_impl->cache.enable = true;
    }
}


////////////////////////////////////////////////////////////
const RenderStates& RenderTarget::getDefaultRenderStates()
{
    return RenderStates::Default;
}


////////////////////////////////////////////////////////////
void RenderTarget::initialize()
{
    // Setup the default and current views
    m_impl->defaultView = View(FloatRect({0, 0}, getSize().to<Vector2f>()));
    m_impl->view        = m_impl->defaultView;

    // Set GL states only on first draw, so that we don't pollute user's states
    m_impl->cache.glStatesSet         = false;
    m_impl->cache.lastUsedProgramId   = 0u;
    m_impl->cache.sfAttribPositionIdx = -1;
    m_impl->cache.sfAttribColorIdx    = -1;
    m_impl->cache.sfAttribTexCoordIdx = -1;

    // Generate a unique ID for this RenderTarget to track
    // whether it is active within a specific context
    m_impl->id = RenderTargetImpl::getUniqueId();
}


////////////////////////////////////////////////////////////
GraphicsContext& RenderTarget::getGraphicsContext()
{
    return *m_impl->graphicsContext;
}


////////////////////////////////////////////////////////////
void RenderTarget::applyCurrentView(const Shader* statesShader, const Texture* statesTexture)
{
    // Set the viewport
    const IntRect viewport    = getViewport(m_impl->view);
    const int     viewportTop = static_cast<int>(getSize().y) - (viewport.position.y + viewport.size.y);
    glCheck(glViewport(viewport.position.x, viewportTop, viewport.size.x, viewport.size.y));

    // Set the scissor rectangle and enable/disable scissor testing
    if (m_impl->view.getScissor() == FloatRect({0, 0}, {1, 1}))
    {
        if (!m_impl->cache.enable || m_impl->cache.scissorEnabled)
        {
            glCheck(glDisable(GL_SCISSOR_TEST));
            m_impl->cache.scissorEnabled = false;
        }
    }
    else
    {
        const IntRect pixelScissor = getScissor(m_impl->view);
        const int     scissorTop   = static_cast<int>(getSize().y) - (pixelScissor.position.y + pixelScissor.size.y);
        glCheck(glScissor(pixelScissor.position.x, scissorTop, pixelScissor.size.x, pixelScissor.size.y));

        if (!m_impl->cache.enable || !m_impl->cache.scissorEnabled)
        {
            glCheck(glEnable(GL_SCISSOR_TEST));
            m_impl->cache.scissorEnabled = true;
        }
    }

    // Set the projection matrix
    Shader& shader = RenderTargetImpl::getShader(*m_impl->graphicsContext, statesShader, statesTexture);
    shader.setUniform(shader.getUniformLocation("sf_u_projectionMatrix").value(),
                      Glsl::Mat4(m_impl->view.getTransform().getMatrix()));

    m_impl->cache.viewChanged = false;
}


////////////////////////////////////////////////////////////
void RenderTarget::applyBlendMode(const BlendMode& mode)
{
    using RenderTargetImpl::equationToGlConstant;
    using RenderTargetImpl::factorToGlConstant;

    // Apply the blend mode, falling back to the non-separate versions if necessary
    if (GLEXT_blend_func_separate)
    {
        glCheck(GLEXT_glBlendFuncSeparate(factorToGlConstant(mode.colorSrcFactor),
                                          factorToGlConstant(mode.colorDstFactor),
                                          factorToGlConstant(mode.alphaSrcFactor),
                                          factorToGlConstant(mode.alphaDstFactor)));
    }
    else
    {
        glCheck(glBlendFunc(factorToGlConstant(mode.colorSrcFactor), factorToGlConstant(mode.colorDstFactor)));
    }

    if (GLEXT_blend_minmax || GLEXT_blend_subtract)
    {
        if (GLEXT_blend_equation_separate)
        {
            glCheck(GLEXT_glBlendEquationSeparate(equationToGlConstant(mode.colorEquation),
                                                  equationToGlConstant(mode.alphaEquation)));
        }
        else
        {
            glCheck(GLEXT_glBlendEquation(equationToGlConstant(mode.colorEquation)));
        }
    }
    else if ((mode.colorEquation != BlendMode::Equation::Add) || (mode.alphaEquation != BlendMode::Equation::Add))
    {
        static bool warned = false;

        if (!warned)
        {
#ifdef SFML_OPENGL_ES
            priv::err() << "OpenGL ES extension OES_blend_subtract unavailable";
#else
            priv::err() << "OpenGL extension EXT_blend_minmax and EXT_blend_subtract unavailable";
#endif
            priv::err() << "Selecting a blend equation not possible" << '\n'
                        << "Ensure that hardware acceleration is enabled if available";

            warned = true;
        }
    }

    m_impl->cache.lastBlendMode = mode;
}


////////////////////////////////////////////////////////////
void RenderTarget::applyStencilMode(const StencilMode& mode)
{
    using RenderTargetImpl::stencilFunctionToGlConstant;
    using RenderTargetImpl::stencilOperationToGlConstant;

    // Fast path if we have a default (disabled) stencil mode
    if (mode == StencilMode())
    {
        if (!m_impl->cache.enable || m_impl->cache.stencilEnabled)
        {
            glCheck(glDisable(GL_STENCIL_TEST));
            glCheck(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));

            m_impl->cache.stencilEnabled = false;
        }
    }
    else
    {
        // Apply the stencil mode
        if (!m_impl->cache.enable || !m_impl->cache.stencilEnabled)
            glCheck(glEnable(GL_STENCIL_TEST));

        glCheck(glStencilOp(GL_KEEP,
                            stencilOperationToGlConstant(mode.stencilUpdateOperation),
                            stencilOperationToGlConstant(mode.stencilUpdateOperation)));
        glCheck(glStencilFunc(stencilFunctionToGlConstant(mode.stencilComparison),
                              static_cast<int>(mode.stencilReference.value),
                              mode.stencilMask.value));

        m_impl->cache.stencilEnabled = true;
    }

    m_impl->cache.lastStencilMode = mode;
}


////////////////////////////////////////////////////////////
void RenderTarget::applyTexture(const Texture* texture, CoordinateType coordinateType)
{
    Texture::bind(*m_impl->graphicsContext, texture);

    m_impl->cache.lastTextureId      = texture ? texture->m_cacheId : 0;
    m_impl->cache.lastCoordinateType = coordinateType;
}


////////////////////////////////////////////////////////////
void RenderTarget::applyShader(const Shader* shader)
{
    if (shader != nullptr)
        shader->bind();
    else
        Shader::unbind(*m_impl->graphicsContext);
}


////////////////////////////////////////////////////////////
void RenderTarget::setupDraw(bool useVertexCache, const RenderStates& states)
{
    // GL_FRAMEBUFFER_SRGB is not available on OpenGL ES
    // If a framebuffer supports sRGB, it will always be enabled on OpenGL ES
#ifndef SFML_OPENGL_ES
    // Enable or disable sRGB encoding
    // This is needed for drivers that do not check the format of the surface drawn to before applying sRGB conversion
    if (!m_impl->cache.enable)
    {
        if (isSrgb())
            glCheck(glEnable(GL_FRAMEBUFFER_SRGB));
        else
            glCheck(glDisable(GL_FRAMEBUFFER_SRGB));
    }
#endif

    // First set the persistent OpenGL states if it's the very first call
    if (!m_impl->cache.glStatesSet)
        resetGLStates();

    Shader& usedShader = RenderTargetImpl::getShader(*m_impl->graphicsContext, states.shader, states.texture);


    if (useVertexCache)
    {
        usedShader.setUniform(usedShader.getUniformLocation("sf_u_modelViewMatrix").value(),
                              Glsl::Mat4(Transform::Identity.getMatrix()));
    }
    else
    {
        usedShader.setUniform(usedShader.getUniformLocation("sf_u_modelViewMatrix").value(),
                              Glsl::Mat4(states.transform.getMatrix()));
    }

    // Apply the view
    // if (!m_impl->cache.enable || m_impl->cache.viewChanged) // TODO P0: uncommenting breaks island example
    applyCurrentView(states.shader, states.texture);

    // Apply the blend mode
    if (!m_impl->cache.enable || (states.blendMode != m_impl->cache.lastBlendMode))
        applyBlendMode(states.blendMode);

    // Apply the stencil mode
    if (!m_impl->cache.enable || (states.stencilMode != m_impl->cache.lastStencilMode))
        applyStencilMode(states.stencilMode);

    // Mask the color buffer off if necessary
    if (states.stencilMode.stencilOnly)
        glCheck(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));

    // Apply the texture
    if (!m_impl->cache.enable || (states.texture && states.texture->m_fboAttachment))
    {
        // If the texture is an FBO attachment, always rebind it
        // in order to inform the OpenGL driver that we want changes
        // made to it in other contexts to be visible here as well
        // This saves us from having to call glFlush() in
        // RenderTextureImplFBO which can be quite costly
        // See: https://www.khronos.org/opengl/wiki/Memory_Model
        applyTexture(states.texture, states.coordinateType);
    }
    else
    {
        const std::uint64_t textureId = states.texture ? states.texture->m_cacheId : 0;
        if (textureId != m_impl->cache.lastTextureId || states.coordinateType != m_impl->cache.lastCoordinateType)
            applyTexture(states.texture, states.coordinateType);
    }

    // Update shader texture matrix
    if (states.texture != nullptr)
    {
        const base::Optional ulTextureMatrix = usedShader.getUniformLocation("sf_u_textureMatrix");
        if (ulTextureMatrix.hasValue()) // Might be optimized out sometimes...?
            usedShader.setUniform(*ulTextureMatrix, states.texture->getMatrix(m_impl->cache.lastCoordinateType));
    }

    // Apply the shader
    applyShader(&usedShader);

    m_impl->vao.bind();
    m_impl->vbo.bind();

    // Update cache
    const auto usedNativeHandle = usedShader.getNativeHandle();
    if (m_impl->cache.lastUsedProgramId != usedNativeHandle)
    {
        m_impl->cache.lastUsedProgramId = usedNativeHandle;

        const auto updateCacheAttrib = [&](GLint& cacheAttrib, const char* attribName)
        {
            cacheAttrib = glGetAttribLocation(usedNativeHandle, attribName);

            if (cacheAttrib >= 0)
                glCheck(glEnableVertexAttribArray(static_cast<GLuint>(cacheAttrib)));
        };

        updateCacheAttrib(m_impl->cache.sfAttribPositionIdx, "sf_a_position");
        updateCacheAttrib(m_impl->cache.sfAttribColorIdx, "sf_a_color");
        updateCacheAttrib(m_impl->cache.sfAttribTexCoordIdx, "sf_a_texCoord");
    }
}


////////////////////////////////////////////////////////////
void RenderTarget::drawPrimitives(PrimitiveType type, std::size_t firstVertex, std::size_t vertexCount)
{
    // Find the OpenGL primitive type
    static constexpr GLenum modes[] = {GL_POINTS, GL_LINES, GL_LINE_STRIP, GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN};
    const GLenum mode = modes[static_cast<std::size_t>(type)];

    // Draw the primitives
    m_impl->vao.bind();
    glCheck(glDrawArrays(mode, static_cast<GLint>(firstVertex), static_cast<GLsizei>(vertexCount)));
}


////////////////////////////////////////////////////////////
void RenderTarget::cleanupDraw(const RenderStates& states)
{
    // Unbind the shader, if any
    applyShader(nullptr);

    // If the texture we used to draw belonged to a RenderTexture, then forcibly unbind that texture.
    // This prevents a bug where some drivers do not clear RenderTextures properly.
    if (states.texture && states.texture->m_fboAttachment)
        applyTexture(nullptr);

    // Mask the color buffer back on if necessary
    if (states.stencilMode.stencilOnly)
        glCheck(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));

    // Re-enable the cache at the end of the draw if it was disabled
    m_impl->cache.enable = true;
}

} // namespace sf


////////////////////////////////////////////////////////////
// Render states caching strategies
//
// * View
//   If SetView was called since last draw, the projection
//   matrix is updated. We don't need more, the view doesn't
//   change frequently.
//
// * Transform
//   The transform matrix is usually expensive because each
//   entity will most likely use a different transform. This can
//   lead, in worst case, to changing it every 4 vertices.
//   To avoid that, when the vertex count is low enough, we
//   pre-transform them and therefore use an identity transform
//   to render them.
//
// * Blending mode
//   Since it overloads the == operator, we can easily check
//   whether any of the 6 blending components changed and,
//   thus, whether we need to update the blend mode.
//
// * Texture
//   Storing the pointer or OpenGL ID of the last used texture
//   is not enough; if the sf::Texture instance is destroyed,
//   both the pointer and the OpenGL ID might be recycled in
//   a new texture instance. We need to use our own unique
//   identifier system to ensure consistent caching.
//
// * Shader
//   Shaders are very hard to optimize, because they have
//   parameters that can be hard (if not impossible) to track,
//   like matrices or textures. The only optimization that we
//   do is that we avoid setting a null shader if there was
//   already none for the previous draw.
//
////////////////////////////////////////////////////////////
