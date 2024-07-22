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
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <SFML/Window/GLCheck.hpp>
#include <SFML/Window/GLExtensions.hpp>
#include <SFML/Window/GraphicsContext.hpp>

#include <SFML/System/Err.hpp>
#include <SFML/System/InputStream.hpp>
#include <SFML/System/Path.hpp>
#include <SFML/System/PathUtils.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>

#include <SFML/Base/Algorithm.hpp>
#include <SFML/Base/Assert.hpp>
#include <SFML/Base/Macros.hpp>
#include <SFML/Base/Optional.hpp>

#include <fstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>


#if defined(SFML_SYSTEM_MACOS) || defined(SFML_SYSTEM_IOS)

#define castToGlHandle(x)   reinterpret_cast<GLEXT_GLhandle>(static_cast<std::ptrdiff_t>(x))
#define castFromGlHandle(x) static_cast<unsigned int>(reinterpret_cast<std::ptrdiff_t>(x))

#else

#define castToGlHandle(x)   (x)
#define castFromGlHandle(x) (x)

#endif


namespace
{
// Retrieve the maximum number of texture units available
[[nodiscard]] std::size_t getMaxTextureUnits()
{
    static const auto maxUnits = static_cast<std::size_t>(sf::priv::getGLInteger(GLEXT_GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS));
    return maxUnits;
}

// Pair of indices into thread-local buffer
struct [[nodiscard]] BufferSlice
{
    const std::size_t beginIdx;
    const std::size_t count;

    explicit BufferSlice(std::size_t b, std::size_t c) : beginIdx(b), count(c)
    {
    }

    [[nodiscard]] std::string_view toView(const std::vector<char>& buffer) const
    {
        return {buffer.data() + beginIdx, count};
    }
};

// Read the contents of a file into an array of char
[[nodiscard]] sf::base::Optional<BufferSlice> appendFileContentsToVector(const sf::Path& filename, std::vector<char>& buffer)
{
    std::ifstream file(filename.to<std::string>(), std::ios_base::binary);

    if (!file)
    {
        sf::priv::err() << "Failed to open shader file";
        return sf::base::nullOpt;
    }

    file.seekg(0, std::ios_base::end);
    const std::ifstream::pos_type size = file.tellg();

    const std::size_t bufferSizeBeforeRead = buffer.size();

    if (size > 0)
    {
        file.seekg(0, std::ios_base::beg);
        buffer.resize(static_cast<std::size_t>(size) + bufferSizeBeforeRead);
        file.read(buffer.data() + bufferSizeBeforeRead, static_cast<std::streamsize>(size));
    }

    buffer.push_back('\0');
    return sf::base::makeOptional<BufferSlice>(bufferSizeBeforeRead, buffer.size() - bufferSizeBeforeRead);
}

// Read the contents of a stream into an array of char
[[nodiscard]] sf::base::Optional<BufferSlice> appendStreamContentsToVector(sf::InputStream& stream, std::vector<char>& buffer)
{
    const sf::base::Optional<std::size_t> size = stream.getSize();

    if (!size.hasValue() || size.value() == 0)
    {
        buffer.push_back('\0');
        return sf::base::nullOpt;
    }

    const std::size_t bufferSizeBeforeRead = buffer.size();
    buffer.resize(*size + bufferSizeBeforeRead);

    if (!stream.seek(0).hasValue())
    {
        sf::priv::err() << "Failed to seek shader stream";
        return sf::base::nullOpt;
    }

    const sf::base::Optional<std::size_t> read = stream.read(buffer.data() + bufferSizeBeforeRead, *size);

    if (!read.hasValue() || *read != *size)
    {
        sf::priv::err() << "Failed to read stream contents into buffer";
        return sf::base::nullOpt;
    }

    buffer.push_back('\0');
    return sf::base::makeOptional<BufferSlice>(bufferSizeBeforeRead, buffer.size() - bufferSizeBeforeRead);
}

// Return a thread-local vector for suitable use as a temporary buffer
// This function is non-reentrant
[[nodiscard]] std::vector<char>& getThreadLocalCharBuffer()
{
    thread_local std::vector<char> result;
    return result;
}

// Transforms an array of 2D vectors into a contiguous array of scalars
[[nodiscard]] std::vector<float> flatten(const sf::Vector2f* vectorArray, std::size_t length)
{
    const std::size_t vectorSize = 2;

    std::vector<float> contiguous(vectorSize * length);
    for (std::size_t i = 0; i < length; ++i)
    {
        contiguous[vectorSize * i]     = vectorArray[i].x;
        contiguous[vectorSize * i + 1] = vectorArray[i].y;
    }

    return contiguous;
}

// Transforms an array of 3D vectors into a contiguous array of scalars
[[nodiscard]] std::vector<float> flatten(const sf::Vector3f* vectorArray, std::size_t length)
{
    const std::size_t vectorSize = 3;

    std::vector<float> contiguous(vectorSize * length);
    for (std::size_t i = 0; i < length; ++i)
    {
        contiguous[vectorSize * i]     = vectorArray[i].x;
        contiguous[vectorSize * i + 1] = vectorArray[i].y;
        contiguous[vectorSize * i + 2] = vectorArray[i].z;
    }

    return contiguous;
}

// Transforms an array of 4D vectors into a contiguous array of scalars
[[nodiscard]] std::vector<float> flatten(const sf::Glsl::Vec4* vectorArray, std::size_t length)
{
    const std::size_t vectorSize = 4;

    std::vector<float> contiguous(vectorSize * length);
    for (std::size_t i = 0; i < length; ++i)
    {
        contiguous[vectorSize * i]     = vectorArray[i].x;
        contiguous[vectorSize * i + 1] = vectorArray[i].y;
        contiguous[vectorSize * i + 2] = vectorArray[i].z;
        contiguous[vectorSize * i + 3] = vectorArray[i].w;
    }

    return contiguous;
}

struct StringHash
{
    using is_transparent = void;

