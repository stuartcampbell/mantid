// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#include "MantidDataHandling/LoadSampleEnvironmentComplex.h"
#include "MantidDataHandling/LoadAsciiStl.h"
#include "MantidDataHandling/LoadBinaryStl.h"
#include "MantidDataHandling/ReadMaterial.h"
#include "MantidGeometry/Instrument/Container.h"
#include "MantidGeometry/Instrument/SampleEnvironment.h"
#include "MantidGeometry/Objects/MeshObject.h"

#include "MantidAPI/FileProperty.h"
#include "MantidAPI/InstrumentValidator.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidAPI/Sample.h"

#include "MantidKernel/ArrayProperty.h"
#include "MantidKernel/BoundedValidator.h"
#include "MantidKernel/EnabledWhenProperty.h"
#include "MantidKernel/Exception.h"
#include "MantidKernel/ListValidator.h"
#include "MantidKernel/MandatoryValidator.h"
#include "MantidKernel/Strings.h"

#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/Document.h>
#include <Poco/DOM/Element.h>
#include <boost/algorithm/string.hpp>
#include <fstream>

namespace Mantid {
namespace DataHandling {

namespace {
double DegreesToRadians(double angle) { return angle * M_PI / 180; }
} // namespace

// Register the algorithm into the algorithm factory
DECLARE_ALGORITHM(LoadSampleEnvironmentComplex)

using namespace Kernel;
using namespace API;
using namespace Geometry;

void LoadSampleEnvironmentComplex::init() {
  auto wsValidator = boost::make_shared<InstrumentValidator>();
  // input workspace
  declareProperty(std::make_unique<WorkspaceProperty<>>(
                      "InputWorkspace", "", Direction::Input, wsValidator),
                  "The name of the workspace containing the instrument to add "
                  "the Environment");

  // Environment file
  const std::vector<std::string> extensions{".xml"};
  declareProperty(std::make_unique<FileProperty>(
                      "Filename", "", FileProperty::Load, extensions),
                  "The path name of the file containing the Environment");

  // Output workspace
  declareProperty(std::make_unique<WorkspaceProperty<>>("OutputWorkspace", "",
                                                        Direction::Output),
                  "The name of the workspace that will contain the loaded "
                  "Environment of the sample");

  // Environment Name
  declareProperty("EnvironmentName", "Environment");

  // New Can or Add
  declareProperty("Add", false);
}

void LoadSampleEnvironmentComplex::parseXML(std::string filename) {
  // Set up the DOM parser and parse xml file
  Poco::XML::DOMParser pParser;
  Poco::AutoPtr<Poco::XML::Document> pDoc;
  try {
    pDoc = pParser.parseString(Strings::loadFile(filename));
  } catch (Poco::Exception &exc) {
    throw std::invalid_argument(exc.displayText() + ". Unable to parse XML");
  } catch (...) {
    throw std::invalid_argument("Unable to parse XML");
  }

  // Get pointer to root element
  Poco::XML::Element *pRootElem = pDoc->documentElement();

  if (!pRootElem->hasChildNodes()) {
    g_log.error("XML file contains no root element.");
    throw Mantid::Kernel::Exception::FileError("No root element in XML file",
                                               filename);
  }

  auto globalOffsetElement = pRootElem->getChildElement("GlobalOffset");
  m_globalOffset = parseTranslationVector(
      globalOffsetElement->getChildElement("TranslationVector")->innerText());

  std::vector<Poco::XML::Element *> compElems;
  for (auto pNode = pRootElem->firstChild(); pNode != nullptr;
       pNode = pNode->nextSibling()) {
    auto pElem = dynamic_cast<Poco::XML::Element *>(pNode);
    if (pElem) {
      if (pElem->tagName() == "Component") {
        ComponentInfo EnvComponent = {};
        EnvComponent.STLFileName =
            pElem->getChildElement("STLFileName")->innerText();
        EnvComponent.scale = pElem->getChildElement("Scale")->innerText();
        EnvComponent.chemicalFormula =
            pElem->getChildElement("ChemicalFormula")->innerText();

        LoadOptionalDoubleFromXML(pElem, "SampleMassDensity",
                                  EnvComponent.sampleMassDensity);
        LoadOptionalDoubleFromXML(pElem, "XDegrees", EnvComponent.xDegrees);
        LoadOptionalDoubleFromXML(pElem, "YDegrees", EnvComponent.yDegrees);
        LoadOptionalDoubleFromXML(pElem, "ZDegrees", EnvComponent.zDegrees);

        EnvComponent.translationVector =
            pElem->getChildElement("TranslationVector")->innerText();
        m_compElems.emplace_back(EnvComponent);
      }
    }
  }
}

void LoadSampleEnvironmentComplex::LoadOptionalDoubleFromXML(
    Poco::XML::Element *componentElement, std::string elementName,
    double &targetVariable) {
  auto childElement = componentElement->getChildElement(elementName);
  if (childElement) {
    auto childElementText = childElement->innerText();
    if (!childElementText.empty()) {
      try {
        targetVariable = std::stod(childElement->innerText());
      } catch (std::invalid_argument &ex) {
        throw std::invalid_argument(
            std::string("Invalid string supplied for " + elementName + " ") +
            ex.what());
      }
    }
  }
}

boost::shared_ptr<MeshObject>
LoadSampleEnvironmentComplex::loadSTLFileForComponent(
    const ComponentInfo &compElem) {
  std::unique_ptr<LoadAsciiStl> asciiStlReader = nullptr;
  std::unique_ptr<LoadBinaryStl> binaryStlReader = nullptr;
  auto scaleProperty = compElem.scale;
  const std::string STLFileName = compElem.STLFileName;
  const ScaleUnits scaleType = getScaleType(scaleProperty);

  ReadMaterial::MaterialParameters params;
  params.chemicalSymbol = compElem.chemicalFormula;
  params.sampleMassDensity = compElem.sampleMassDensity;
  binaryStlReader =
      std::make_unique<LoadBinaryStl>(STLFileName, scaleType, params);
  asciiStlReader =
      std::make_unique<LoadAsciiStl>(STLFileName, scaleType, params);

  boost::shared_ptr<MeshObject> environmentMesh = nullptr;
  if (binaryStlReader->isBinarySTL(STLFileName)) {
    environmentMesh = binaryStlReader->readStl();
  } else if (asciiStlReader->isAsciiSTL(STLFileName)) {
    environmentMesh = asciiStlReader->readStl();
  } else {
    throw Exception::ParseError(
        "Could not read file, did not match either STL Format", STLFileName, 0);
  }

  environmentMesh = rotate(environmentMesh, compElem);
  environmentMesh = translate(environmentMesh, scaleType, compElem);

  auto translatedVertices = environmentMesh->getVertices();
  if (g_log.is(Logger::Priority::PRIO_DEBUG)) {
    int i = 0;
    for (double vertex : translatedVertices) {
      i++;
      g_log.debug(std::to_string(vertex));
      if (i % 3 == 0) {
        g_log.debug("\n");
      }
    }
  }

  return environmentMesh;
}

void LoadSampleEnvironmentComplex::exec() {

  MatrixWorkspace_const_sptr inputWS = getProperty("InputWorkspace");
  MatrixWorkspace_sptr outputWS = getProperty("OutputWorkspace");

  if (inputWS != outputWS) {
    outputWS = inputWS->clone();
  }

  const std::string filename = getProperty("Filename");
  const std::ifstream file(filename.c_str());
  if (!file) {
    g_log.error("Unable to open file: " + filename);
    throw Exception::FileError("Unable to open file: ", filename);
  }

  parseXML(filename);

  std::string name = getProperty("EnvironmentName");
  const bool add = getProperty("Add");
  Sample &sample = outputWS->mutableSample();
  std::unique_ptr<Geometry::SampleEnvironment> environment = nullptr;
  boost::shared_ptr<MeshObject> environmentMesh = nullptr;
  std::string debugString;

  for (auto compElem : m_compElems) {

    environmentMesh = loadSTLFileForComponent(compElem);

    if (!environment) {
      if (add) {
        environment =
            std::make_unique<SampleEnvironment>(sample.getEnvironment());
        environment->add(environmentMesh);
      } else {
        auto can = boost::make_shared<Container>(environmentMesh);
        environment = std::make_unique<SampleEnvironment>(name, can);
      }
    } else {
      environment->add(environmentMesh);
    }

    debugString +=
        "Environment has: " + std::to_string(environment->nelements()) +
        " elements.";
  }

  // Put Environment into sample.
  sample.setEnvironment(std::move(environment));

  // get the material name and number density for debug
  const auto outMaterial =
      outputWS->sample().getEnvironment().getContainer().material();
  debugString += "\n"
                 "Environment Material: " +
                 outMaterial.name();
  debugString += "\n"
                 "Environment Material Number Density: " +
                 std::to_string(outMaterial.numberDensity());
  // Set output workspace
  setProperty("OutputWorkspace", outputWS);
  g_log.debug(debugString);
}

std::vector<double> LoadSampleEnvironmentComplex::parseTranslationVector(
    std::string translationVectorStr) {
  std::vector<double> translationVector;

  // Split up comma-separated properties
  using tokenizer = Mantid::Kernel::StringTokenizer;
  tokenizer values(translationVectorStr, ",",
                   tokenizer::TOK_IGNORE_EMPTY | tokenizer::TOK_TRIM);

  translationVector.clear();
  translationVector.reserve(values.count());
  std::transform(
      values.cbegin(), values.cend(), std::back_inserter(translationVector),
      [](const std::string &str) { return boost::lexical_cast<double>(str); });
  return translationVector;
}

/**
 * translates the environment by a provided matrix
 * @param environmentMesh The environment to translate
 * @param scaleType The scale to use
 * @returns a shared pointer to the newly translated environment
 */
boost::shared_ptr<MeshObject> LoadSampleEnvironmentComplex::translate(
    boost::shared_ptr<MeshObject> environmentMesh, ScaleUnits scaleType,
    ComponentInfo EnvComponent) {
  std::vector<double> translationVector =
      parseTranslationVector(EnvComponent.translationVector);
  std::transform(translationVector.begin(), translationVector.end(),
                 m_globalOffset.begin(), translationVector.begin(),
                 std::plus<double>());

  std::vector<double> checkVector = std::vector<double>(3, 0.0);
  if (translationVector != checkVector) {
    if (translationVector.size() != 3) {
      throw std::invalid_argument(
          "Invalid Translation vector, must have exactly 3 dimensions");
    }
    V3D translate = createScaledV3D(translationVector[0], translationVector[1],
                                    translationVector[2], scaleType);
    environmentMesh->translate(translate);
  }
  return environmentMesh;
}

/**
 * Rotates the environment by a generated matrix
 * @param environmentMesh The environment to rotate
 * @returns a shared pointer to the newly rotated environment
 */
boost::shared_ptr<MeshObject> LoadSampleEnvironmentComplex::rotate(
    boost::shared_ptr<MeshObject> environmentMesh, ComponentInfo EnvComponent) {
  const std::vector<double> rotationMatrix = generateMatrix(EnvComponent);
  environmentMesh->rotate(rotationMatrix);
  return environmentMesh;
}

/**
 * Generates a rotation Matrix applying the x rotation then y rotation, then z
 * rotation
 * @returns a matrix of doubles to use as the rotation matrix
 */
Matrix<double>
LoadSampleEnvironmentComplex::generateMatrix(ComponentInfo EnvComponent) {
  Kernel::Matrix<double> xMatrix = generateXRotation(EnvComponent.xDegrees);
  Kernel::Matrix<double> yMatrix = generateYRotation(EnvComponent.yDegrees);
  Kernel::Matrix<double> zMatrix = generateZRotation(EnvComponent.zDegrees);

  return zMatrix * yMatrix * xMatrix;
}

/**
 * Generates the x component of the rotation matrix
 * using the xDegrees Property.
 * @returns a matrix of doubles to use as the x axis rotation matrix
 */
Matrix<double>
LoadSampleEnvironmentComplex::generateXRotation(double xRotation) {
  auto xRotationInRad = DegreesToRadians(xRotation);
  const double sinX = sin(xRotationInRad);
  const double cosX = cos(xRotationInRad);
  std::vector<double> matrixList = {1, 0, 0, 0, cosX, -sinX, 0, sinX, cosX};
  return Kernel::Matrix<double>(matrixList);
}

/**
 * Generates the y component of the rotation matrix
 * using the yDegrees Property.
 * @returns a matrix of doubles to use as the y axis rotation matrix
 */
Matrix<double>
LoadSampleEnvironmentComplex::generateYRotation(double yRotation) {
  auto yRotationInRad = DegreesToRadians(yRotation);
  const double sinY = sin(yRotationInRad);
  const double cosY = cos(yRotationInRad);
  std::vector<double> matrixList = {cosY, 0, sinY, 0, 1, 0, -sinY, 0, cosY};
  return Kernel::Matrix<double>(matrixList);
}

/**
 * Generates the z component of the rotation matrix
 * using the zDegrees Property.
 * @returns a matrix of doubles to use as the z axis rotation matrix
 */
Matrix<double>
LoadSampleEnvironmentComplex::generateZRotation(double zRotation) {
  auto zRotationInRad = DegreesToRadians(zRotation);
  const double sinZ = sin(zRotationInRad);
  const double cosZ = cos(zRotationInRad);
  std::vector<double> matrixList = {cosZ, -sinZ, 0, sinZ, cosZ, 0, 0, 0, 1};
  return Kernel::Matrix<double>(matrixList);
}

Kernel::V3D LoadSampleEnvironmentComplex::createScaledV3D(
    double xVal, double yVal, double zVal, ScaleUnits scaleType) {
  switch (scaleType) {
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
  }
  return Kernel::V3D(double(xVal), double(yVal), double(zVal));
}

} // namespace DataHandling
} // namespace Mantid
