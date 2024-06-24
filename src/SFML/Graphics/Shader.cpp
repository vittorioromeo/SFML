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
#include <SFML/Graphics/GLCheck.hpp>
#include <SFML/Graphics/GLExtensions.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <SFML/System/Err.hpp>
#include <SFML/System/InputStream.hpp>
#include <SFML/System/Utils.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>

#include <fstream>
#include <iomanip>
#include <optional>
#include <ostream>
#include <utility>
#include <vector>

#include <cassert>
#include <cstdint>

#ifndef SFML_OPENGL_ES

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
std::size_t getMaxTextureUnits()
{
    static const GLint maxUnits = []
    {
        GLint value = 0;
        glCheck(glGetIntegerv(GLEXT_GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &value));

        return value;
    }();

    return static_cast<std::size_t>(maxUnits);
}

// Pair of indices into thread-local buffer
struct BufferSlice
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
std::optional<BufferSlice> appendFileContentsToVector(const std::filesystem::path& filename, std::vector<char>& buffer)
{
    std::ifstream file(filename, std::ios_base::binary);

    if (!file)
    {
        sf::err() << "Failed to open shader file" << std::endl;
        return std::nullopt;
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
    return std::make_optional<BufferSlice>(bufferSizeBeforeRead, buffer.size() - bufferSizeBeforeRead);
}

// Read the contents of a stream into an array of char
std::optional<BufferSlice> appendStreamContentsToVector(sf::InputStream& stream, std::vector<char>& buffer)
{
    const std::optional<std::size_t> size = stream.getSize();

    if (!size.has_value() || size.value() == 0)
    {
        buffer.push_back('\0');
        return std::nullopt;
    }

    const std::size_t bufferSizeBeforeRead = buffer.size();
    buffer.resize(*size + bufferSizeBeforeRead);

    if (stream.seek(0) == -1)
    {
        sf::err() << "Failed to seek shader stream" << std::endl;
        return std::nullopt;
    }

    const std::optional<std::size_t> read = stream.read(buffer.data() + bufferSizeBeforeRead, *size);

    if (!read.has_value() || *read != size)
    {
        sf::err() << "Failed to read stream contents into buffer" << std::endl;
        return std::nullopt;
    }

    buffer.push_back('\0');
    return std::make_optional<BufferSlice>(bufferSizeBeforeRead, buffer.size() - bufferSizeBeforeRead);
}

// Return a thread-local vector for suitable use as a temporary buffer
// This function is non-reentrant
std::vector<char>& getThreadLocalCharBuffer()
{
    thread_local std::vector<char> result;
    return result;
}

// Transforms an array of 2D vectors into a contiguous array of scalars
std::vector<float> flatten(const sf::Vector2f* vectorArray, std::size_t length)
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
std::vector<float> flatten(const sf::Vector3f* vectorArray, std::size_t length)
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
std::vector<float> flatten(const sf::Glsl::Vec4* vectorArray, std::size_t length)
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
} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
struct Shader::UniformBinder
{
    ////////////////////////////////////////////////////////////
    /// \brief Constructor: set up state before uniform is set
    ///
    ////////////////////////////////////////////////////////////
    UniformBinder(Shader& shader, const std::string& name) : currentProgram(castToGlHandle(shader.m_shaderProgram))
    {
        if (currentProgram)
        {
            // Enable program object
            glCheck(savedProgram = GLEXT_glGetHandle(GLEXT_GL_PROGRAM_OBJECT));
            if (currentProgram != savedProgram)
                glCheck(GLEXT_glUseProgramObject(currentProgram));

            // Store uniform location for further use outside constructor
            location = shader.getUniformLocation(name);
        }
    }

    ////////////////////////////////////////////////////////////
    /// \brief Destructor: restore state after uniform is set
    ///
    ////////////////////////////////////////////////////////////
    ~UniformBinder()
    {
        // Disable program object
        if (currentProgram && (currentProgram != savedProgram))
            glCheck(GLEXT_glUseProgramObject(savedProgram));
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

    TransientContextLock lock;           //!< Lock to keep context active while uniform is bound
    GLEXT_GLhandle       savedProgram{}; //!< Handle to the previously active program object
    GLEXT_GLhandle       currentProgram; //!< Handle to the program object of the modified sf::Shader instance
    GLint                location{-1};   //!< Uniform location, used by the surrounding sf::Shader code
};


////////////////////////////////////////////////////////////
struct Shader::UnsafeUniformBinder
{
    ////////////////////////////////////////////////////////////
    /// \brief Constructor: set up state before uniform is set
    ///
    ////////////////////////////////////////////////////////////
    UnsafeUniformBinder(Shader& shader, const std::string& name) : currentProgram(shader.m_shaderProgram)
    {
        if (currentProgram)
        {
            // Enable program object
            GLint temp;
            glCheck(glGetIntegerv(GL_CURRENT_PROGRAM, &temp));
            savedProgram = static_cast<GLuint>(temp);

            if (currentProgram != savedProgram)
                glCheck(glUseProgram(currentProgram));

            // Store uniform location for further use outside constructor
            location = shader.getUniformLocation(name);
        }
    }

