// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#ifndef DATAHANDLING_LOAD_ENVIRONMENT_COMPLEX_H_
#define DATAHANDLING_LOAD_ENVIRONMENT_COMPLEX_H_

#include "LoadShape.h"
#include "MantidAPI/Algorithm.h"
#include "MantidAPI/Sample.h"
#include "MantidKernel/Matrix.h"
#include <Poco/DOM/Element.h>

namespace Mantid {
namespace Geometry {
class MeshObject;
}
namespace DataHandling {

struct ComponentInfo {
  std::string STLFileName;
  std::string scale;
  std::string chemicalFormula;
  double sampleMassDensity = EMPTY_DBL();
  double xDegrees = 0;
  double yDegrees = 0;
  double zDegrees = 0;
  std::string translationVector;
};

/**
 * Load Environment into the sample of a workspace, either replacing the
 * current environment, or replacing it, you may also set the material
 *
 * The environment description is in the form of an xml file
 */

class DLLExport LoadSampleEnvironmentComplex : public Mantid::API::Algorithm {
public:
  /// Algorithm's name for identification overriding a virtual method
  const std::string name() const override {
    return "LoadSampleEnvironmentComplex";
  }
  /// Summary of algorithms purpose
  const std::string summary() const override {
    return "The algorithm loads an multi-part environment into the instrument "
           "of a "
           "workspace "
           "at the sample.";
  }

  /// Algorithm's version for identification overriding a virtual method
  int version() const override { return 1; }
  /// Related algorithms
  const std::vector<std::string> seeAlso() const override {
    return {"LoadSampleEnvironment", "CopySample", "SetSampleMaterial",
            "LoadSampleShape"};
  }
  /// Algorithm's category for identification overriding a virtual method
  const std::string category() const override {
    return "DataHandling\\Instrument";
  }

  boost::shared_ptr<Geometry::MeshObject>
  translate(boost::shared_ptr<Geometry::MeshObject> environmentMesh,
            ScaleUnits scaleType, ComponentInfo EnvComponent);
  boost::shared_ptr<Geometry::MeshObject>
  rotate(boost::shared_ptr<Geometry::MeshObject> environmentMesh,
         ComponentInfo EnvComponent);

  Kernel::Matrix<double> generateMatrix(ComponentInfo EnvComponent);
  Kernel::Matrix<double> generateXRotation(double xDegress);
  Kernel::Matrix<double> generateYRotation(double yDegrees);
  Kernel::Matrix<double> generateZRotation(double zDegrees);

private:
  std::vector<ComponentInfo> m_compElems;
  std::vector<double> m_globalOffset;
  // Implement abstract Algorithm methods
  void init() override;
  void exec() override;
  Kernel::V3D createScaledV3D(double xVal, double yVal, double zVal,
                              ScaleUnits scaleType);
  void parseXML(std::string filename);
  boost::shared_ptr<Geometry::MeshObject>
  loadSTLFileForComponent(const ComponentInfo &compElem);
  std::vector<double> parseTranslationVector(std::string translationVectorStr);
  void LoadOptionalDoubleFromXML(Poco::XML::Element *componentElement,
                          std::string elementName, double &targetVariable);
};

} // end namespace DataHandling
} // namespace Mantid

#endif /* DATAHANDLING_LOAD_ENVIRONMENT_COMPLEX_H_ */