    [[nodiscard]] size_t operator()(const char* txt) const
    {
        return std::hash<std::string_view>{}(txt);
    }

    [[nodiscard]] size_t operator()(std::string_view txt) const
    {
        return std::hash<std::string_view>{}(txt);
    }

    [[nodiscard]] size_t operator()(const std::string& txt) const
    {
        return std::hash<std::string>{}(txt);
    }
};

} // namespace


namespace sf
{
struct Shader::Impl
{
    using TextureTable = std::unordered_map<int, const Texture*>;
    using UniformTable = std::unordered_map<std::string, int, StringHash, std::equal_to<>>;

    GraphicsContext* graphicsContext;
    unsigned int     shaderProgram{};    //!< OpenGL identifier for the program
    int              currentTexture{-1}; //!< Location of the current texture in the shader
    TextureTable     textures;           //!< Texture variables in the shader, mapped to their location
    UniformTable     uniforms;           //!< Parameters location cache

    explicit Impl(GraphicsContext& theGraphicsContext, unsigned int theShaderProgram) :
    graphicsContext(&theGraphicsContext),
    shaderProgram(theShaderProgram)
    {
    }

    Impl(Impl&& rhs) noexcept :
    graphicsContext(rhs.graphicsContext),
    shaderProgram(base::exchange(rhs.shaderProgram, 0u)),
    currentTexture(base::exchange(rhs.currentTexture, -1)),
    textures(SFML_BASE_MOVE(rhs.textures)),
    uniforms(SFML_BASE_MOVE(rhs.uniforms))
    {
    }
};


////////////////////////////////////////////////////////////
Shader::UniformLocation::UniformLocation(int location) : m_value(location)
{
    SFML_BASE_ASSERT(m_value != -1);
}


////////////////////////////////////////////////////////////
class [[nodiscard]] Shader::UniformBinder
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Constructor: set up state before uniform is set
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] explicit UniformBinder(Shader& shader) :
    m_currentProgram(static_cast<GLEXT_GLhandle>(castToGlHandle(shader.m_impl->shaderProgram)))
    {
        SFML_BASE_ASSERT(m_currentProgram != 0);


// Enable program object
#ifndef SFML_OPENGL_ES
        glCheck(m_savedProgram = GLEXT_glGetHandle(GLEXT_GL_PROGRAM_OBJECT));
#else
        glCheck(glGetIntegerv(GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>(&m_savedProgram)));
#endif

        if (m_currentProgram != m_savedProgram)
            glCheck(GLEXT_glUseProgramObject(m_currentProgram));
    }

    ////////////////////////////////////////////////////////////
    /// \brief Destructor: restore state after uniform is set
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] ~UniformBinder()
    {
        // Disable program object
        if (m_currentProgram && (m_currentProgram != m_savedProgram))
            glCheck(GLEXT_glUseProgramObject(m_savedProgram));
    }

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    UniformBinder(const UniformBinder&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    UniformBinder& operator=(const UniformBinder&) = delete;

private:
    GLEXT_GLhandle m_currentProgram; //!< Handle to the program object of the modified `sf::Shader` instance
    GLEXT_GLhandle m_savedProgram{}; //!< Handle to the previously active program object
};


////////////////////////////////////////////////////////////
Shader::~Shader()
{
    SFML_BASE_ASSERT(m_impl->graphicsContext->hasActiveThreadLocalOrSharedGlContext());

    // Destroy effect program
    if (m_impl->shaderProgram)
        glCheck(GLEXT_glDeleteProgram(castToGlHandle(m_impl->shaderProgram)));
}