    ////////////////////////////////////////////////////////////
    /// \brief Destructor: restore state after uniform is set
    ///
    ////////////////////////////////////////////////////////////
    ~UnsafeUniformBinder()
    {
        // Disable program object
        if (currentProgram && (currentProgram != savedProgram))
            glCheck(glUseProgram(savedProgram));
    }

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    UnsafeUniformBinder(const UnsafeUniformBinder&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    UnsafeUniformBinder& operator=(const UnsafeUniformBinder&) = delete;

    GLuint savedProgram{}; //!< Handle to the previously active program object
    GLuint currentProgram; //!< Handle to the program object of the modified sf::Shader instance
    GLint  location{-1};   //!< Uniform location, used by the surrounding sf::Shader code
};


////////////////////////////////////////////////////////////
Shader::~Shader()
{
    const TransientContextLock lock;

    // Destroy effect program
    if (m_shaderProgram)
        glCheck(GLEXT_glDeleteObject(castToGlHandle(m_shaderProgram)));
}

////////////////////////////////////////////////////////////
Shader::Shader(Shader&& source) noexcept :
m_shaderProgram(std::exchange(source.m_shaderProgram, 0U)),
m_currentTexture(std::exchange(source.m_currentTexture, -1)),
m_textures(std::move(source.m_textures)),
m_uniforms(std::move(source.m_uniforms))
{
}


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
        const TransientContextLock lock;
        assert(m_shaderProgram);
        glCheck(GLEXT_glDeleteObject(castToGlHandle(m_shaderProgram)));
    }

    // Move the contents of right.
    m_shaderProgram  = std::exchange(right.m_shaderProgram, 0U);
    m_currentTexture = std::exchange(right.m_currentTexture, -1);
    m_textures       = std::move(right.m_textures);
    m_uniforms       = std::move(right.m_uniforms);
    return *this;
}


////////////////////////////////////////////////////////////
std::optional<Shader> Shader::loadFromFile(const std::filesystem::path& filename, Type type)
{
    // Prepare thread-local buffer
    std::vector<char>& buffer = getThreadLocalCharBuffer();
    buffer.clear();

    // Read the file
    const std::optional<BufferSlice> shaderSlice = appendFileContentsToVector(filename, buffer);
    if (!shaderSlice.has_value())
    {
        err() << "Failed to open shader file\n" << formatDebugPathInfo(filename) << std::endl;
        return std::nullopt;
    }

    const std::string_view shaderView = shaderSlice->toView(buffer);

    // Compile the shader program
    if (type == Type::Vertex)
        return compile(shaderView, {}, {});
    else if (type == Type::Geometry)
        return compile({}, shaderView, {});
    else
        return compile({}, {}, shaderView);
}


////////////////////////////////////////////////////////////
std::optional<Shader> Shader::loadFromFile(const std::filesystem::path& vertexShaderFilename,
                                           const std::filesystem::path& fragmentShaderFilename)
{
    // Prepare thread-local buffer
    std::vector<char>& buffer = getThreadLocalCharBuffer();
    buffer.clear();

    // Read the vertex shader file
    const std::optional<BufferSlice> vertexShaderSlice = appendFileContentsToVector(vertexShaderFilename, buffer);
    if (!vertexShaderSlice.has_value())
    {
        err() << "Failed to open vertex shader file\n" << formatDebugPathInfo(vertexShaderFilename) << std::endl;
        return std::nullopt;
    }

    // Read the fragment shader file
    const std::optional<BufferSlice> fragmentShaderSlice = appendFileContentsToVector(fragmentShaderFilename, buffer);
    if (!fragmentShaderSlice.has_value())
    {
        err() << "Failed to open fragment shader file\n" << formatDebugPathInfo(fragmentShaderFilename) << std::endl;
        return std::nullopt;
    }

    // Compile the shader program
    return compile(vertexShaderSlice->toView(buffer), {}, fragmentShaderSlice->toView(buffer));
}


