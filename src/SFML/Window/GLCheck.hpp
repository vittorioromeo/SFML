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

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Config.hpp>

#include <SFML/Window/GLExtensions.hpp>

#include <SFML/Base/Assert.hpp>


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// Let's define a macro to quickly check every OpenGL API call
////////////////////////////////////////////////////////////
#ifdef SFML_DEBUG

// In debug mode, perform a test on every OpenGL call
// The do-while loop is needed so that glCheck can be used as a single statement in if/else branches
#define glCheck(expr)                                        \
    do                                                       \
    {                                                        \
        SFML_BASE_ASSERT(glGetError() == GL_NO_ERROR);       \
                                                             \
        expr;                                                \
        ::sf::priv::glCheckError(__FILE__, __LINE__, #expr); \
    } while (false)

#define glCheckExpr(expr)                                    \
    [&]                                                      \
    {                                                        \
        SFML_BASE_ASSERT(glGetError() == GL_NO_ERROR);       \
                                                             \
        auto result = expr;                                  \
        ::sf::priv::glCheckError(__FILE__, __LINE__, #expr); \
                                                             \
        return result;                                       \
    }()

#else

// Else, we don't add any overhead
#define glCheck(expr)     (expr)
#define glCheckExpr(expr) (expr)

#endif

////////////////////////////////////////////////////////////
/// \brief Check the last OpenGL error
///
/// \param file Source file where the call is located
/// \param line Line number of the source file where the call is located
/// \param expression The evaluated expression as a string
///
////////////////////////////////////////////////////////////
void glCheckError(const char* file, unsigned int line, const char* expression);

} // namespace sf::priv
