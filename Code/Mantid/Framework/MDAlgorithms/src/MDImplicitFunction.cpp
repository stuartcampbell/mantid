#include "MantidMDAlgorithms/MDImplicitFunction.h"
#include "MantidKernel/System.h"

namespace Mantid
{
namespace MDAlgorithms
{


  //----------------------------------------------------------------------------------------------
  /** Constructor
   */
  MDImplicitFunction::MDImplicitFunction()
  : m_nd(0)
  {
  }
    
  //----------------------------------------------------------------------------------------------
  /** Destructor
   */
  MDImplicitFunction::~MDImplicitFunction()
  {
  }
  

  //----------------------------------------------------------------------------------------------
  /** Add a bounded plane to this implicit function
   *
   * @param plane :: MDPlane to add.
   */
  void MDImplicitFunction::addPlane(const MDPlane & plane)
  {
    // Number of dimensions must match
    if (m_planes.size() > 0)
    {
      if (m_nd != plane.getNumDims())
        throw std::invalid_argument("MDImplicitFunction::addPlane(): cannot add a plane with different number of dimensions as the previous ones.");
    }
    m_planes.push_back(plane);
    m_nd = plane.getNumDims();
    m_numPlanes = m_planes.size();
  }




} // namespace Mantid
} // namespace MDAlgorithms

