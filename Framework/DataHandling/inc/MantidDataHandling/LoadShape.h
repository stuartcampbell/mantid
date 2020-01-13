// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#include "MantidKernel/Logger.h"
#include "MantidKernel/V3D.h"

#ifndef MANTID_DATAHANDLING_LOADSHAPE_H_
#define MANTID_DATAHANDLING_LOADSHAPE_H_
namespace {
Mantid::Kernel::Logger g_log("LoadShape");
}
namespace Mantid {
namespace DataHandling {
enum class ScaleUnits { undefined, metres, centimetres, millimetres };

class DLLExport LoadShape {

protected:
  LoadShape(ScaleUnits scaleType) : m_scaleType(scaleType) {}
  Kernel::V3D createScaledV3D(double xVal, double yVal, double zVal) {
    /*switch (m_scaleType) {
    case ScaleUnits::centimetres:
      xVal = xVal / 100;
      yVal = yVal / 100;
      zVal = zVal / 100;
      break;
    case ScaleUnits::millimetres:
      xVal = xVal / 1000;
      yVal = yVal / 1000;
      zVal = zVal / 1000;
      break;
    case ScaleUnits::metres:
      break;
    }*/
    xVal = scaleValue(xVal);
    yVal = scaleValue(yVal);
    zVal = scaleValue(zVal);

    return Kernel::V3D(double(xVal), double(yVal), double(zVal));
  }

  double scaleValue(double val) {
    switch (m_scaleType) {
    case ScaleUnits::centimetres:
      return val / 100;
    case ScaleUnits::millimetres:
      return val / 1000;
    case ScaleUnits::metres:
      return val;
    default:
      return val;
    }
  }

  ScaleUnits m_scaleType;
  std::vector<uint32_t> m_triangle;
  std::vector<Kernel::V3D> m_vertices;

  inline void setScaleType(const ScaleUnits scaleType) {
    if (m_scaleType == ScaleUnits::undefined) {
      m_scaleType = scaleType;
    }
  }
};
inline ScaleUnits getScaleType(const std::string &scaleProperty) {
  ScaleUnits scaleType;
  if (scaleProperty == "m") {
    scaleType = ScaleUnits::metres;
  } else if (scaleProperty == "cm") {
    scaleType = ScaleUnits::centimetres;
  } else if (scaleProperty == "mm") {
    scaleType = ScaleUnits::millimetres;
  } else {
    throw std::invalid_argument(scaleProperty +
                                " is not an accepted scale of stl or 3mf file.");
  }
  return scaleType;
}

} // namespace DataHandling
} // namespace Mantid
#endif /*MANTID_DATAHANDLING_LOADSHAPE_H_*/