////////////////////////////////////////////////////////////
std::optional<Shader> Shader::loadFromFile(const std::filesystem::path& vertexShaderFilename,
                                           const std::filesystem::path& geometryShaderFilename,
                                           const std::filesystem::path& fragmentShaderFilename)
{
    // Prepare thread-local buffer
    std::vector<char>& buffer = getThreadLocalCharBuffer();
    buffer.clear();

    // Read the vertex shader file
    const std::optional<BufferSlice> vertexShaderSlice = appendFileContentsToVector(vertexShaderFilename, buffer);
    if (!vertexShaderSlice.has_value())
    {
        err() << "Failed to open vertex shader file\n" << formatDebugPathInfo(vertexShaderFilename) << std::endl;
        return std::nullopt;
    }

    // Read the geometry shader file
    const std::optional<BufferSlice> geometryShaderSlice = appendFileContentsToVector(geometryShaderFilename, buffer);
    if (!geometryShaderSlice.has_value())
    {
        err() << "Failed to open geometry shader file\n" << formatDebugPathInfo(geometryShaderFilename) << std::endl;
        return std::nullopt;
    }

    // Read the fragment shader file
    const std::optional<BufferSlice> fragmentShaderSlice = appendFileContentsToVector(fragmentShaderFilename, buffer);
    if (!fragmentShaderSlice.has_value())
    {
        err() << "Failed to open fragment shader file\n" << formatDebugPathInfo(fragmentShaderFilename) << std::endl;
        return std::nullopt;
    }

    // Compile the shader program
    return compile(vertexShaderSlice->toView(buffer),
                   geometryShaderSlice->toView(buffer),
                   fragmentShaderSlice->toView(buffer));
}


////////////////////////////////////////////////////////////
std::optional<Shader> Shader::loadFromMemory(std::string_view shader, Type type)
{
    // Compile the shader program
    if (type == Type::Vertex)
        return compile(shader, {}, {});
    else if (type == Type::Geometry)
        return compile({}, shader, {});
    else
        return compile({}, {}, shader);
}


////////////////////////////////////////////////////////////
std::optional<Shader> Shader::loadFromMemory(std::string_view vertexShader, std::string_view fragmentShader)
{
    // Compile the shader program
    return compile(vertexShader, {}, fragmentShader);
}


////////////////////////////////////////////////////////////
std::optional<Shader> Shader::loadFromMemory(std::string_view vertexShader,
                                             std::string_view geometryShader,
                                             std::string_view fragmentShader)
{
    // Compile the shader program
    return compile(vertexShader, geometryShader, fragmentShader);
}


////////////////////////////////////////////////////////////
std::optional<Shader> Shader::loadFromStream(InputStream& stream, Type type)
{
    // Prepare thread-local buffer
    std::vector<char>& buffer = getThreadLocalCharBuffer();
    buffer.clear();

    // Read the shader code from the stream
    const std::optional<BufferSlice> shaderSlice = appendStreamContentsToVector(stream, buffer);
    if (!shaderSlice.has_value())
    {
        err() << "Failed to read vertex shader from stream" << std::endl;
        return std::nullopt;
    }

    const std::string_view shaderView = shaderSlice->toView(buffer);

    // Compile the shader program
    if (type == Type::Vertex)
        return compile(shaderView, {}, {});
    else if (type == Type::Geometry)
        return compile({}, shaderView, {});
    else
        return compile({}, {}, shaderView);
}


////////////////////////////////////////////////////////////
std::optional<Shader> Shader::loadFromStream(InputStream& vertexShaderStream, InputStream& fragmentShaderStream)
{
    // Prepare thread-local buffer
    std::vector<char>& buffer = getThreadLocalCharBuffer();
    buffer.clear();

    // Read the vertex shader code from the stream
    const std::optional<BufferSlice> vertexShaderSlice = appendStreamContentsToVector(vertexShaderStream, buffer);
    if (!vertexShaderSlice.has_value())
    {
        err() << "Failed to read vertex shader from stream" << std::endl;
        return std::nullopt;
    }

    // Read the fragment shader code from the stream
    const std::optional<BufferSlice> fragmentShaderSlice = appendStreamContentsToVector(fragmentShaderStream, buffer);
    if (!fragmentShaderSlice.has_value())
    {
        err() << "Failed to read fragment shader from stream" << std::endl;
        return std::nullopt;
    }

    // Compile the shader program
    return compile(vertexShaderSlice->toView(buffer), {}, fragmentShaderSlice->toView(buffer));
}


