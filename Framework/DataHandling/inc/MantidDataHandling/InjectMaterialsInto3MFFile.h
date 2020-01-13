// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2020 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#ifndef DATAHANDLING_INJECT_MATERIALS_3MF_H_
#define DATAHANDLING_INJECT_MATERIALS_3MF_H_

#include "MantidAPI/Algorithm.h"

namespace Mantid {
namespace DataHandling {

class DLLExport InjectMaterialsInto3MFFile : public Mantid::API::Algorithm {
public:
  /// Algorithm's name for identification overriding a virtual method
  const std::string name() const override {
    return "InjectMaterialsInto3MFFile";
  }
  /// Summary of algorithms purpose
  const std::string summary() const override {
    return "The algorithm loads an existing .3mf file "
           "and adds materials for each part in the"
           ".3mf file based on information provided"
           "in a separate csv file.";
  }

  /// Algorithm's version for identification overriding a virtual method
  int version() const override { return 1; }
  /// Related algorithms
  const std::vector<std::string> seeAlso() const override {
    return {"LoadSampleEnvironmentComplex"};
  }
  /// Algorithm's category for identification overriding a virtual method
  const std::string category() const override {
    return "DataHandling\\Instrument";
  }

private:
  // Implement abstract Algorithm methods
  void init() override;
  void exec() override;
};

} // end namespace DataHandling
} // namespace Mantid

#endif /* DATAHANDLING_INJECT_MATERIALS_3MF_H_ */