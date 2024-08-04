#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Export.hpp>

#include <SFML/System/Vector2.hpp>

#include <SFML/Base/InPlacePImpl.hpp>
#include <SFML/Base/Optional.hpp>
#include <SFML/Base/PassKey.hpp>

#include <cstdint>


namespace sf
{
namespace priv
{
class CursorImpl;
} // namespace priv

////////////////////////////////////////////////////////////
/// \brief Cursor defines the appearance of a system cursor
///
////////////////////////////////////////////////////////////
class SFML_WINDOW_API Cursor
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Enumeration of the native system cursor types
    ///
    /// Refer to the following table to determine which cursor
    /// is available on which platform.
    ///
    ///  Type                                     | Linux | macOS | Windows  |
    /// ------------------------------------------|:-----:|:-----:|:--------:|
    ///  sf::Cursor::Type::Arrow                  |  yes  | yes   |   yes    |
    ///  sf::Cursor::Type::ArrowWait              |  no   | no    |   yes    |
    ///  sf::Cursor::Type::Wait                   |  yes  | no    |   yes    |
    ///  sf::Cursor::Type::Text                   |  yes  | yes   |   yes    |
    ///  sf::Cursor::Type::Hand                   |  yes  | yes   |   yes    |
    ///  sf::Cursor::Type::SizeHorizontal         |  yes  | yes   |   yes    |
    ///  sf::Cursor::Type::SizeVertical           |  yes  | yes   |   yes    |
    ///  sf::Cursor::Type::SizeTopLeftBottomRight |  no   | yes*  |   yes    |
    ///  sf::Cursor::Type::SizeBottomLeftTopRight |  no   | yes*  |   yes    |
    ///  sf::Cursor::Type::SizeLeft               |  yes  | yes** |   yes**  |
    ///  sf::Cursor::Type::SizeRight              |  yes  | yes** |   yes**  |
    ///  sf::Cursor::Type::SizeTop                |  yes  | yes** |   yes**  |
    ///  sf::Cursor::Type::SizeBottom             |  yes  | yes** |   yes**  |
    ///  sf::Cursor::Type::SizeTopLeft            |  yes  | yes** |   yes**  |
    ///  sf::Cursor::Type::SizeTopRight           |  yes  | yes** |   yes**  |
    ///  sf::Cursor::Type::SizeBottomLeft         |  yes  | yes** |   yes**  |
    ///  sf::Cursor::Type::SizeBottomRight        |  yes  | yes** |   yes**  |
    ///  sf::Cursor::Type::SizeAll                |  yes  | no    |   yes    |
    ///  sf::Cursor::Type::Cross                  |  yes  | yes   |   yes    |
    ///  sf::Cursor::Type::Help                   |  yes  | yes*  |   yes    |
    ///  sf::Cursor::Type::NotAllowed             |  yes  | yes   |   yes    |
    ///
    ///  * These cursor types are undocumented so may not
    ///    be available on all versions, but have been tested on 10.13
    ///
    ///  ** On Windows and macOS, double-headed arrows are used
    ///
    ////////////////////////////////////////////////////////////
    enum class Type
    {
        Arrow,                  //!< Arrow cursor (default)
        ArrowWait,              //!< Busy arrow cursor
        Wait,                   //!< Busy cursor
        Text,                   //!< I-beam, cursor when hovering over a field allowing text entry
        Hand,                   //!< Pointing hand cursor
        SizeHorizontal,         //!< Horizontal double arrow cursor
        SizeVertical,           //!< Vertical double arrow cursor
        SizeTopLeftBottomRight, //!< Double arrow cursor going from top-left to bottom-right
        SizeBottomLeftTopRight, //!< Double arrow cursor going from bottom-left to top-right
        SizeLeft,               //!< Left arrow cursor on Linux, same as SizeHorizontal on other platforms
        SizeRight,              //!< Right arrow cursor on Linux, same as SizeHorizontal on other platforms
        SizeTop,                //!< Up arrow cursor on Linux, same as SizeVertical on other platforms
        SizeBottom,             //!< Down arrow cursor on Linux, same as SizeVertical on other platforms
        SizeTopLeft,            //!< Top-left arrow cursor on Linux, same as SizeTopLeftBottomRight on other platforms
        SizeBottomRight, //!< Bottom-right arrow cursor on Linux, same as SizeTopLeftBottomRight on other platforms
        SizeBottomLeft,  //!< Bottom-left arrow cursor on Linux, same as SizeBottomLeftTopRight on other platforms
        SizeTopRight,    //!< Top-right arrow cursor on Linux, same as SizeBottomLeftTopRight on other platforms
        SizeAll,         //!< Combination of SizeHorizontal and SizeVertical
        Cross,           //!< Crosshair cursor
        Help,            //!< Help cursor
        NotAllowed       //!< Action not allowed cursor
    };

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~Cursor();

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    Cursor(const Cursor&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    Cursor& operator=(const Cursor&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    Cursor(Cursor&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    Cursor& operator=(Cursor&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Create a cursor with the provided image
    ///
    /// \a pixels must be an array of \a width by \a height pixels
    /// in 32-bit RGBA format. If not, this will cause undefined behavior.
    ///
    /// If \a pixels is null or either \a width or \a height are 0,
    /// the current cursor is left unchanged and the function will
    /// return false.
    ///
    /// In addition to specifying the pixel data, you can also
    /// specify the location of the hotspot of the cursor. The
    /// hotspot is the pixel coordinate within the cursor image
    /// which will be located exactly where the mouse pointer
    /// position is. Any mouse actions that are performed will
    /// return the window/screen location of the hotspot.
    ///
    /// \warning On Unix platforms which do not support colored
    ///          cursors, the pixels are mapped into a monochrome
    ///          bitmap: pixels with an alpha channel to 0 are
    ///          transparent, black if the RGB channel are close
    ///          to zero, and white otherwise.
    ///
    /// \param pixels   Array of pixels of the image
    /// \param size     Width and height of the image
    /// \param hotspot  (x,y) location of the hotspot
    /// \return Cursor if the cursor was successfully loaded;
    ///         `base::nullOpt` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<Cursor> loadFromPixels(const std::uint8_t* pixels, Vector2u size, Vector2u hotspot);

    ////////////////////////////////////////////////////////////
    /// \brief Create a native system cursor
    ///
    /// Refer to the list of cursor available on each system
    /// (see sf::Cursor::Type) to know whether a given cursor is
    /// expected to load successfully or is not supported by
    /// the operating system.
    ///
    /// \param type Native system cursor type
    /// \return Cursor if and only if the corresponding cursor is
    ///         natively supported by the operating system;
    ///         `base::nullOpt` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static base::Optional<Cursor> loadFromSystem(Type type);

private:
    friend class WindowBase;

public:
    ////////////////////////////////////////////////////////////
    /// \private
    ///
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Cursor(base::PassKey<Cursor>&&);

private:
    ////////////////////////////////////////////////////////////
    /// \brief Get access to the underlying implementation
    ///
    /// This is primarily designed for sf::WindowBase::setMouseCursor,
    /// hence the friendship.
    ///
    /// \return a reference to the OS-specific implementation
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const priv::CursorImpl& getImpl() const;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<priv::CursorImpl, 32> m_impl; //!< Implementation details
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::Cursor
/// \ingroup window
///
/// \warning Features related to Cursor are not supported on
///          iOS and Android.
///
/// This class abstracts the operating system resources
/// associated with either a native system cursor or a custom
/// cursor.
///
/// After loading the cursor the graphical appearance
/// with either loadFromPixels() or loadFromSystem(), the
/// cursor can be changed with sf::WindowBase::setMouseCursor().
///
/// The behavior is undefined if the cursor is destroyed while
/// in use by the window.
///
/// Usage example:
/// \code
/// sf::Window window;
///
/// // ... create window as usual ...
///
/// const auto cursor = sf::Cursor::loadFromSystem(sf::Cursor::Type::Hand).value();
/// window.setMouseCursor(cursor);
/// \endcode
///
/// \see sf::WindowBase::setMouseCursor
///
////////////////////////////////////////////////////////////
