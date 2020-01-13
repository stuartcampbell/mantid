// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#include "MantidAlgorithms/SampleCorrections/MCInteractionVolume.h"
#include "MantidAPI/Sample.h"
#include "MantidGeometry/Instrument/SampleEnvironment.h"
#include "MantidGeometry/Objects/CSGObject.h"
#include "MantidKernel/Material.h"
#include "MantidKernel/PseudoRandomNumberGenerator.h"
#include <iomanip>

namespace Mantid {
using Geometry::Track;
using Kernel::V3D;

namespace Algorithms {

/**
 * Construct the volume encompassing the sample + any environment kit. The
 * beam profile defines a bounding region for the sampling of the scattering
 * position.
 * @param sample A reference to a sample object that defines a valid shape
 * & material
 * @param activeRegion Restrict scattering point sampling to this region
 * @param maxScatterAttempts The maximum number of tries to generate a random
 * point within the object. [Default=5000]
 */
MCInteractionVolume::MCInteractionVolume(
    const API::Sample &sample, const Geometry::BoundingBox &activeRegion,
    const size_t maxScatterAttempts)
    : m_sample(sample.getShape().clone()), m_env(nullptr),
      m_activeRegion(activeRegion), m_maxScatterAttempts(maxScatterAttempts) {
  if (!m_sample->hasValidShape()) {
    throw std::invalid_argument(
        "MCInteractionVolume() - Sample shape does not have a valid shape.");
  }
  try {
    m_env = &sample.getEnvironment();
    if (m_env->nelements() == 0) {
      throw std::invalid_argument(
          "MCInteractionVolume() - Sample enviroment has zero components.");
    }
  } catch (std::runtime_error &) {
    // swallow this as no defined environment from getEnvironment
  }

  m_envScatterPoints.resize(m_env->nelements());
}

/**
 * Returns the axis-aligned bounding box for the volume
 * @return A reference to the bounding box
 */
const Geometry::BoundingBox &MCInteractionVolume::getBoundingBox() const {
  return m_sample->getBoundingBox();
}

/**
 * Generate point randomly across one of the components of the environment
 * including the sample itself in the selection
 */
Kernel::V3D
MCInteractionVolume::generatePoint(Kernel::PseudoRandomNumberGenerator &rng,
                                   const Geometry::BoundingBox &activeRegion,
                                   const size_t maxAttempts, bool buildCache,
                                   Geometry::scatterBeforeAfter stage) {
  for (int i = 0; i < maxAttempts; i++) {
    Kernel::V3D point;
    int componentIndex;
    if (m_env) {
      componentIndex = rng.nextInt(0, static_cast<int>(m_env->nelements())) - 1;
    } else {
      componentIndex = -1;
    }

    bool pointGenerated;
    if (componentIndex == -1) {
      pointGenerated = m_sample->generatePointInObject(
          rng, activeRegion, 1, point, buildCache, stage);
    } else {
      pointGenerated = m_env->getComponent(componentIndex)
                           .generatePointInObject(rng, activeRegion, 1, point,
                                                  buildCache, stage);
    }
    if (pointGenerated) {
      if (componentIndex == -1) {
        m_sampleScatterPoints++;
      } else {
        m_envScatterPoints[componentIndex]++;
      }
      return point;
    }
  }
  throw std::runtime_error("MCInteractionVolume::generatePoint() - Unable to "
                           "generate point in object after " +
                           std::to_string(maxAttempts) + " attempts");
}

/**
 * Calculate the attenuation correction factor the volume given a start and
 * end point.
 * @param rng A reference to a PseudoRandomNumberGenerator producing
 * random number between [0,1]
 * @param startPos Origin of the initial track
 * @param endPos Final position of neutron after scattering (assumed to be
 * outside of the "volume")
 * @param lambdaBefore Wavelength, in \f$\\A^-1\f$, before scattering
 * @param lambdaAfter Wavelength, in \f$\\A^-1\f$, after scattering
 * @return The fraction of the beam that has been attenuated. A negative number
 * indicates the track was not valid.
 */
bool MCInteractionVolume::calculateBeforeAfterTrack(
    Kernel::PseudoRandomNumberGenerator &rng, const Kernel::V3D &startPos,
    const Kernel::V3D &endPos, Track &beforeScatter, Track &afterScatter,
    bool useCaching, int detectorID) {
  // Generate scatter point. If there is an environment present then
  // first select whether the scattering occurs on the sample or the
  // environment. The attenuation for the path leading to the scatter point
  // is calculated in reverse, i.e. defining the track from the scatter pt
  // backwards for simplicity with how the Track object works. This avoids
  // having to understand exactly which object the scattering occurred in.
  V3D scatterPos;

  scatterPos =
      generatePoint(rng, m_activeRegion, m_maxScatterAttempts, false,
                    useCaching ? Geometry::scatterBeforeAfter::scScatter
                               : Geometry::scatterBeforeAfter::scNone);

  const auto toStart = normalize(startPos - scatterPos);
  beforeScatter = Track(scatterPos, toStart);
  int nlinks = m_sample->interceptSurface(
      beforeScatter, false,
      useCaching ? Geometry::scatterBeforeAfter::scBefore
                 : Geometry::scatterBeforeAfter::scNone,
      detectorID);
  if (m_env) {
    nlinks += m_env->interceptSurfaces(
        beforeScatter, false,
        useCaching ? Geometry::scatterBeforeAfter::scBefore
                   : Geometry::scatterBeforeAfter::scNone,
        detectorID);
  }
  // This should not happen but numerical precision means that it can
  // occasionally occur with tracks that are very close to the surface
  if (nlinks == 0) {
    return false;
  }

  // Now track to final destination
  const V3D scatteredDirec = normalize(endPos - scatterPos);
  afterScatter = Track(scatterPos, scatteredDirec);
  m_sample->interceptSurface(afterScatter, false,
                             useCaching ? Geometry::scatterBeforeAfter::scAfter
                                        : Geometry::scatterBeforeAfter::scNone,
                             detectorID);
  if (m_env) {
    m_env->interceptSurfaces(afterScatter, false,
                             useCaching ? Geometry::scatterBeforeAfter::scAfter
                                        : Geometry::scatterBeforeAfter::scNone,
                             detectorID);
  }
  return true;
}

double MCInteractionVolume::calculateAbsorption(const Track &beforeScatter,
                                                const Track &afterScatter,
                                                double lambdaBefore,
                                                double lambdaAfter) const {

  // Function to calculate total attenuation for a track
  auto calculateAttenuation = [](const Track &path, double lambda) {
    double factor(1.0);
    for (const auto &segment : path) {
      const double length = segment.distInsideObject;
      const auto &segObj = *(segment.object);
      factor *= segObj.material().attenuation(length, lambda);
    }
    return factor;
  };

  return calculateAttenuation(beforeScatter, lambdaBefore) *
         calculateAttenuation(afterScatter, lambdaAfter);
}

void MCInteractionVolume::resetActiveElements(
    Geometry::scatterBeforeAfter stage, bool active, int nDetectors) {
  m_sample->resetActiveElements(stage, active, nDetectors);
  if (m_env) {
    m_env->resetActiveElements(stage, active, nDetectors);
  }
}

void MCInteractionVolume::addActiveElementsForScatterPoint(
    Kernel::PseudoRandomNumberGenerator &rng) {

  generatePoint(rng, m_activeRegion, m_maxScatterAttempts, true,
                Geometry::scatterBeforeAfter::scScatter);
}

void MCInteractionVolume::addActiveElementsForBeforeScatter(
    Kernel::PseudoRandomNumberGenerator &rng, const Kernel::V3D &startPos) {

  V3D scatterPos;

  // need to generate scatter point in order to generate the caches for the
  // before\after tracks
  scatterPos = generatePoint(rng, m_activeRegion, m_maxScatterAttempts, false,
                             Geometry::scatterBeforeAfter::scScatter);

  const auto toStart = normalize(startPos - scatterPos);
  Track beforeScatter(scatterPos, toStart);

  m_sample->interceptSurface(beforeScatter, true,
                             Geometry::scatterBeforeAfter::scBefore, -1);
  if (m_env) {
    m_env->interceptSurfaces(beforeScatter, true,
                             Geometry::scatterBeforeAfter::scBefore, -1);
  }
}

void MCInteractionVolume::addActiveElementsForAfterScatter(
    Kernel::PseudoRandomNumberGenerator &rng, const Kernel::V3D &endPos,
    int detectorID) {

  V3D scatterPos;

  // need to generate scatter point in order to generate the caches for the
  // before\after tracks
  scatterPos = generatePoint(rng, m_activeRegion, m_maxScatterAttempts, false,
                             Geometry::scatterBeforeAfter::scScatter);

  const V3D scatteredDirec = normalize(endPos - scatterPos);
  Track afterScatter(scatterPos, scatteredDirec);

  m_sample->interceptSurface(afterScatter, true,
                             Geometry::scatterBeforeAfter::scAfter, detectorID);
  if (m_env) {
    m_env->interceptSurfaces(afterScatter, true,
                             Geometry::scatterBeforeAfter::scAfter, detectorID);
  }
}

std::string MCInteractionVolume::printScatterPointCounts() {
  std::stringstream scatterPointSummary;
  scatterPointSummary << std::fixed;
  scatterPointSummary << std::setprecision(2);

  scatterPointSummary << "Scatter point counts:" << std::endl;

  int totalScatterPoints = m_sampleScatterPoints;
  for (int i = 0; i < m_envScatterPoints.size(); i++) {
    totalScatterPoints += m_envScatterPoints[i];
  }
  scatterPointSummary << "Total scatter points: " << totalScatterPoints
                      << std::endl;

  double percentage =
      static_cast<double>(m_sampleScatterPoints) / totalScatterPoints * 100;
  scatterPointSummary << "Sample: " << m_sampleScatterPoints << " ("
                      << percentage << "%)" << std::endl;

  for (int i = 0; i < m_envScatterPoints.size(); i++) {
    percentage =
        static_cast<double>(m_envScatterPoints[i]) / totalScatterPoints * 100;
    scatterPointSummary << "Environment part " << i << " ("
                        << m_env->getComponent(i).id()
                        << "): " << m_envScatterPoints[i] << " (" << percentage
                        << "%)" << std::endl;
  }
  return scatterPointSummary.str();
}

} // namespace Algorithms
} // namespace Mantid