////////////////////////////////////////////////////////////
std::optional<Shader> Shader::loadFromStream(InputStream& vertexShaderStream,
                                             InputStream& geometryShaderStream,
                                             InputStream& fragmentShaderStream)
{
    // Prepare thread-local buffer
    std::vector<char>& buffer = getThreadLocalCharBuffer();
    buffer.clear();

    // Read the vertex shader code from the stream
    const std::optional<BufferSlice> vertexShaderSlice = appendStreamContentsToVector(vertexShaderStream, buffer);
    if (!vertexShaderSlice.has_value())
    {
        err() << "Failed to read vertex shader from stream" << std::endl;
        return std::nullopt;
    }

    // Read the geometry shader code from the stream
    const std::optional<BufferSlice> geometryShaderSlice = appendStreamContentsToVector(geometryShaderStream, buffer);
    if (!geometryShaderSlice.has_value())
    {
        err() << "Failed to read geometry shader from stream" << std::endl;
        return std::nullopt;
    }

    // Read the fragment shader code from the stream
    const std::optional<BufferSlice> fragmentShaderSlice = appendStreamContentsToVector(fragmentShaderStream, buffer);
    if (!fragmentShaderSlice.has_value())
    {
        err() << "Failed to read fragment shader from stream" << std::endl;
        return std::nullopt;
    }

    // Compile the shader program
    return compile(vertexShaderSlice->toView(buffer),
                   geometryShaderSlice->toView(buffer),
                   fragmentShaderSlice->toView(buffer));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& name, float x)
{
    const UniformBinder binder(*this, name);
    if (binder.location != -1)
        glCheck(GLEXT_glUniform1f(binder.location, x));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& name, const Glsl::Vec2& v)
{
    const UniformBinder binder(*this, name);
    if (binder.location != -1)
        glCheck(GLEXT_glUniform2f(binder.location, v.x, v.y));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& name, const Glsl::Vec3& v)
{
    const UniformBinder binder(*this, name);
    if (binder.location != -1)
        glCheck(GLEXT_glUniform3f(binder.location, v.x, v.y, v.z));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& name, const Glsl::Vec4& v)
{
    const UniformBinder binder(*this, name);
    if (binder.location != -1)
        glCheck(GLEXT_glUniform4f(binder.location, v.x, v.y, v.z, v.w));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& name, int x)
{
    const UniformBinder binder(*this, name);
    if (binder.location != -1)
        glCheck(GLEXT_glUniform1i(binder.location, x));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& name, const Glsl::Ivec2& v)
{
    const UniformBinder binder(*this, name);
    if (binder.location != -1)
        glCheck(GLEXT_glUniform2i(binder.location, v.x, v.y));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& name, const Glsl::Ivec3& v)
{
    const UniformBinder binder(*this, name);
    if (binder.location != -1)
        glCheck(GLEXT_glUniform3i(binder.location, v.x, v.y, v.z));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& name, const Glsl::Ivec4& v)
{
    const UniformBinder binder(*this, name);
    if (binder.location != -1)
        glCheck(GLEXT_glUniform4i(binder.location, v.x, v.y, v.z, v.w));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& name, bool x)
{
    setUniform(name, static_cast<int>(x));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& name, const Glsl::Bvec2& v)
{
    setUniform(name, Glsl::Ivec2(v));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& name, const Glsl::Bvec3& v)
{
    setUniform(name, Glsl::Ivec3(v));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& name, const Glsl::Bvec4& v)
{
    setUniform(name, Glsl::Ivec4(v));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& name, const Glsl::Mat3& matrix)
{
    const UniformBinder binder(*this, name);
    if (binder.location != -1)
        glCheck(GLEXT_glUniformMatrix3fv(binder.location, 1, GL_FALSE, matrix.array));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& name, const Glsl::Mat4& matrix)
{
    const UniformBinder binder(*this, name);
    if (binder.location != -1)
        glCheck(GLEXT_glUniformMatrix4fv(binder.location, 1, GL_FALSE, matrix.array));
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& name, const Texture& texture)
{
    assert(m_shaderProgram);

    const TransientContextLock lock;

    // Find the location of the variable in the shader
    const int location = getUniformLocation(name);
    if (location != -1)
    {
        // Store the location -> texture mapping
        const auto it = m_textures.find(location);
        if (it == m_textures.end())
        {
            // New entry, make sure there are enough texture units
            if (m_textures.size() + 1 >= getMaxTextureUnits())
            {
                err() << "Impossible to use texture " << std::quoted(name)
                      << " for shader: all available texture units are used" << std::endl;
                return;
            }

            m_textures[location] = &texture;
        }
        else
        {
            // Location already used, just replace the texture
            it->second = &texture;
        }
    }
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& name, CurrentTextureType)
{
    assert(m_shaderProgram);

    const TransientContextLock lock;

    // Find the location of the variable in the shader
    m_currentTexture = getUniformLocation(name);
}


////////////////////////////////////////////////////////////
void Shader::setUniformArray(const std::string& name, const float* scalarArray, std::size_t length)
{
    const UniformBinder binder(*this, name);
    if (binder.location != -1)
        glCheck(GLEXT_glUniform1fv(binder.location, static_cast<GLsizei>(length), scalarArray));
}