////////////////////////////////////////////////////////////
Shader::Shader(Shader&& source) noexcept = default;


////////////////////////////////////////////////////////////
Shader& Shader::operator=(Shader&& right) noexcept
{
    // Make sure we aren't moving ourselves.
    if (&right == this)
    {
        return *this;
    }

    // Explicit scope for RAII
    {
        // Destroy effect program
        SFML_BASE_ASSERT(m_impl->graphicsContext->hasActiveThreadLocalOrSharedGlContext());
        SFML_BASE_ASSERT(m_impl->shaderProgram);
        glCheck(GLEXT_glDeleteProgram(castToGlHandle(m_impl->shaderProgram)));
    }

    // Move the contents of right.
    m_impl->shaderProgram  = base::exchange(right.m_impl->shaderProgram, 0u);
    m_impl->currentTexture = base::exchange(right.m_impl->currentTexture, -1);
    m_impl->textures       = SFML_BASE_MOVE(right.m_impl->textures);
    m_impl->uniforms       = SFML_BASE_MOVE(right.m_impl->uniforms);

    return *this;
}


////////////////////////////////////////////////////////////
base::Optional<Shader> Shader::loadFromFile(GraphicsContext& graphicsContext, const Path& filename, Type type)
{
    // Prepare thread-local buffer
    std::vector<char>& buffer = getThreadLocalCharBuffer();
    buffer.clear();

    // Read the file
    const base::Optional<BufferSlice> shaderSlice = appendFileContentsToVector(filename, buffer);
    if (!shaderSlice.hasValue())
    {
        priv::err() << "Failed to open shader file\n" << priv::PathDebugFormatter{filename};
        return base::nullOpt;
    }

    const std::string_view shaderView = shaderSlice->toView(buffer);

    // Compile the shader program
    if (type == Type::Vertex)
        return compile(graphicsContext, shaderView, {}, {});

    if (type == Type::Geometry)
        return compile(graphicsContext, {}, shaderView, {});

    return compile(graphicsContext, {}, {}, shaderView);
}


////////////////////////////////////////////////////////////
base::Optional<Shader> Shader::loadFromFile(GraphicsContext& graphicsContext,
                                            const Path&      vertexShaderFilename,
                                            const Path&      fragmentShaderFilename)
{
    // Prepare thread-local buffer
    std::vector<char>& buffer = getThreadLocalCharBuffer();
    buffer.clear();

    // Read the vertex shader file
    const base::Optional<BufferSlice> vertexShaderSlice = appendFileContentsToVector(vertexShaderFilename, buffer);
    if (!vertexShaderSlice.hasValue())
    {
        priv::err() << "Failed to open vertex shader file\n" << priv::PathDebugFormatter{vertexShaderFilename};
        return base::nullOpt;
    }

    // Read the fragment shader file
    const base::Optional<BufferSlice> fragmentShaderSlice = appendFileContentsToVector(fragmentShaderFilename, buffer);
    if (!fragmentShaderSlice.hasValue())
    {
        priv::err() << "Failed to open fragment shader file\n" << priv::PathDebugFormatter{fragmentShaderFilename};
        return base::nullOpt;
    }

    // Compile the shader program
    return compile(graphicsContext, vertexShaderSlice->toView(buffer), {}, fragmentShaderSlice->toView(buffer));
}


////////////////////////////////////////////////////////////
base::Optional<Shader> Shader::loadFromFile(GraphicsContext& graphicsContext,
                                            const Path&      vertexShaderFilename,
                                            const Path&      geometryShaderFilename,
                                            const Path&      fragmentShaderFilename)
{
    // Prepare thread-local buffer
    std::vector<char>& buffer = getThreadLocalCharBuffer();
    buffer.clear();

    // Read the vertex shader file
    const base::Optional<BufferSlice> vertexShaderSlice = appendFileContentsToVector(vertexShaderFilename, buffer);
    if (!vertexShaderSlice.hasValue())
    {
        priv::err() << "Failed to open vertex shader file\n" << priv::PathDebugFormatter{vertexShaderFilename};
        return base::nullOpt;
    }

    // Read the geometry shader file
    const base::Optional<BufferSlice> geometryShaderSlice = appendFileContentsToVector(geometryShaderFilename, buffer);
    if (!geometryShaderSlice.hasValue())
    {
        priv::err() << "Failed to open geometry shader file\n" << priv::PathDebugFormatter{geometryShaderFilename};
        return base::nullOpt;
    }

    // Read the fragment shader file
    const base::Optional<BufferSlice> fragmentShaderSlice = appendFileContentsToVector(fragmentShaderFilename, buffer);
    if (!fragmentShaderSlice.hasValue())
    {
        priv::err() << "Failed to open fragment shader file\n" << priv::PathDebugFormatter{fragmentShaderFilename};
        return base::nullOpt;
    }

    // Compile the shader program
    return compile(graphicsContext,
                   vertexShaderSlice->toView(buffer),
                   geometryShaderSlice->toView(buffer),
                   fragmentShaderSlice->toView(buffer));
}


