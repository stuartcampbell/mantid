// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#include "MantidDataHandling/InjectMaterialsInto3MFFile.h"
#include "MantidAPI/FileProperty.h"
#include "MantidDataHandling/Mantid3MFFileIO.h"
#include "MantidKernel/Exception.h"
#include <fstream>
#include <string>

namespace Mantid {
namespace DataHandling {

// Register the algorithm into the algorithm factory
DECLARE_ALGORITHM(InjectMaterialsInto3MFFile)

using namespace Kernel;
using namespace API;

void InjectMaterialsInto3MFFile::init() {
  // Environment file
  const std::vector<std::string> extensions{".3mf"};
  declareProperty(
      std::make_unique<FileProperty>("Input Filename", "", FileProperty::Load,
                                     extensions),
      "The path name of the file that needs material information added to it");

  const std::vector<std::string> extensionsMaterials{".csv"};
  declareProperty(
      std::make_unique<FileProperty>("Material Information File", "",
                                     FileProperty::Load, extensionsMaterials),
      "The path name of the file containing the material information");

  declareProperty(std::make_unique<FileProperty>(
                      "Output Filename", "", FileProperty::Save, extensions),
                  "The name of the 3mf file to write, as a full or relative\n"
                  "path");
}

void InjectMaterialsInto3MFFile::exec() {
  const std::string filename = getProperty("Input Filename");
  const std::ifstream file(filename.c_str());
  if (!file) {
    g_log.error("Unable to open file: " + filename);
    throw Exception::FileError("Unable to open file: ", filename);
  }
  Mantid3MFFileIO MeshLoader;
  MeshLoader.LoadFile(filename);

  const std::string materialFilename = getProperty("Material Information File");
  std::ifstream materialFile(materialFilename.c_str());
  if (!materialFile) {
    g_log.error("Unable to open file: " + materialFilename);
    throw Exception::FileError("Unable to open file: ", materialFilename);
  }

  std::string line;
  int columnCount = 0;
  int currentRow = 0, currentColumn = 0;
  int partNameCol = -1, materialNameCol = -1, colorCol = -1,
      massDensityCol = -1;
  std::string partName, materialName, massDensity, materialNameExt;
  int colorRGB = 0, scanfvarpop;
  while (std::getline(materialFile, line)) {
    std::stringstream ss(line);
    std::string field;
    currentColumn = 0;
    while (std::getline(ss, field, ',')) {
      if (currentRow == 0) {
        if (field == "ObjectName") {
          partNameCol = columnCount;
        } else if (field == "MaterialName") {
          materialNameCol = columnCount;
        } else if (field == "Color") {
          colorCol = columnCount;
        } else if (field == "MassDensity") {
          massDensityCol = columnCount;
        }
        columnCount++;
      } else {
        if (currentColumn == partNameCol) {
          partName = field;
        } else if (currentColumn == materialNameCol) {
          materialName = field;
        } else if (currentColumn == colorCol) {
          scanfvarpop = sscanf(field.c_str(), "%x", &colorRGB);
          // colorRGB = std::stoi(colorRGBstr);
        } else if (currentColumn == massDensityCol) {
          massDensity = field;
        }
      }
      if ((massDensity != "") && (massDensity != "0")) {
        materialNameExt = materialName + " (MassDensity=" + massDensity + ')';
      } else {
        materialNameExt = materialName;
      }
      currentColumn++;
    }
    if (currentRow == 0) {
      if (partNameCol < 0) {
        throw Exception::ParseError("part name column not found",
                                    materialFilename, 0);
      }
      if (materialNameCol < 0) {
        throw Exception::ParseError("material name column not found",
                                    materialFilename, 0);
      }
      if (colorCol < 0) {
        throw Exception::ParseError("color column not found", materialFilename,
                                    0);
      }
    } else {
      MeshLoader.setMaterialOnObject(partName, materialNameExt, colorRGB);
    }
    currentRow++;
  }

  // MeshLoader.setMaterialOnObject("B-sdough", "Boron", 0xA0B0D0);

  const std::string outFilename = getProperty("Output Filename");
  MeshLoader.saveFile(outFilename);
} // namespace DataHandling

} // namespace DataHandling
} // namespace Mantid