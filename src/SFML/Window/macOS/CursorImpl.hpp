#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Cursor.hpp>

#include <SFML/System/Vector2.hpp>

////////////////////////////////////////////////////////////
// Predefine OBJ-C classes
////////////////////////////////////////////////////////////
#ifdef __OBJC__

@class NSCursor;
using NSCursorRef = NSCursor*;

#else // If C++

using NSCursorRef = void*;

#endif

namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief macOS implementation of Cursor
///
////////////////////////////////////////////////////////////
class CursorImpl
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// Refer to sf::Cursor::Cursor().
    ///
    ////////////////////////////////////////////////////////////
    CursorImpl() = default;

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    /// Refer to sf::Cursor::~Cursor().
    ///
    ////////////////////////////////////////////////////////////
    ~CursorImpl();

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    CursorImpl(const CursorImpl&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    CursorImpl& operator=(const CursorImpl&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    CursorImpl(CursorImpl&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    CursorImpl& operator=(CursorImpl&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Create a cursor with the provided image
    ///
    /// Refer to sf::Cursor::loadFromPixels().
    ///
    ////////////////////////////////////////////////////////////
    bool loadFromPixels(const std::uint8_t* pixels, Vector2u size, Vector2u hotspot);

    ////////////////////////////////////////////////////////////
    /// \brief Create a native system cursor
    ///
    /// Refer to sf::Cursor::loadFromSystem().
    ///
    ////////////////////////////////////////////////////////////
    bool loadFromSystem(Cursor::Type type);

private:
    friend class WindowImplCocoa;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    NSCursorRef m_cursor{}; ///< System cursor handle
};

} // namespace sf::priv
