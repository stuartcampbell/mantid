// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2015 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include "MantidAlgorithms/TimeAtSampleStrategy.h"
#include "MantidKernel/V3D.h"
#include <boost/shared_ptr.hpp>

namespace Mantid {

namespace API {
class MatrixWorkspace;
class SpectrumInfo;
} // namespace API
namespace Algorithms {

/** TimeAtSampleStrategyElastic : Time at sample stragegy for elastic scattering
 */
class MANTID_ALGORITHMS_DLL TimeAtSampleStrategyElastic
    : public Mantid::Algorithms::TimeAtSampleStrategy {
public:
  TimeAtSampleStrategyElastic(
      boost::shared_ptr<const Mantid::API::MatrixWorkspace> ws);
  Correction calculate(const size_t &workspace_index) const override;

private:
  boost::shared_ptr<const Mantid::API::MatrixWorkspace> m_ws;
  const API::SpectrumInfo &m_spectrumInfo;
  const Kernel::V3D m_beamDir;
};

} // namespace Algorithms
} // namespace Mantid