////////////////////////////////////////////////////////////
void Shader::setUniformArray(const std::string& name, const Glsl::Vec2* vectorArray, std::size_t length)
{
    std::vector<float> contiguous = flatten(vectorArray, length);

    const UniformBinder binder(*this, name);
    if (binder.location != -1)
        glCheck(GLEXT_glUniform2fv(binder.location, static_cast<GLsizei>(length), contiguous.data()));
}


////////////////////////////////////////////////////////////
void Shader::setUniformArray(const std::string& name, const Glsl::Vec3* vectorArray, std::size_t length)
{
    std::vector<float> contiguous = flatten(vectorArray, length);

    const UniformBinder binder(*this, name);
    if (binder.location != -1)
        glCheck(GLEXT_glUniform3fv(binder.location, static_cast<GLsizei>(length), contiguous.data()));
}


////////////////////////////////////////////////////////////
void Shader::setUniformArray(const std::string& name, const Glsl::Vec4* vectorArray, std::size_t length)
{
    std::vector<float> contiguous = flatten(vectorArray, length);

    const UniformBinder binder(*this, name);
    if (binder.location != -1)
        glCheck(GLEXT_glUniform4fv(binder.location, static_cast<GLsizei>(length), contiguous.data()));
}


////////////////////////////////////////////////////////////
void Shader::setUniformArray(const std::string& name, const Glsl::Mat3* matrixArray, std::size_t length)
{
    const std::size_t matrixSize = 3 * 3;

    std::vector<float> contiguous(matrixSize * length);
    for (std::size_t i = 0; i < length; ++i)
        priv::copyMatrix(matrixArray[i].array, matrixSize, &contiguous[matrixSize * i]);

    const UniformBinder binder(*this, name);
    if (binder.location != -1)
        glCheck(GLEXT_glUniformMatrix3fv(binder.location, static_cast<GLsizei>(length), GL_FALSE, contiguous.data()));
}


////////////////////////////////////////////////////////////
void Shader::setUniformArray(const std::string& name, const Glsl::Mat4* matrixArray, std::size_t length)
{
    const std::size_t matrixSize = 4 * 4;

    std::vector<float> contiguous(matrixSize * length);
    for (std::size_t i = 0; i < length; ++i)
        priv::copyMatrix(matrixArray[i].array, matrixSize, &contiguous[matrixSize * i]);

    const UniformBinder binder(*this, name);
    if (binder.location != -1)
        glCheck(GLEXT_glUniformMatrix4fv(binder.location, static_cast<GLsizei>(length), GL_FALSE, contiguous.data()));
}


////////////////////////////////////////////////////////////
void Shader::setUniformUnsafe(const std::string& name, float x)
{
    UnsafeUniformBinder binder(*this, name);
    if (binder.location != -1)
        glCheck(GLEXT_glUniform1f(binder.location, x));
}


////////////////////////////////////////////////////////////
void Shader::setUniformUnsafe(const std::string& name, const Glsl::Vec2& v)
{
    UnsafeUniformBinder binder(*this, name);
    if (binder.location != -1)
        glCheck(GLEXT_glUniform2f(binder.location, v.x, v.y));
}


////////////////////////////////////////////////////////////
void Shader::setUniformUnsafe(const std::string& name, const Glsl::Vec3& v)
{
    UnsafeUniformBinder binder(*this, name);
    if (binder.location != -1)
        glCheck(GLEXT_glUniform3f(binder.location, v.x, v.y, v.z));
}


////////////////////////////////////////////////////////////
void Shader::setUniformUnsafe(const std::string& name, const Glsl::Vec4& v)
{
    UnsafeUniformBinder binder(*this, name);
    if (binder.location != -1)
        glCheck(GLEXT_glUniform4f(binder.location, v.x, v.y, v.z, v.w));
}


////////////////////////////////////////////////////////////
void Shader::setUniformUnsafe(const std::string& name, int x)
{
    UnsafeUniformBinder binder(*this, name);
    if (binder.location != -1)
        glCheck(GLEXT_glUniform1i(binder.location, x));
}


////////////////////////////////////////////////////////////
void Shader::setUniformUnsafe(const std::string& name, const Glsl::Ivec2& v)
{
    UnsafeUniformBinder binder(*this, name);
    if (binder.location != -1)
        glCheck(GLEXT_glUniform2i(binder.location, v.x, v.y));
}


////////////////////////////////////////////////////////////
void Shader::setUniformUnsafe(const std::string& name, const Glsl::Ivec3& v)
{
    UnsafeUniformBinder binder(*this, name);
    if (binder.location != -1)
        glCheck(GLEXT_glUniform3i(binder.location, v.x, v.y, v.z));
}


