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
#include <SFML/Graphics/ConvexShape.hpp>

#include <SFML/Base/Assert.hpp>


namespace sf
{
////////////////////////////////////////////////////////////
ConvexShape::ConvexShape(std::size_t pointCount)
{
    setPointCount(pointCount);
}


////////////////////////////////////////////////////////////
void ConvexShape::setPointCount(std::size_t count)
{
    m_points.resize(count);
    update(m_points.data(), m_points.size());
}


////////////////////////////////////////////////////////////
std::size_t ConvexShape::getPointCount() const
{
    return m_points.size();
}


////////////////////////////////////////////////////////////
void ConvexShape::setPoint(std::size_t index, Vector2f point)
{
    SFML_BASE_ASSERT(index < m_points.size() && "Index is out of bounds");
    m_points[index] = point;
    update(m_points.data(), m_points.size());
}


////////////////////////////////////////////////////////////
Vector2f ConvexShape::getPoint(std::size_t index) const
{
    SFML_BASE_ASSERT(index < m_points.size() && "Index is out of bounds");
    return m_points[index];
}


////////////////////////////////////////////////////////////
Vector2f ConvexShape::getGeometricCenter() const
{
    const auto pointCount = m_points.size();

    switch (pointCount)
    {
        case 0:
            SFML_BASE_ASSERT(false && "Cannot calculate geometric center of shape with no points");
            return Vector2f{};
        case 1:
            return m_points[0];
        case 2:
            return (m_points[0] + m_points[1]) / 2.f;
        default: // more than two points
            Vector2f centroid;
            float    twiceArea = 0;

            auto previousPoint = m_points[pointCount - 1];
            for (std::size_t i = 0; i < pointCount; ++i)
            {
                const auto  currentPoint = m_points[i];
                const float product      = previousPoint.cross(currentPoint);
                twiceArea += product;
                centroid += (currentPoint + previousPoint) * product;

                previousPoint = currentPoint;
            }

            if (twiceArea != 0.f)
            {
                return centroid / 3.f / twiceArea;
            }

            // Fallback for no area - find the center of the bounding box
            auto minPoint = m_points[0];
            auto maxPoint = minPoint;
            for (std::size_t i = 1; i < pointCount; ++i)
            {
                const auto currentPoint = m_points[i];

                minPoint.x = std::min(minPoint.x, currentPoint.x);
                maxPoint.x = std::max(maxPoint.x, currentPoint.x);
                minPoint.y = std::min(minPoint.y, currentPoint.y);
                maxPoint.y = std::max(maxPoint.y, currentPoint.y);
            }
            return (maxPoint + minPoint) / 2.f;
    }
}

} // namespace sf
