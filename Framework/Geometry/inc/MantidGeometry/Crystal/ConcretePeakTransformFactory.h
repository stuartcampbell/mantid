// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include "MantidGeometry/Crystal/PeakTransformFactory.h"
#include <boost/make_shared.hpp>

namespace Mantid {
namespace Geometry {
/**
@class ConcretePeakTransformFactory
Concrete PeakTransformFactory producing PeakTransforms of type provided by type
arguement
*/
template <typename PeakTransformProduct>
class DLLExport ConcretePeakTransformFactory : public PeakTransformFactory {
public:
  /**
  Overriden Factory Method.
  @param xPlotLabel : X-axis plot label
  @param yPlotLabel : Y-axis plot label
  */
  PeakTransform_sptr
  createTransform(const std::string &xPlotLabel,
                  const std::string &yPlotLabel) const override {
    return boost::make_shared<PeakTransformProduct>(xPlotLabel, yPlotLabel);
  }

  /**
  Overriden Factory Method.
  */
  PeakTransform_sptr createDefaultTransform() const override {
    return boost::make_shared<PeakTransformProduct>();
  }
};
} // namespace Geometry
} // namespace Mantid