////////////////////////////////////////////////////////////
void Shader::setUniformUnsafe(const std::string& name, const Glsl::Ivec4& v)
{
    UnsafeUniformBinder binder(*this, name);
    if (binder.location != -1)
        glCheck(GLEXT_glUniform4i(binder.location, v.x, v.y, v.z, v.w));
}


////////////////////////////////////////////////////////////
unsigned int Shader::getNativeHandle() const
{
    return m_shaderProgram;
}


////////////////////////////////////////////////////////////
void Shader::bind(const Shader* shader)
{
    const TransientContextLock lock;

    // Make sure that we can use shaders
    if (!isAvailable())
    {
        err() << "Failed to bind or unbind shader: your system doesn't support shaders "
              << "(you should test Shader::isAvailable() before trying to use the Shader class)" << std::endl;
        return;
    }

    if (shader && shader->m_shaderProgram)
    {
        // Enable the program
        glCheck(GLEXT_glUseProgramObject(castToGlHandle(shader->m_shaderProgram)));

        // Bind the textures
        shader->bindTextures();

        // Bind the current texture
        if (shader->m_currentTexture != -1)
            glCheck(GLEXT_glUniform1i(shader->m_currentTexture, 0));
    }
    else
    {
        // Bind no shader
        glCheck(GLEXT_glUseProgramObject({}));
    }
}


////////////////////////////////////////////////////////////
bool Shader::isAvailable()
{
    static const bool available = []
    {
        const TransientContextLock contextLock;

        // Make sure that extensions are initialized
        priv::ensureExtensionsInit();

        return GLEXT_multitexture && GLEXT_shading_language_100 && GLEXT_shader_objects && GLEXT_vertex_shader &&
               GLEXT_fragment_shader;
    }();

    return available;
}


////////////////////////////////////////////////////////////
bool Shader::isGeometryAvailable()
{
    static const bool available = []
    {
        const TransientContextLock contextLock;

        // Make sure that extensions are initialized
        priv::ensureExtensionsInit();

        return isAvailable() && (GLEXT_geometry_shader4 || GLEXT_GL_VERSION_3_2);
    }();

    return available;
}


////////////////////////////////////////////////////////////
Shader::Shader(priv::PassKey<Shader>&&, unsigned int shaderProgram) : m_shaderProgram(shaderProgram)
{
}


