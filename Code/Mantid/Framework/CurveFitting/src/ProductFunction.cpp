//----------------------------------------------------------------------
// Includes
//----------------------------------------------------------------------
#include "MantidCurveFitting/ProductFunction.h"
#include "MantidAPI/FunctionFactory.h"

namespace Mantid
{
namespace CurveFitting
{

DECLARE_FUNCTION(ProductFunction)

/** Function you want to fit to. 
 *  @param domain :: The buffer for writing the calculated values. Must be big enough to accept dataSize() values
 */
void ProductFunction::function(const API::FunctionDomain& domain, API::FunctionValues& values)const
{
  API::FunctionValues tmp(domain);
  values.setCalculated(1.0);
  for(size_t iFun = 0; iFun < nFunctions(); ++iFun)
  {
    domain.reset();
    getFunction( iFun )->function(domain,tmp);
    values *= tmp;
  }
}

} // namespace CurveFitting
} // namespace Mantid
