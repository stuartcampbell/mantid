// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2016 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#ifndef MANTID_ALGORITHMS_MCINTERACTIONVOLUME_H_
#define MANTID_ALGORITHMS_MCINTERACTIONVOLUME_H_

#include "MantidAlgorithms/DllConfig.h"
#include "MantidGeometry/Objects/BoundingBox.h"
#include "MantidGeometry/Objects/Track.h"

namespace Mantid {
namespace API {
class Sample;
}
namespace Geometry {
class IObject;
class SampleEnvironment;
enum class scatterBeforeAfter;
} // namespace Geometry

namespace Kernel {
class PseudoRandomNumberGenerator;
class V3D;
} // namespace Kernel
namespace Algorithms {
class IBeamProfile;

/**
  Defines a volume where interactions of Tracks and Objects can take place.
  Given an initial Track, end point & wavelengths it calculates the absorption
  correction factor.
*/
class MANTID_ALGORITHMS_DLL MCInteractionVolume {
public:
  MCInteractionVolume(const API::Sample &sample,
                      const Geometry::BoundingBox &activeRegion,
                      const size_t maxScatterAttempts = 5000);
  // No creation from temporaries as we store a reference to the object in
  // the sample
  MCInteractionVolume(const API::Sample &&sample,
                      const Geometry::BoundingBox &&activeRegion) = delete;

  const Geometry::BoundingBox &getBoundingBox() const;
  bool calculateBeforeAfterTrack(Kernel::PseudoRandomNumberGenerator &rng,
                                 const Kernel::V3D &startPos,
                                 const Kernel::V3D &endPos,
                                 Geometry::Track &beforeScatter,
                                 Geometry::Track &afterScatter, bool useCaching,
                                 int detectorID) const;
  double calculateAbsorption(const Geometry::Track &beforeScatter,
                             const Geometry::Track &afterScatter,
                             double lambdaBefore, double lambdaAfter) const;
  void resetActiveElements(Geometry::scatterBeforeAfter stage, int dectectorID,
                           bool active);
  void addActiveElements(Kernel::PseudoRandomNumberGenerator &rng,
                         const Kernel::V3D &startPos, const Kernel::V3D &endPos,
                         int detectorID);
  void
  addActiveElementsForScatterPoint(Kernel::PseudoRandomNumberGenerator &rng);

private:
  const boost::shared_ptr<Geometry::IObject> m_sample;
  const Geometry::SampleEnvironment *m_env;
  const Geometry::BoundingBox m_activeRegion;
  const size_t m_maxScatterAttempts;
  Kernel::V3D generatePoint(Kernel::PseudoRandomNumberGenerator &rng,
                            const Geometry::BoundingBox &activeRegion,
                            const size_t maxAttempts, bool buildCache,
                            Geometry::scatterBeforeAfter stage) const;
};

} // namespace Algorithms
} // namespace Mantid

#endif /* MANTID_ALGORITHMS_MCINTERACTIONVOLUME_H_ */