////////////////////////////////////////////////////////////
std::optional<Shader> Shader::compile(std::string_view vertexShaderCode,
                                      std::string_view geometryShaderCode,
                                      std::string_view fragmentShaderCode)
{
    const TransientContextLock lock;

    // First make sure that we can use shaders
    if (!isAvailable())
    {
        err() << "Failed to create a shader: your system doesn't support shaders "
              << "(you should test Shader::isAvailable() before trying to use the Shader class)" << std::endl;
        return std::nullopt;
    }

    // Make sure we can use geometry shaders
    if (geometryShaderCode.data() && !isGeometryAvailable())
    {
        err() << "Failed to create a shader: your system doesn't support geometry shaders "
              << "(you should test Shader::isGeometryAvailable() before trying to use geometry shaders)" << std::endl;
        return std::nullopt;
    }

    // Create the program
    GLEXT_GLhandle shaderProgram{};
    glCheck(shaderProgram = GLEXT_glCreateProgramObject());

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

        // Check the compile log
        GLint success = 0;
        glCheck(GLEXT_glGetObjectParameteriv(vertexShader, GLEXT_GL_OBJECT_COMPILE_STATUS, &success));
        if (success == GL_FALSE)
        {
            char log[1024];
            glCheck(GLEXT_glGetInfoLog(vertexShader, sizeof(log), nullptr, log));
            err() << "Failed to compile vertex shader:" << '\n' << log << std::endl;
            glCheck(GLEXT_glDeleteObject(vertexShader));
            glCheck(GLEXT_glDeleteObject(shaderProgram));
            return std::nullopt;
        }

        // Attach the shader to the program, and delete it (not needed anymore)
        glCheck(GLEXT_glAttachObject(shaderProgram, vertexShader));
        glCheck(GLEXT_glDeleteObject(vertexShader));
    }

    // Create the geometry shader if needed
    if (geometryShaderCode.data())
    {
        // Create and compile the shader
        const GLEXT_GLhandle geometryShader   = GLEXT_glCreateShaderObject(GLEXT_GL_GEOMETRY_SHADER);
        const GLcharARB*     sourceCode       = geometryShaderCode.data();
        const auto           sourceCodeLength = static_cast<GLint>(geometryShaderCode.length());
        glCheck(GLEXT_glShaderSource(geometryShader, 1, &sourceCode, &sourceCodeLength));
        glCheck(GLEXT_glCompileShader(geometryShader));

        // Check the compile log
        GLint success = 0;
        glCheck(GLEXT_glGetObjectParameteriv(geometryShader, GLEXT_GL_OBJECT_COMPILE_STATUS, &success));
        if (success == GL_FALSE)
        {
            char log[1024];
            glCheck(GLEXT_glGetInfoLog(geometryShader, sizeof(log), nullptr, log));
            err() << "Failed to compile geometry shader:" << '\n' << log << std::endl;
            glCheck(GLEXT_glDeleteObject(geometryShader));
            glCheck(GLEXT_glDeleteObject(shaderProgram));
            return std::nullopt;
        }

        // Attach the shader to the program, and delete it (not needed anymore)
        glCheck(GLEXT_glAttachObject(shaderProgram, geometryShader));
        glCheck(GLEXT_glDeleteObject(geometryShader));
    }

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

        // Check the compile log
        GLint success = 0;
        glCheck(GLEXT_glGetObjectParameteriv(fragmentShader, GLEXT_GL_OBJECT_COMPILE_STATUS, &success));
        if (success == GL_FALSE)
        {
            char log[1024];
            glCheck(GLEXT_glGetInfoLog(fragmentShader, sizeof(log), nullptr, log));
            err() << "Failed to compile fragment shader:" << '\n' << log << std::endl;
            glCheck(GLEXT_glDeleteObject(fragmentShader));
            glCheck(GLEXT_glDeleteObject(shaderProgram));
            return std::nullopt;
        }

        // Attach the shader to the program, and delete it (not needed anymore)
        glCheck(GLEXT_glAttachObject(shaderProgram, fragmentShader));
        glCheck(GLEXT_glDeleteObject(fragmentShader));
    }

    // Link the program
    glCheck(GLEXT_glLinkProgram(shaderProgram));

    // Check the link log
    GLint success = 0;
    glCheck(GLEXT_glGetObjectParameteriv(shaderProgram, GLEXT_GL_OBJECT_LINK_STATUS, &success));
    if (success == GL_FALSE)
    {
        char log[1024];
        glCheck(GLEXT_glGetInfoLog(shaderProgram, sizeof(log), nullptr, log));
        err() << "Failed to link shader:" << '\n' << log << std::endl;
        glCheck(GLEXT_glDeleteObject(shaderProgram));
        return std::nullopt;
    }

    // Force an OpenGL flush, so that the shader will appear updated
    // in all contexts immediately (solves problems in multi-threaded apps)
    glCheck(glFlush());

    return std::make_optional<Shader>(priv::PassKey<Shader>{}, castFromGlHandle(shaderProgram));
}


////////////////////////////////////////////////////////////
void Shader::bindTextures() const
{
    auto it = m_textures.begin();
    for (std::size_t i = 0; i < m_textures.size(); ++i)
    {
        const auto index = static_cast<GLsizei>(i + 1);
        glCheck(GLEXT_glUniform1i(it->first, index));
        glCheck(GLEXT_glActiveTexture(GLEXT_GL_TEXTURE0 + static_cast<GLenum>(index)));
        Texture::bind(it->second);
        ++it;
    }

    // Make sure that the texture unit which is left active is the number 0
    glCheck(GLEXT_glActiveTexture(GLEXT_GL_TEXTURE0));
}


////////////////////////////////////////////////////////////
int Shader::getUniformLocation(const std::string& name)
{
    // Check the cache
    if (const auto it = m_uniforms.find(name); it != m_uniforms.end())
    {
        // Already in cache, return it
        return it->second;
    }
    else
    {
        // Not in cache, request the location from OpenGL
        const int location = GLEXT_glGetUniformLocation(castToGlHandle(m_shaderProgram), name.c_str());
        m_uniforms.emplace(name, location);

        if (location == -1)
            err() << "Uniform " << std::quoted(name) << " not found in shader" << std::endl;

        return location;
    }
}

} // namespace sf

#else // SFML_OPENGL_ES

// OpenGL ES 1 doesn't support GLSL shaders at all, we have to provide an empty implementation

