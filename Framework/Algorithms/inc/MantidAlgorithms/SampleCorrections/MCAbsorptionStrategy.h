// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2016 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#ifndef MANTID_ALGORITHMS_MCABSORPTIONSTRATEGY_H_
#define MANTID_ALGORITHMS_MCABSORPTIONSTRATEGY_H_

#include "MantidAlgorithms/DllConfig.h"
#include "MantidAlgorithms/SampleCorrections/MCInteractionVolume.h"
#include "MantidHistogramData/Histogram.h"
#include "MantidKernel/DeltaEMode.h"
#include <tuple>

namespace Mantid {
namespace API {
class Sample;
}
namespace Kernel {
class PseudoRandomNumberGenerator;
class V3D;
} // namespace Kernel
namespace Algorithms {
class IBeamProfile;

/**
  Implements the algorithm for calculating the correction factor for
  self-attenuation and single wavelength using a Monte Carlo. A single
  instance has a fixed nominal source position, nominal sample
  position & sample + containers shapes.

  The error on all points is defined to be \f$\frac{1}{\sqrt{N}}\f$, where N is
  the number of events generated.
*/
class MANTID_ALGORITHMS_DLL MCAbsorptionStrategy {
public:
  MCAbsorptionStrategy(Kernel::PseudoRandomNumberGenerator &rng,
                       const IBeamProfile &beamProfile,
                       const API::Sample &sample, size_t nevents,
                       size_t maxScatterPtAttempts, bool useCaching,
                       int nDetectors);
  void initialise(Kernel::PseudoRandomNumberGenerator &rng,
                  const Kernel::V3D &finalPos, int detectorID);
  std::tuple<double, double> calculate(Kernel::PseudoRandomNumberGenerator &rng,
                                       const Kernel::V3D &finalPos,
                                       int detectorID, double lambdaBefore,
                                       double lambdaAfter);
  void calculateAllLambdas(
      Kernel::PseudoRandomNumberGenerator &rng, const Kernel::V3D &finalPos,
      int detectorID, Mantid::Kernel::DeltaEMode::Type EMode,
      Mantid::HistogramData::Points lambdas, const int lambdaStepSize,
      double lambdaFixed, Mantid::HistogramData::HistogramY &attenuationFactors,
      std::string& debugString);

private:
  const IBeamProfile &m_beamProfile;
  MCInteractionVolume m_scatterVol;
  const size_t m_nevents;
  const size_t m_maxScatterAttempts;
  const double m_error;
  const bool m_useCaching;
  std::map<size_t, int> m_attemptsCounts;
};

} // namespace Algorithms
} // namespace Mantid

#endif /* MANTID_ALGORITHMS_MCABSORPTIONSTRATEGY_H_ */
