// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#include "MantidAlgorithms/SampleCorrections/MCAbsorptionStrategy.h"
#include "MantidAPI/Sample.h"
#include "MantidAlgorithms/SampleCorrections/IBeamProfile.h"
#include "MantidKernel/PseudoRandomNumberGenerator.h"
#include "MantidKernel/V3D.h"

#include "MantidAlgorithms/SampleCorrections/RectangularBeamProfile.h"
#include "MantidGeometry/Objects/CSGObject.h"

namespace Mantid {
using Kernel::DeltaEMode;
using Kernel::PseudoRandomNumberGenerator;

namespace Algorithms {

/**
 * Constructor
 * @param beamProfile A reference to the object the beam profile
 * @param sample A reference to the object defining details of the sample
 * @param nevents The number of Monte Carlo events used in the simulation
 * @param maxScatterPtAttempts The maximum number of tries to generate a random
 * point within the object.
 */
MCAbsorptionStrategy::MCAbsorptionStrategy(
    Kernel::PseudoRandomNumberGenerator &rng, const IBeamProfile &beamProfile,
    const API::Sample &sample, size_t nevents, size_t maxScatterPtAttempts,
    bool useCaching, int nDetectors)
    : m_beamProfile(beamProfile),
      m_scatterVol(
          MCInteractionVolume(sample, beamProfile.defineActiveRegion(sample))),
      m_nevents(nevents), m_maxScatterAttempts(maxScatterPtAttempts),
      m_error(1.0 / std::sqrt(m_nevents)), m_useCaching(useCaching) {

  m_scatterVol.resetActiveElements(Geometry::scatterBeforeAfter::scBefore,
                                   false, nDetectors);
  m_scatterVol.resetActiveElements(Geometry::scatterBeforeAfter::scScatter,
                                   false, nDetectors);
  m_scatterVol.resetActiveElements(Geometry::scatterBeforeAfter::scAfter, false,
                                   nDetectors);
  // populate the caches for the before scatter track and scatter point
  if (m_useCaching) {
    const auto scatterBounds = m_scatterVol.getBoundingBox();
    for (size_t i = 0; i < m_nevents; ++i) {
      m_scatterVol.addActiveElementsForScatterPoint(rng);
    }
    for (size_t i = 0; i < m_nevents; ++i) {
      const auto neutron = m_beamProfile.generatePoint(rng, scatterBounds);
      m_scatterVol.addActiveElementsForBeforeScatter(rng, neutron.startPos);
    }
  }
}

void MCAbsorptionStrategy::initialise(Kernel::PseudoRandomNumberGenerator &rng,
                                      const Kernel::V3D &finalPos,
                                      int detectorID) {
  if (m_useCaching) {
    for (size_t i = 0; i < m_nevents; ++i) {
      m_scatterVol.addActiveElementsForScatterPoint(rng);
    }

    for (size_t i = 0; i < m_nevents; ++i) {
      m_scatterVol.addActiveElementsForAfterScatter(rng, finalPos, detectorID);
    }
  }
}

/**
 * Compute the correction for a final position of the neutron and wavelengths
 * before and after scattering
 * @param rng A reference to a PseudoRandomNumberGenerator
 * @param finalPos Defines the final position of the neutron, assumed to be
 * where it is detected
 * @param lambdaBefore Wavelength, in \f$\\A^-1\f$, before scattering
 * @param lambdaAfter Wavelength, in \f$\\A^-1\f$, after scattering
 * @return A tuple of the <correction factor, associated error>.
 */
std::tuple<double, double>
MCAbsorptionStrategy::calculate(Kernel::PseudoRandomNumberGenerator &rng,
                                const Kernel::V3D &finalPos, int detectorID,
                                double lambdaBefore, double lambdaAfter) {
  initialise(rng, finalPos, detectorID);
  const auto scatterBounds = m_scatterVol.getBoundingBox();
  double factor(0.0);
  for (size_t i = 0; i < m_nevents; ++i) {
    size_t attempts(0);
    do {
      const auto neutron = m_beamProfile.generatePoint(rng, scatterBounds);

      Geometry::Track beforeScatter;
      Geometry::Track afterScatter;
      const bool success = m_scatterVol.calculateBeforeAfterTrack(
          rng, neutron.startPos, finalPos, beforeScatter, afterScatter,
          m_useCaching, detectorID);
      if (!success) {
        ++attempts;
      } else {
        const double wgt = m_scatterVol.calculateAbsorption(
            beforeScatter, afterScatter, lambdaBefore, lambdaAfter);
        factor += wgt;
        break;
      }
      if (attempts == m_maxScatterAttempts) {
        throw std::runtime_error("Unable to generate valid track through "
                                 "sample interaction volume after " +
                                 std::to_string(m_maxScatterAttempts) +
                                 " attempts. Try increasing the maximum "
                                 "threshold or if this does not help then "
                                 "please check the defined shape.");
      }
    } while (true);
  }
  using std::make_tuple;
  return make_tuple(factor / static_cast<double>(m_nevents), m_error);
}

void MCAbsorptionStrategy::calculateAllLambdas(
    Kernel::PseudoRandomNumberGenerator &rng, const Kernel::V3D &finalPos,
    int detectorID, DeltaEMode::Type EMode,
    Mantid::HistogramData::Points lambdas, const int lambdaStepSize,
    double lambdaFixed, Mantid::HistogramData::HistogramY &attenuationFactors,
    std::string& debugString) {

  initialise(rng, finalPos, detectorID);
  const auto scatterBounds = m_scatterVol.getBoundingBox();
  const auto nbins = static_cast<int>(lambdas.size());

  for (size_t i = 0; i < m_nevents; ++i) {
    size_t attempts(0);
    do {
      const auto neutron = m_beamProfile.generatePoint(rng, scatterBounds);

      Geometry::Track beforeScatter;
      Geometry::Track afterScatter;
      const bool success = m_scatterVol.calculateBeforeAfterTrack(
          rng, neutron.startPos, finalPos, beforeScatter, afterScatter,
          m_useCaching, detectorID);
      if (!success) {
        ++attempts;
      } else {
        m_attemptsCounts[attempts]++;
        for (int j = 0; j < nbins; j += lambdaStepSize) {
          const double lambdaStep = lambdas[j];
          double lambdaIn(lambdaStep), lambdaOut(lambdaStep);
          if (EMode == DeltaEMode::Direct) {
            lambdaIn = lambdaFixed;
          } else if (EMode == DeltaEMode::Indirect) {
            lambdaOut = lambdaFixed;
          } else {
            // elastic case already initialized
          }
          const double wgt = m_scatterVol.calculateAbsorption(
              beforeScatter, afterScatter, lambdaIn, lambdaOut);
          attenuationFactors[j] += wgt;

          // Ensure we have the last point for the interpolation
          if (lambdaStepSize > 1 && j + lambdaStepSize >= nbins &&
              j + 1 != nbins) {
            j = nbins - lambdaStepSize - 1;
          }
        }

        break;
      }
      if (attempts == m_maxScatterAttempts) {
        throw std::runtime_error("Unable to generate valid track through "
                                 "sample interaction volume after " +
                                 std::to_string(m_maxScatterAttempts) +
                                 " attempts. Try increasing the maximum "
                                 "threshold or if this does not help then "
                                 "please check the defined shape.");
      }
    } while (true);
  }

  attenuationFactors = attenuationFactors / static_cast<double>(m_nevents);

  debugString += m_scatterVol.printScatterPointCounts();
}

} // namespace Algorithms
} // namespace Mantid