namespace sf
{
////////////////////////////////////////////////////////////
Shader::~Shader() = default;


////////////////////////////////////////////////////////////
Shader::Shader(Shader&& source) noexcept = default;


////////////////////////////////////////////////////////////
Shader& Shader::operator=(Shader&& right) noexcept = default;


////////////////////////////////////////////////////////////
std::optional<Shader> Shader::loadFromFile(const std::filesystem::path& /* filename */, Type /* type */)
{
    return std::nullopt;
}


////////////////////////////////////////////////////////////
std::optional<Shader> Shader::loadFromFile(const std::filesystem::path& /* vertexShaderFilename */,
                                           const std::filesystem::path& /* fragmentShaderFilename */)
{
    return std::nullopt;
}


////////////////////////////////////////////////////////////
std::optional<Shader> Shader::loadFromFile(const std::filesystem::path& /* vertexShaderFilename */,
                                           const std::filesystem::path& /* geometryShaderFilename */,
                                           const std::filesystem::path& /* fragmentShaderFilename */)
{
    return std::nullopt;
}


////////////////////////////////////////////////////////////
std::optional<Shader> Shader::loadFromMemory(std::string_view /* shader */, Type /* type */)
{
    return std::nullopt;
}


////////////////////////////////////////////////////////////
std::optional<Shader> Shader::loadFromMemory(std::string_view /* vertexShader */, std::string_view /* fragmentShader */)
{
    return std::nullopt;
}


////////////////////////////////////////////////////////////
std::optional<Shader> Shader::loadFromMemory(std::string_view /* vertexShader */,
                                             std::string_view /* geometryShader */,
                                             std::string_view /* fragmentShader */)
{
    return std::nullopt;
}


////////////////////////////////////////////////////////////
std::optional<Shader> Shader::loadFromStream(InputStream& /* stream */, Type /* type */)
{
    return std::nullopt;
}


////////////////////////////////////////////////////////////
std::optional<Shader> Shader::loadFromStream(InputStream& /* vertexShaderStream */, InputStream& /* fragmentShaderStream */)
{
    return std::nullopt;
}


////////////////////////////////////////////////////////////
std::optional<Shader> Shader::loadFromStream(InputStream& /* vertexShaderStream */,
                                             InputStream& /* geometryShaderStream */,
                                             InputStream& /* fragmentShaderStream */)
{
    return std::nullopt;
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& /* name */, float)
{
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& /* name */, const Glsl::Vec2&)
{
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& /* name */, const Glsl::Vec3&)
{
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& /* name */, const Glsl::Vec4&)
{
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& /* name */, int)
{
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& /* name */, const Glsl::Ivec2&)
{
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& /* name */, const Glsl::Ivec3&)
{
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& /* name */, const Glsl::Ivec4&)
{
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& /* name */, bool)
{
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& /* name */, const Glsl::Bvec2&)
{
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& /* name */, const Glsl::Bvec3&)
{
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& /* name */, const Glsl::Bvec4&)
{
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& /* name */, const Glsl::Mat3& /* matrix */)
{
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& /* name */, const Glsl::Mat4& /* matrix */)
{
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& /* name */, const Texture& /* texture */)
{
}


////////////////////////////////////////////////////////////
void Shader::setUniform(const std::string& /* name */, CurrentTextureType)
{
}


////////////////////////////////////////////////////////////
void Shader::setUniformArray(const std::string& /* name */, const float* /* scalarArray */, std::size_t /* length */)
{
}


////////////////////////////////////////////////////////////
void Shader::setUniformArray(const std::string& /* name */, const Glsl::Vec2* /* vectorArray */, std::size_t /* length */)
{
}


////////////////////////////////////////////////////////////
void Shader::setUniformArray(const std::string& /* name */, const Glsl::Vec3* /* vectorArray */, std::size_t /* length */)
{
}


////////////////////////////////////////////////////////////
void Shader::setUniformArray(const std::string& /* name */, const Glsl::Vec4* /* vectorArray */, std::size_t /* length */)
{
}


////////////////////////////////////////////////////////////
void Shader::setUniformArray(const std::string& /* name */, const Glsl::Mat3* /* matrixArray */, std::size_t /* length */)
{
}


////////////////////////////////////////////////////////////
void Shader::setUniformArray(const std::string& /* name */, const Glsl::Mat4* /* matrixArray */, std::size_t /* length */)
{
}


////////////////////////////////////////////////////////////
unsigned int Shader::getNativeHandle() const
{
    return 0;
}


////////////////////////////////////////////////////////////
void Shader::bind(const Shader* /* shader */)
{
}


////////////////////////////////////////////////////////////
bool Shader::isAvailable()
{
    return false;
}


////////////////////////////////////////////////////////////
bool Shader::isGeometryAvailable()
{
    return false;
}


////////////////////////////////////////////////////////////
Shader::Shader(priv::PassKey<Shader>&&, unsigned int shaderProgram) : m_shaderProgram(shaderProgram)
{
}


////////////////////////////////////////////////////////////
std::optional<Shader> Shader::compile(std::string_view /* vertexShaderCode */,
                                      std::string_view /* geometryShaderCode */,
                                      std::string_view /* fragmentShaderCode */)
{
    return std::nullopt;
}


////////////////////////////////////////////////////////////
void Shader::bindTextures() const
{
}

} // namespace sf

#endif // SFML_OPENGL_ES
