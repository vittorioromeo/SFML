#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Export.hpp"

#include "SFML/Graphics/Transform.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/AutoWrapAngle.hpp"
#include "SFML/System/Rect.hpp"
#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Assert.hpp"


namespace sf
{
// TODO P0:
struct [[nodiscard]] ScissorRect : FloatRect
{
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr explicit ScissorRect(Vector2f thePosition, Vector2f theSize) :
    FloatRect{thePosition, theSize}
    {
        SFML_BASE_ASSERT(position.x >= 0.0f && position.x <= 1.0f && "position.x must lie within [0, 1]");
        SFML_BASE_ASSERT(position.y >= 0.0f && position.y <= 1.0f && "position.y must lie within [0, 1]");
        SFML_BASE_ASSERT(size.x >= 0.0f && "size.x must lie within [0, 1]");
        SFML_BASE_ASSERT(size.y >= 0.0f && "size.y must lie within [0, 1]");
        SFML_BASE_ASSERT(position.x + size.x <= 1.0f && "position.x + size.x must lie within [0, 1]");
        SFML_BASE_ASSERT(position.y + size.y <= 1.0f && "position.y + size.y must lie within [0, 1]");
    }

    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr explicit(false) ScissorRect(const FloatRect& rect) :
    ScissorRect{rect.position, rect.size}
    {
    }
};


////////////////////////////////////////////////////////////
/// \brief 2D camera that defines what region is shown on screen
///
////////////////////////////////////////////////////////////
struct [[nodiscard]] SFML_GRAPHICS_API View
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// This constructor creates a default view of (0, 0, 1000, 1000)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] View() = default;

    ////////////////////////////////////////////////////////////
    /// \brief Construct the view from a rectangle
    ///
    /// \param rectangle Rectangle defining the zone to display
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit View(const FloatRect& rectangle);

    ////////////////////////////////////////////////////////////
    /// \brief Construct the view from its center and size
    ///
    /// \param theCenter Center of the zone to display
    /// \param theSize   Size of zone to display
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit View(Vector2f theCenter, Vector2f theSize);

    ////////////////////////////////////////////////////////////
    /// \brief Move the view relatively to its current position
    ///
    /// \param offset Move offset
    ///
    /// \see `setCenter`, `rotate`, `zoom`
    ///
    ////////////////////////////////////////////////////////////
    void move(Vector2f offset);

    ////////////////////////////////////////////////////////////
    /// \brief Rotate the view relatively to its current orientation
    ///
    /// \param angle Angle to rotate
    ///
    /// \see `setRotation`, `move`, `zoom`
    ///
    ////////////////////////////////////////////////////////////
    void rotate(Angle angle);

    ////////////////////////////////////////////////////////////
    /// \brief Resize the view rectangle relatively to its current size
    ///
    /// Resizing the view simulates a zoom, as the zone displayed on
    /// screen grows or shrinks.
    /// \a factor is a multiplier:
    /// \li 1 keeps the size unchanged
    /// \li > 1 makes the view bigger (objects appear smaller)
    /// \li < 1 makes the view smaller (objects appear bigger)
    ///
    /// \param factor Zoom factor to apply
    ///
    /// \see `setSize`, `move`, `rotate`
    ///
    ////////////////////////////////////////////////////////////
    void zoom(float factor);

    ////////////////////////////////////////////////////////////
    /// \brief Get the projection transform of the view
    ///
    /// This function is meant for internal use only.
    ///
    /// \return Projection transform defining the view
    ///
    /// \see `getInverseTransform`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] Transform getTransform() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the inverse projection transform of the view
    ///
    /// This function is meant for internal use only.
    ///
    /// \return Inverse of the projection transform defining the view
    ///
    /// \see `getTransform`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::pure]] Transform getInverseTransform() const;

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    Vector2f      center{500.f, 500.f};         //!< Center of the view, in scene coordinates
    Vector2f      size{1000.f, 1000.f};         //!< Size of the view, in scene coordinates
    AutoWrapAngle rotation;                     //!< Angle of rotation of the view rectangle
    FloatRect viewport{{0.f, 0.f}, {1.f, 1.f}}; //!< Viewport rectangle, expressed as a factor of the render-target's size
    ScissorRect scissor{{0.f, 0.f}, {1.f, 1.f}}; //!< Scissor rectangle, expressed as a factor of the render-target's size
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::View
/// \ingroup graphics
///
/// `sf::View` defines a camera in the 2D scene. This is a
/// very powerful concept: you can scroll, rotate or zoom
/// the entire scene without altering the way that your
/// drawable objects are drawn.
///
/// A view is composed of a source rectangle, which defines
/// what part of the 2D scene is shown, and a target viewport,
/// which defines where the contents of the source rectangle
/// will be displayed on the render target (window or texture).
///
/// The viewport allows to map the scene to a custom part
/// of the render target, and can be used for split-screen
/// or for displaying a minimap, for example. If the source
/// rectangle doesn't have the same size as the viewport, its
/// contents will be stretched to fit in.
///
/// The scissor rectangle allows for specifying regions of the
/// render target to which modifications can be made by draw
/// and clear operations. Only pixels that are within the region
/// will be able to be modified. Pixels outside of the region will
/// not be modified by draw or clear operations.
///
/// Certain effects can be created by either using the viewport or
/// scissor rectangle. While the results appear identical, there
/// can be times where one method should be preferred over the other.
/// Viewport transformations are applied during the vertex processing
/// stage of the graphics pipeline, before the primitives are
/// rasterized into fragments for fragment processing. Since
/// viewport processing has to be performed and cannot be disabled,
/// effects that are performed using the viewport transform are
/// basically free performance-wise. Scissor testing is performed in
/// the per-sample processing stage of the graphics pipeline, after
/// fragment processing has been performed. Because per-sample
/// processing is performed at the last stage of the pipeline,
/// fragments that are discarded at this stage will cause the
/// highest waste of GPU resources compared to any method that
/// would have discarded vertices or fragments earlier in the
/// pipeline. There are situations in which scissor testing has
/// to be used to control whether fragments are discarded or not.
/// An example of such a situation is when performing the viewport
/// transform on vertices is necessary but a subset of the generated
/// fragments should not have an effect on the stencil buffer or
/// blend with the color buffer.
//
/// To apply a view, you have to assign it to the render target.
/// Then, objects drawn in this render target will be
/// affected by the view until you use another view.
///
/// Usage example:
/// \code
/// sf::RenderWindow window;
///
/// // Initialize the view to a rectangle located at (100, 100) and with a size of 400x200
/// sf::View view(sf::FloatRect({100, 100}, {400, 200}));
///
/// // Rotate it by 45 degrees
/// view.rotate(sf::degrees(45));
///
/// // Set its target viewport to be half of the window
/// view.setViewport(sf::FloatRect({0.f, 0.f}, {0.5f, 1.f}));
///
/// // Apply it
/// window.setView(view);
///
/// // Render stuff
/// window.draw(someSprite);
///
/// // Set the default view back
/// window.setView(window.getDefaultView());
///
/// // Render stuff not affected by the view
/// window.draw(someText);
/// \endcode
///
/// See also the note on coordinates and undistorted rendering in `sf::Transformable`.
///
/// \see `sf::RenderWindow`, `sf::RenderTexture`
///
////////////////////////////////////////////////////////////