////////////////////////////////////////////////////////////
base::Optional<Shader> Shader::loadFromMemory(GraphicsContext& graphicsContext, std::string_view shader, Type type)
{
    // Compile the shader program
    if (type == Type::Vertex)
        return compile(graphicsContext, shader, {}, {});

    if (type == Type::Geometry)
        return compile(graphicsContext, {}, shader, {});

    SFML_BASE_ASSERT(type == Type::Fragment);
    return compile(graphicsContext, {}, {}, shader);
}


////////////////////////////////////////////////////////////
base::Optional<Shader> Shader::loadFromMemory(GraphicsContext& graphicsContext,
                                              std::string_view vertexShader,
                                              std::string_view fragmentShader)
{
    // Compile the shader program
    return compile(graphicsContext, vertexShader, {}, fragmentShader);
}


////////////////////////////////////////////////////////////
base::Optional<Shader> Shader::loadFromMemory(GraphicsContext& graphicsContext,
                                              std::string_view vertexShader,
                                              std::string_view geometryShader,
                                              std::string_view fragmentShader)
{
    // Compile the shader program
    return compile(graphicsContext, vertexShader, geometryShader, fragmentShader);
}


////////////////////////////////////////////////////////////
base::Optional<Shader> Shader::loadFromStream(GraphicsContext& graphicsContext, InputStream& stream, Type type)
{
    // Prepare thread-local buffer
    std::vector<char>& buffer = getThreadLocalCharBuffer();
    buffer.clear();

    // Read the shader code from the stream
    const base::Optional<BufferSlice> shaderSlice = appendStreamContentsToVector(stream, buffer);
    if (!shaderSlice.hasValue())
    {
        priv::err() << "Failed to read vertex shader from stream";
        return base::nullOpt;
    }

    const std::string_view shaderView = shaderSlice->toView(buffer);

    // Compile the shader program
    if (type == Type::Vertex)
        return compile(graphicsContext, shaderView, {}, {});

    if (type == Type::Geometry)
        return compile(graphicsContext, {}, shaderView, {});

    SFML_BASE_ASSERT(type == Type::Fragment);
    return compile(graphicsContext, {}, {}, shaderView);
}


////////////////////////////////////////////////////////////
base::Optional<Shader> Shader::loadFromStream(GraphicsContext& graphicsContext,
                                              InputStream&     vertexShaderStream,
                                              InputStream&     fragmentShaderStream)
{
    // Prepare thread-local buffer
    std::vector<char>& buffer = getThreadLocalCharBuffer();
    buffer.clear();

    // Read the vertex shader code from the stream
    const base::Optional<BufferSlice> vertexShaderSlice = appendStreamContentsToVector(vertexShaderStream, buffer);
    if (!vertexShaderSlice.hasValue())
    {
        priv::err() << "Failed to read vertex shader from stream";
        return base::nullOpt;
    }

    // Read the fragment shader code from the stream
    const base::Optional<BufferSlice> fragmentShaderSlice = appendStreamContentsToVector(fragmentShaderStream, buffer);
    if (!fragmentShaderSlice.hasValue())
    {
        priv::err() << "Failed to read fragment shader from stream";
        return base::nullOpt;
    }

    // Compile the shader program
    return compile(graphicsContext, vertexShaderSlice->toView(buffer), {}, fragmentShaderSlice->toView(buffer));
}


////////////////////////////////////////////////////////////
base::Optional<Shader> Shader::loadFromStream(GraphicsContext& graphicsContext,
                                              InputStream&     vertexShaderStream,
                                              InputStream&     geometryShaderStream,
                                              InputStream&     fragmentShaderStream)
{
    // Prepare thread-local buffer
    std::vector<char>& buffer = getThreadLocalCharBuffer();
    buffer.clear();

    // Read the vertex shader code from the stream
    const base::Optional<BufferSlice> vertexShaderSlice = appendStreamContentsToVector(vertexShaderStream, buffer);
    if (!vertexShaderSlice.hasValue())
    {
        priv::err() << "Failed to read vertex shader from stream";
        return base::nullOpt;
    }

    // Read the geometry shader code from the stream
    const base::Optional<BufferSlice> geometryShaderSlice = appendStreamContentsToVector(geometryShaderStream, buffer);
    if (!geometryShaderSlice.hasValue())
    {
        priv::err() << "Failed to read geometry shader from stream";
        return base::nullOpt;
    }

    // Read the fragment shader code from the stream
    const base::Optional<BufferSlice> fragmentShaderSlice = appendStreamContentsToVector(fragmentShaderStream, buffer);
    if (!fragmentShaderSlice.hasValue())
    {
        priv::err() << "Failed to read fragment shader from stream";
        return base::nullOpt;
    }

    // Compile the shader program
    return compile(graphicsContext,
                   vertexShaderSlice->toView(buffer),
                   geometryShaderSlice->toView(buffer),
                   fragmentShaderSlice->toView(buffer));
}


