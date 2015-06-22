//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "MantidGeometry/Math/Quadrilateral.h"
#include "MantidKernel/Exception.h"

namespace Mantid {
namespace Geometry {
using Kernel::V2D;

//----------------------------------------------------------------------------------------------
/** Constructor
 */
Quadrilateral::Quadrilateral(const V2D &lowerLeft, const V2D &lowerRight,
                             const V2D &upperRight, const V2D &upperLeft)
    : ConvexPolygon(), m_lowerLeft(lowerLeft), m_lowerRight(lowerRight),
      m_upperRight(upperRight), m_upperLeft(upperLeft) {}

/**
 *  Special constructor for a rectangle
 */
Quadrilateral::Quadrilateral(const double lowerX, const double upperX,
                             const double lowerY, const double upperY)
    : ConvexPolygon(), m_lowerLeft(lowerX, lowerY),
      m_lowerRight(upperX, lowerY), m_upperRight(upperX, upperY),
      m_upperLeft(lowerX, upperY)

{}

/**
 * Copy constructor
 * @param other :: The object to construct this from
 */
Quadrilateral::Quadrilateral(const Quadrilateral &other)
    : ConvexPolygon(), m_lowerLeft(other.m_lowerLeft),
      m_lowerRight(other.m_lowerRight), m_upperRight(other.m_upperRight),
      m_upperLeft(other.m_upperLeft) {}

/**
 * @param rhs The source object to copy from
 */
Quadrilateral &Quadrilateral::operator=(const Quadrilateral &rhs) {
  if (this != &rhs) {
    m_lowerLeft = rhs.m_lowerLeft;
    m_lowerRight = rhs.m_lowerRight;
    m_upperRight = rhs.m_upperRight;
    m_upperLeft = rhs.m_upperLeft;
  }
  return *this;
}

/**
 * Return the vertex at the given index
 * @param index :: An index, starting at 0
 * @returns A reference to the polygon at that index
 * @throws Exception::IndexError if the index is out of range
 */
const V2D &Quadrilateral::operator[](const size_t index) const {
  if (index < npoints()) {
    switch (index) {
    case 0:
      return m_lowerLeft;
    case 1:
      return m_upperLeft;
    case 2:
      return m_upperRight;
    case 3:
      return m_lowerRight;
    }
  }
  throw Kernel::Exception::IndexError(index, npoints(),
                                      "Quadrilateral::operator[]");
}

/**
 * @return True if the point is inside the polygon or on the edge
 */
bool Quadrilateral::contains(const Kernel::V2D &point) const {
  return (minX() <= point.X() && point.X() <= maxX() && minY() <= point.Y() &&
          point.Y() <= maxY());
}

/**
 * @return True if the given polygon is completely encosed by this one
 */
bool Quadrilateral::contains(const ConvexPolygon &poly) const {
  return (minX() <= poly.minX() && poly.maxX() <= maxX() &&
          minY() <= poly.minY() && poly.maxY() <= maxY());
}

/**
 * Compute the area of the polygon using triangulation. As this is a
 * convex polygon the calculation is exact. The algorithm uses one vertex
 * as a common vertex and sums the areas of the triangles formed by this
 * and two other vertices, moving in an anti-clockwise direction.
 * @returns The area of the polygon
 */
double Quadrilateral::area() const {
  const double lhs =
      m_lowerLeft.Y() * m_upperLeft.X() + m_upperLeft.Y() * m_upperRight.X() +
      m_upperRight.Y() * m_lowerRight.X() + m_lowerRight.Y() * m_lowerLeft.X();
  const double rhs =
      m_lowerLeft.X() * m_upperLeft.Y() + m_upperLeft.X() * m_upperRight.Y() +
      m_upperRight.X() * m_lowerRight.Y() + m_lowerRight.X() * m_lowerLeft.Y();
  return 0.5 * (lhs - rhs);
}

/**
 * Compute the determinant of the set of points as if they were contained in
 * an (N+1)x(N+1) matrix where N=number of vertices. Each row contains the
 * [X,Y] values of the vertex padded with zeroes to the column length.
 * @returns The determinant of the set of points
 */
double Quadrilateral::determinant() const { return 2.0 * area(); }

/// @return The smallest X value for all points
double Quadrilateral::minX() const {
  return (m_lowerLeft.X() < m_upperLeft.X()) ? m_lowerLeft.X()
                                             : m_upperLeft.X();
}

/// @return The largest X value for all points
double Quadrilateral::maxX() const {
  return (m_lowerRight.X() > m_upperRight.X()) ? m_lowerRight.X()
                                               : m_upperRight.X();
}

/// @return The smallest Y value for all points
double Quadrilateral::minY() const {
  return (m_lowerLeft.Y() < m_lowerRight.Y()) ? m_lowerLeft.Y()
                                              : m_lowerRight.Y();
}

/// @return The largest Y value for all points
double Quadrilateral::maxY() const {
  return (m_upperLeft.Y() > m_upperRight.Y()) ? m_upperLeft.Y()
                                              : m_upperRight.Y();
}

} // namespace Mantid
} // namespace Geometry