////////////////////////////////////////////////////////////
base::Optional<Shader::UniformLocation> Shader::getUniformLocation(std::string_view uniformName)
{
    // Check the cache
    if (const auto it = m_impl->uniforms.find(uniformName); it != m_impl->uniforms.end())
    {
        // Already in cache, return it
        return it->second == -1 ? base::nullOpt : sf::base::makeOptional(UniformLocation{it->second});
    }

    // Use thread-local string buffer to get a null-terminated uniform name
    thread_local std::string uniformNameBuffer;
    uniformNameBuffer.clear();
    uniformNameBuffer.assign(uniformName);

    // Not in cache, request the location from OpenGL
    const int location = GLEXT_glGetUniformLocation(castToGlHandle(m_impl->shaderProgram), uniformNameBuffer.c_str());
    m_impl->uniforms.emplace(uniformName, location);

    return location == -1 ? base::nullOpt : sf::base::makeOptional(UniformLocation{location});
}


////////////////////////////////////////////////////////////
void Shader::setUniform(UniformLocation location, float x)
{
    const UniformBinder binder{*this};
    glCheck(GLEXT_glUniform1f(location.m_value, x));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(UniformLocation location, Glsl::Vec2 v)
{
    const UniformBinder binder{*this};
    glCheck(GLEXT_glUniform2f(location.m_value, v.x, v.y));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(UniformLocation location, const Glsl::Vec3& v)
{
    const UniformBinder binder{*this};
    glCheck(GLEXT_glUniform3f(location.m_value, v.x, v.y, v.z));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(UniformLocation location, const Glsl::Vec4& v)
{
    const UniformBinder binder{*this};
    glCheck(GLEXT_glUniform4f(location.m_value, v.x, v.y, v.z, v.w));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(UniformLocation location, int x)
{
    const UniformBinder binder{*this};
    glCheck(GLEXT_glUniform1i(location.m_value, x));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(UniformLocation location, Glsl::Ivec2 v)
{
    const UniformBinder binder{*this};
    glCheck(GLEXT_glUniform2i(location.m_value, v.x, v.y));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(UniformLocation location, const Glsl::Ivec3& v)
{
    const UniformBinder binder{*this};
    glCheck(GLEXT_glUniform3i(location.m_value, v.x, v.y, v.z));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(UniformLocation location, const Glsl::Ivec4& v)
{
    const UniformBinder binder{*this};
    glCheck(GLEXT_glUniform4i(location.m_value, v.x, v.y, v.z, v.w));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(UniformLocation location, bool x)
{
    return setUniform(location, static_cast<int>(x));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(UniformLocation location, Glsl::Bvec2 v)
{
    return setUniform(location, v.to<Glsl::Ivec2>());
}


////////////////////////////////////////////////////////////
void Shader::setUniform(UniformLocation location, const Glsl::Bvec3& v)
{
    return setUniform(location, v.to<Glsl::Ivec3>());
}


////////////////////////////////////////////////////////////
void Shader::setUniform(UniformLocation location, const Glsl::Bvec4& v)
{
    return setUniform(location, Glsl::Ivec4(v));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(UniformLocation location, const Glsl::Mat3& matrix)
{
    const UniformBinder binder{*this};
    glCheck(GLEXT_glUniformMatrix3fv(location.m_value, 1, GL_FALSE, matrix.array));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(UniformLocation location, const Glsl::Mat4& matrix)
{
    const UniformBinder binder{*this};
    glCheck(GLEXT_glUniformMatrix4fv(location.m_value, 1, GL_FALSE, matrix.array));
}


////////////////////////////////////////////////////////////
bool Shader::setUniform(UniformLocation location, const Texture& texture)
{
    SFML_BASE_ASSERT(m_impl->shaderProgram);

    SFML_BASE_ASSERT(m_impl->graphicsContext->hasActiveThreadLocalOrSharedGlContext());

    // Store the location -> texture mapping
    if (const auto it = m_impl->textures.find(location.m_value); it != m_impl->textures.end())
    {
        // Location already used, just replace the texture
        it->second = &texture;
        return true;
    }

    // New entry, make sure there are enough texture units
    if (m_impl->textures.size() + 1 >= getMaxTextureUnits())
    {
        priv::err() << "Impossible to use texture \"" << location.m_value << '"'
                    << " for shader: all available texture units are used";

        return false;
    }

    m_impl->textures[location.m_value] = &texture;
    return true;
}


////////////////////////////////////////////////////////////
void Shader::setUniform(UniformLocation location, CurrentTextureType)
{
    SFML_BASE_ASSERT(m_impl->shaderProgram);

    SFML_BASE_ASSERT(m_impl->graphicsContext->hasActiveThreadLocalOrSharedGlContext());

    // Find the location of the variable in the shader
    m_impl->currentTexture = location.m_value;
}


////////////////////////////////////////////////////////////
void Shader::setUniformArray(UniformLocation location, const float* scalarArray, std::size_t length)
{
    const UniformBinder binder{*this};
    glCheck(GLEXT_glUniform1fv(location.m_value, static_cast<GLsizei>(length), scalarArray));
}


////////////////////////////////////////////////////////////
void Shader::setUniformArray(UniformLocation location, const Glsl::Vec2* vectorArray, std::size_t length)
{
    std::vector<float>  contiguous = flatten(vectorArray, length);
    const UniformBinder binder{*this};
    glCheck(GLEXT_glUniform2fv(location.m_value, static_cast<GLsizei>(length), contiguous.data()));
}


////////////////////////////////////////////////////////////
void Shader::setUniformArray(UniformLocation location, const Glsl::Vec3* vectorArray, std::size_t length)
{
    std::vector<float>  contiguous = flatten(vectorArray, length);
    const UniformBinder binder{*this};
    glCheck(GLEXT_glUniform3fv(location.m_value, static_cast<GLsizei>(length), contiguous.data()));
}


////////////////////////////////////////////////////////////
void Shader::setUniformArray(UniformLocation location, const Glsl::Vec4* vectorArray, std::size_t length)
{
    std::vector<float>  contiguous = flatten(vectorArray, length);
    const UniformBinder binder{*this};
    glCheck(GLEXT_glUniform4fv(location.m_value, static_cast<GLsizei>(length), contiguous.data()));
}


////////////////////////////////////////////////////////////
void Shader::setUniformArray(UniformLocation location, const Glsl::Mat3* matrixArray, std::size_t length)
{
    const std::size_t matrixSize = 3 * 3;

    std::vector<float> contiguous(matrixSize * length);
    for (std::size_t i = 0; i < length; ++i)
        priv::copyMatrix(matrixArray[i].array, matrixSize, &contiguous[matrixSize * i]);

    const UniformBinder binder{*this};
    glCheck(GLEXT_glUniformMatrix3fv(location.m_value, static_cast<GLsizei>(length), GL_FALSE, contiguous.data()));
}


////////////////////////////////////////////////////////////
void Shader::setUniformArray(UniformLocation location, const Glsl::Mat4* matrixArray, std::size_t length)
{
    const std::size_t matrixSize = 4 * 4;

    std::vector<float> contiguous(matrixSize * length);
    for (std::size_t i = 0; i < length; ++i)
        priv::copyMatrix(matrixArray[i].array, matrixSize, &contiguous[matrixSize * i]);

    const UniformBinder binder{*this};
    glCheck(GLEXT_glUniformMatrix4fv(location.m_value, static_cast<GLsizei>(length), GL_FALSE, contiguous.data()));
}


////////////////////////////////////////////////////////////
unsigned int Shader::getNativeHandle() const
{
    return m_impl->shaderProgram;
}


////////////////////////////////////////////////////////////
void Shader::bind() const
{
    SFML_BASE_ASSERT(m_impl->graphicsContext->hasActiveThreadLocalOrSharedGlContext());

    // Make sure that we can use shaders
    ensureIsAvailable(*m_impl->graphicsContext);

    if (m_impl->shaderProgram == 0)
    {
        // Bind no shader
        glCheck(GLEXT_glUseProgramObject({}));
        return;
    }

    // Enable the program
    SFML_BASE_ASSERT(glIsProgram(castToGlHandle(m_impl->shaderProgram)));
    glCheck(GLEXT_glUseProgramObject(castToGlHandle(m_impl->shaderProgram)));

    // Bind the textures
    bindTextures();

    // Bind the current texture
    if (m_impl->currentTexture != -1)
        glCheck(GLEXT_glUniform1i(m_impl->currentTexture, 0));
}


void Shader::unbind([[maybe_unused]] GraphicsContext& graphicsContext)
{
    SFML_BASE_ASSERT(graphicsContext.hasActiveThreadLocalOrSharedGlContext());

    // Bind no shader
    glCheck(GLEXT_glUseProgramObject({}));
}


////////////////////////////////////////////////////////////
void Shader::ensureIsAvailable([[maybe_unused]] GraphicsContext& graphicsContext)
{
#ifndef SFML_OPENGL_ES

    static const bool available = [&graphicsContext]
    {
        SFML_BASE_ASSERT(graphicsContext.hasActiveThreadLocalOrSharedGlContext());
        return GLEXT_multitexture && GLEXT_shader_objects && GLEXT_vertex_shader && GLEXT_fragment_shader;
    }();

    if (!available)
    {
        priv::err() << "[[SFML FATAL ERROR]]: your system doesn't support shaders";
        std::abort();
    }

#endif
}


////////////////////////////////////////////////////////////
bool Shader::isGeometryAvailable([[maybe_unused]] GraphicsContext& graphicsContext)
{
    ensureIsAvailable(graphicsContext);

    static const bool available = [&graphicsContext]
    {
        SFML_BASE_ASSERT(graphicsContext.hasActiveThreadLocalOrSharedGlContext());
        return GLEXT_geometry_shader4 || GLAD_GL_ES_VERSION_3_2 || GLEXT_GL_VERSION_3_2;
    }();

    return available;
}


////////////////////////////////////////////////////////////
Shader::Shader(base::PassKey<Shader>&&, GraphicsContext& graphicsContext, unsigned int shaderProgram) :
m_impl(graphicsContext, shaderProgram)
{
}


////////////////////////////////////////////////////////////
base::Optional<Shader> Shader::compile(GraphicsContext& graphicsContext,
                                       std::string_view vertexShaderCode,
                                       std::string_view geometryShaderCode,
                                       std::string_view fragmentShaderCode)
{
    SFML_BASE_ASSERT(graphicsContext.hasActiveThreadLocalOrSharedGlContext());

    // First make sure that we can use shaders
    ensureIsAvailable(graphicsContext);

    // Make sure we can use geometry shaders
    if (geometryShaderCode.data() != nullptr && !isGeometryAvailable(graphicsContext))
    {
        priv::err() << "Failed to create a shader: your system doesn't support geometry shaders "
                    << "(you should test Shader::isGeometryAvailable() before trying to use geometry shaders)";

        return base::nullOpt;
    }

    // Create the program
    GLEXT_GLhandle shaderProgram{};
    glCheck(shaderProgram = glCreateProgram());
    SFML_BASE_ASSERT(glIsProgram(shaderProgram));

#ifdef SFML_OPENGL_ES
    if (vertexShaderCode.data() == nullptr)
    {
        vertexShaderCode = R"glsl(

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
    }
#endif

    // Create the vertex shader if needed
    if (vertexShaderCode.data())
    {
        // Create and compile the shader
        GLEXT_GLhandle vertexShader{};
        glCheck(vertexShader = GLEXT_glCreateShaderObject(GLEXT_GL_VERTEX_SHADER));
        const GLcharARB* sourceCode       = vertexShaderCode.data();
        const auto       sourceCodeLength = static_cast<GLint>(vertexShaderCode.length());
        glCheck(GLEXT_glShaderSource(vertexShader, 1, &sourceCode, &sourceCodeLength));
        glCheck(GLEXT_glCompileShader(vertexShader));
        SFML_BASE_ASSERT(glIsShader(vertexShader));

        // Check the compile log
        GLint success = 0;
        glCheck(GLEXT_glGetShaderParameteriv(vertexShader, GLEXT_GL_OBJECT_COMPILE_STATUS, &success));
        if (success == GL_FALSE)
        {
            char log[1024];
            glCheck(GLEXT_glGetShaderInfoLog(vertexShader, sizeof(log), nullptr, log));
            priv::err() << "Failed to compile vertex shader:" << '\n' << static_cast<const char*>(log);
            glCheck(GLEXT_glDeleteShader(vertexShader));
            glCheck(GLEXT_glDeleteProgram(shaderProgram));
            return base::nullOpt;
        }

        // Attach the shader to the program, and delete it (not needed anymore)
        glCheck(GLEXT_glAttachShader(shaderProgram, vertexShader));
        glCheck(GLEXT_glDeleteShader(vertexShader));
    }

    // Create the geometry shader if needed
    if (geometryShaderCode.data())
    {
        // Create and compile the shader
        const GLEXT_GLhandle geometryShader   = GLEXT_glCreateShaderObject(GL_GEOMETRY_SHADER);
        const GLcharARB*     sourceCode       = geometryShaderCode.data();
        const auto           sourceCodeLength = static_cast<GLint>(geometryShaderCode.length());
        glCheck(GLEXT_glShaderSource(geometryShader, 1, &sourceCode, &sourceCodeLength));
        glCheck(GLEXT_glCompileShader(geometryShader));
        SFML_BASE_ASSERT(glIsShader(geometryShader));

        // Check the compile log
        GLint success = 0;
        glCheck(GLEXT_glGetShaderParameteriv(geometryShader, GLEXT_GL_OBJECT_COMPILE_STATUS, &success));
        if (success == GL_FALSE)
        {
            char log[1024];
            glCheck(GLEXT_glGetShaderInfoLog(geometryShader, sizeof(log), nullptr, log));
            priv::err() << "Failed to compile geometry shader:" << '\n' << static_cast<const char*>(log);
            glCheck(GLEXT_glDeleteShader(geometryShader));
            glCheck(GLEXT_glDeleteProgram(shaderProgram));
            return base::nullOpt;
        }

        // Attach the shader to the program, and delete it (not needed anymore)
        glCheck(GLEXT_glAttachShader(shaderProgram, geometryShader));
        glCheck(GLEXT_glDeleteShader(geometryShader));
    }

#ifdef SFML_OPENGL_ES
    if (fragmentShaderCode.data() == nullptr)
    {
        fragmentShaderCode = R"glsl(

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
    }
#endif

    // Create the fragment shader if needed
    if (fragmentShaderCode.data())
    {
        // Create and compile the shader
        GLEXT_GLhandle fragmentShader{};
        glCheck(fragmentShader = GLEXT_glCreateShaderObject(GLEXT_GL_FRAGMENT_SHADER));
        const GLcharARB* sourceCode       = fragmentShaderCode.data();
        const auto       sourceCodeLength = static_cast<GLint>(fragmentShaderCode.length());
        glCheck(GLEXT_glShaderSource(fragmentShader, 1, &sourceCode, &sourceCodeLength));
        glCheck(GLEXT_glCompileShader(fragmentShader));
        SFML_BASE_ASSERT(glIsShader(fragmentShader));

        // Check the compile log
        GLint success = 0;
        glCheck(GLEXT_glGetShaderParameteriv(fragmentShader, GLEXT_GL_OBJECT_COMPILE_STATUS, &success));
        if (success == GL_FALSE)
        {
            char log[1024];
            glCheck(GLEXT_glGetShaderInfoLog(fragmentShader, sizeof(log), nullptr, log));
            priv::err() << "Failed to compile fragment shader:" << '\n' << static_cast<const char*>(log);
            glCheck(GLEXT_glDeleteShader(fragmentShader));
            glCheck(GLEXT_glDeleteProgram(shaderProgram));
            return base::nullOpt;
        }

        // Attach the shader to the program, and delete it (not needed anymore)
        glCheck(GLEXT_glAttachShader(shaderProgram, fragmentShader));
        glCheck(GLEXT_glDeleteShader(fragmentShader));
    }

    // Link the program
    glCheck(GLEXT_glLinkProgram(shaderProgram));

    // Check the link log
    GLint success = 0;
    glCheck(GLEXT_glGetProgramParameteriv(shaderProgram, GLEXT_GL_OBJECT_LINK_STATUS, &success));
    if (success == GL_FALSE)
    {
        char log[1024];
        glCheck(GLEXT_glGetProgramInfoLog(shaderProgram, sizeof(log), nullptr, log));
        priv::err() << "Failed to link shader:" << '\n' << static_cast<const char*>(log);
        glCheck(GLEXT_glDeleteProgram(shaderProgram));
        return base::nullOpt;
    }

    // Force an OpenGL flush, so that the shader will appear updated
    // in all contexts immediately (solves problems in multi-threaded apps)
    glCheck(glFlush());

    return sf::base::makeOptional<Shader>(base::PassKey<Shader>{}, graphicsContext, castFromGlHandle(shaderProgram));
}


////////////////////////////////////////////////////////////
void Shader::bindTextures() const
{
    auto it = m_impl->textures.begin();
    for (std::size_t i = 0; i < m_impl->textures.size(); ++i)
    {
        const auto index = static_cast<GLsizei>(i + 1);
        glCheck(GLEXT_glUniform1i(it->first, index));
        glCheck(GLEXT_glActiveTexture(GLEXT_GL_TEXTURE0 + static_cast<GLenum>(index)));
        Texture::bind(*m_impl->graphicsContext, it->second);
        ++it;
    }

    // Make sure that the texture unit which is left active is the number 0
    glCheck(GLEXT_glActiveTexture(GLEXT_GL_TEXTURE0));
}

} // namespace sf
