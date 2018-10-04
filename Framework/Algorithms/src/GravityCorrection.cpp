#include "MantidAlgorithms/GravityCorrection.h"

#include "MantidAPI/AlgorithmHistory.h"
#include "MantidAPI/HistogramValidator.h"
#include "MantidAPI/InstrumentValidator.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidAPI/Progress.h"
#include "MantidAPI/SpectrumInfo.h"
#include "MantidAPI/WorkspaceHistory.h"
#include "MantidAPI/WorkspaceProperty.h"
#include "MantidAPI/WorkspaceUnitValidator.h"
#include "MantidDataObjects/Workspace2D.h"
#include "MantidDataObjects/WorkspaceCreation.h"
#include "MantidGeometry/IComponent.h"
#include "MantidGeometry/Instrument.h"
#include "MantidGeometry/Instrument/ComponentInfo.h"
#include "MantidGeometry/Instrument/DetectorInfo.h"
#include "MantidHistogramData/BinEdges.h"
#include "MantidHistogramData/Histogram.h"
#include "MantidHistogramData/HistogramX.h"
#include "MantidIndexing/IndexInfo.h"
#include "MantidKernel/BoundedValidator.h"
#include "MantidKernel/CompositeValidator.h"
#include "MantidKernel/IPropertyManager.h"
#include "MantidKernel/PhysicalConstants.h"
#include "MantidKernel/Property.h"
#include "MantidKernel/Quat.h"
#include "MantidKernel/make_unique.h"

#include <cmath>
#include <iterator>
#include <memory>
#include <utility>

using Mantid::API::AlgorithmHistories;
using Mantid::API::AlgorithmHistory_const_sptr;
using Mantid::API::HistogramValidator;
using Mantid::API::InstrumentValidator;
using Mantid::API::MatrixWorkspace;
using Mantid::API::MatrixWorkspace_sptr;
using Mantid::API::Progress;
using Mantid::API::SpectrumInfo;
using Mantid::API::WorkspaceHistory;
using Mantid::API::WorkspaceProperty;
using Mantid::API::WorkspaceUnitValidator;
using Mantid::DataObjects::Workspace2D;
using Mantid::DataObjects::create;
using Mantid::Geometry::ComponentInfo;
using Mantid::Geometry::DetectorInfo;
using Mantid::Geometry::IComponent_const_sptr;
using Mantid::Geometry::PointingAlong;
using Mantid::HistogramData::BinEdges;
using Mantid::HistogramData::HistogramX;
using Mantid::Indexing::IndexInfo;
using Mantid::Kernel::BoundedValidator;
using Mantid::Kernel::CompositeValidator;
using Mantid::Kernel::Direction;
using Mantid::Kernel::Property;
using Mantid::Kernel::Quat;
using Mantid::Kernel::V3D;
using Mantid::Kernel::make_unique;
using Mantid::PhysicalConstants::g;

using boost::const_pointer_cast;
using boost::make_shared;
using std::abs;
using std::find_if;
using std::map;
using std::pair;
using std::pow;
using std::runtime_error;
using std::string;
using std::vector;

namespace Mantid {
namespace Algorithms {

// Register the algorithm into the AlgorithmFactory
DECLARE_ALGORITHM(GravityCorrection)

void GravityCorrection::init() {
  auto wsValidator = make_shared<CompositeValidator>();
  wsValidator->add<WorkspaceUnitValidator>("TOF");
  wsValidator->add<HistogramValidator>();
  wsValidator->add<InstrumentValidator>();
  this->declareProperty(
      make_unique<WorkspaceProperty<MatrixWorkspace>>(
          "InputWorkspace", "", Direction::Input, wsValidator),
      "The name of the input workspace. Values of X and Y must be "
      "TOF and counts, respectively.");
  this->declareProperty(make_unique<WorkspaceProperty<MatrixWorkspace>>(
                            "OutputWorkspace", "", Direction::Output),
                        "The name of the output workspace.");
  this->declareProperty("FirstSlitName", "slit1",
                        "Component name of the first slit; Workflow.slit1 "
                        "value in parameter or instrument definition file may "
                        "have priority.");
  this->declareProperty("SecondSlitName", "slit2",
                        "Component name of the second slit; Workflow.slit2 "
                        "value in parameter or instrument definition file may "
                        "have priority.");
}

/**
 * @brief GravityCorrection::componentName checks whether the component name is
 * given in the parameter file
 * @param inputName :: name of the algorithm property
 * @param instr :: from which instrument to read parameter value
 * @return the name of the component
 */
string GravityCorrection::componentName(
    const string &inputName, Mantid::Geometry::Instrument_const_sptr &instr) {
  const string propName = this->getPropertyValue(inputName);
  const string compName = instr->getParameterAsString("Workflow." + propName);
  if (!compName.empty()) {
    g_log.information("Found " + propName + " in Parameter File. " + inputName +
                      ": " + compName);
    return compName;
  }
  return propName;
}

/**
 * @brief GravityCorrection::validateInputs of InputWorkspace, FirstSlitName,
 * SecondSlitName
 * @return a string map containing the error messages
 */
map<string, string> GravityCorrection::validateInputs() {
  map<string, string> result;
  // InputWorkspace
  this->m_ws = this->getProperty("InputWorkspace");
  if (this->m_ws == nullptr)
    result["InputWorkspace"] = "InputWorkspace not defined.";
  const WorkspaceHistory history = this->m_ws->getHistory();
  const AlgorithmHistories &histories = history.getAlgorithmHistories();
  // iterator to first occurence of the algorithm name
  const auto it = find_if(
      histories.cbegin(), histories.cend(),
      [this](const boost::shared_ptr<Mantid::API::AlgorithmHistory> &i) {
        return i->name() == this->name();
      });
  if (it != histories.end())
    result["InputWorkspace"] = "GravityCorrection did already execute "
                               "(check workspace history).";
  // Slits (name empty? component exists? positions differ?)
  // Get a pointer to the instrument of the input workspace
  Mantid::Geometry::Instrument_const_sptr instrument =
      this->m_ws->getInstrument();
  const auto refFrame = instrument->getReferenceFrame();
  m_beamDirection = refFrame->pointingAlongBeam();
  m_upDirection = refFrame->pointingUp();
  m_horizontalDirection = refFrame->pointingHorizontal();
  map<string, string> slits;
  slits["FirstSlitName"] = this->componentName("FirstSlitName", instrument);
  slits["SecondSlitName"] = this->componentName("SecondSlitName", instrument);
  double iposition{0.}, last{0.};
  for (auto mapit = slits.begin(); mapit != slits.end(); ++mapit) {
    if (mapit->second.empty())
      result[mapit->first] = "Provide a name.";
    else {
      IComponent_const_sptr slit =
          instrument->getComponentByName(mapit->second);
      if (slit == nullptr)
        result[mapit->first] = "Instrument component with name " +
                               (mapit->second) + " does not exist. ";
      else {
        last = this->coordinate(mapit->second, m_beamDirection, instrument);
        iposition += last;
        if ((mapit->first == "SecondSlitName") &&
            (iposition / 2 == last && last > 0.))
          result["SecondSlitName"] = "Position of slits must differ.";
      }
    }
  }
  return result;
}

/**
 * @brief GravityCorrection::coordinate
 * @param componentName :: name of the instrument component
 * @param direction :: direction of the coordinate (i.e. PointingAlong::X,
 * PointingAlong::Y, PointingAlong::Z)
 * @param instrument :: instrument containing the component
 * @return the coordinate of the instrument component (unit metres)
 */
double GravityCorrection::coordinate(
    const string &componentName, PointingAlong direction,
    Mantid::Geometry::Instrument_const_sptr instrument) const {
  double position{0.};
  IComponent_const_sptr component;
  if (instrument == nullptr)
    component = this->m_virtualInstrument->getComponentByName(componentName);
  else
    component = instrument->getComponentByName(componentName);
  if (component == nullptr)
    this->g_log.error("Cannot get instrument component with name " +
                      componentName);
  else {
    V3D pos = component->getPos();
    position = this->coordinate(pos, direction);
  }
  return position;
}

/**
 * @brief GravityCorrection::coordinate
 * @param detectorInfo :: reference detectorInfo
 * @param i :: detector index
 * @param direction :: direction of the coordinate (i.e. PointingAlong::X,
 * PointingAlong::Y, PointingAlong::Z)
 * @return the coordinate of a detector at index i (unit metres)
 */
double GravityCorrection::coordinate(const DetectorInfo &detectorInfo, size_t i,
                                     PointingAlong direction) const {
  V3D position{detectorInfo.position(i)};
  return this->coordinate(position, direction);
}

/**
 * @brief GravityCorrection::coordinate
 * @param spectrumInfo :: reference spectrumInfo
 * @param i :: spectrum index
 * @param direction :: direction of the coordinate (i.e. PointingAlong::X,
 * PointingAlong::Y, PointingAlong::Z)
 * @return the coordinate of a detector or group of detectors of a spectrum at
 * index 1 (unit metres)
 */
double GravityCorrection::coordinate(const SpectrumInfo &spectrumInfo, size_t i,
                                     PointingAlong direction) const {
  V3D position{spectrumInfo.position(i)};
  return this->coordinate(position, direction);
}

/**
 * @brief GravityCorrection::coordinate
 * @param pos :: a V3D holding a position in Cartesian coordinates in metres
 * @param direction :: direction of the coordinate (i.e. PointingAlong::X,
 * PointingAlong::Y, PointingAlong::Z)
 * @return the coordinate of the position in direction in metres
 */
double GravityCorrection::coordinate(const V3D &pos,
                                     PointingAlong direction) const {
  double position{0.};
  switch (direction) {
  case Mantid::Geometry::X:
    position = pos.X();
    break;
  case Mantid::Geometry::Y:
    position = pos.Y();
    break;
  case Mantid::Geometry::Z:
    position = pos.Z();
    break;
  default:
    g_log.error("Axis is not X/Y/Z");
    throw runtime_error("Axis is not X/Y/Z");
    break;
  }
  return position;
}

/**
 * @brief GravityCorrection::setCoordinate
 * @param pos :: holds the position vector to be updated in metres
 * @param direction :: direction of the coordinate of the position to be updated
 * (i.e. PointingAlong::X, PointingAlong::Y, PointingAlong::Z)
 * @param coor :: coordinate to be added to the position vector in metres
 */
void GravityCorrection::setCoordinate(V3D &pos, PointingAlong direction,
                                      double coor) {
  switch (direction) {
  case Mantid::Geometry::X:
    pos.setX(pos.X() + coor);
    break;
  case Mantid::Geometry::Y:
    pos.setY(pos.Y() + coor);
    break;
  case Mantid::Geometry::Z:
    pos.setZ(pos.Z() + coor);
    break;
  default:
    g_log.error("Axis is not X/Y/Z");
    throw runtime_error("Axis is not X/Y/Z");
    break;
  }
}

/**
 * @brief GravityCorrection::slitCheck attempts to set the class member
 * variables m_slit1Name and m_slit2Name according to their position.
 * Errors get logged in case of insensible slit positions.
 */
void GravityCorrection::slitCheck() {
  const string sourceName = this->m_virtualInstrument->getSource()->getName();
  const string sampleName = this->m_virtualInstrument->getSample()->getName();
  const string slit1{
      this->componentName("FirstSlitName", this->m_virtualInstrument)};
  const string slit2{
      this->componentName("SecondSlitName", this->m_virtualInstrument)};
  // initial guess
  this->m_slit1Name = slit1;
  this->m_slit2Name = slit2;
  // in beam directions
  const double sourceD = this->coordinate(sourceName, this->m_beamDirection);
  const double sampleD = this->coordinate(sampleName, this->m_beamDirection);
  const double slit1D = this->coordinate(slit1, this->m_beamDirection);
  const double slit2D = this->coordinate(slit2, this->m_beamDirection);
  // slits must be located between source and sample
  // source -> slit1 -> slit2 -> sample
  if (abs(slit2D) < abs(slit1D)) {
    this->m_slit1Name = slit2;
    this->m_slit2Name = slit1;
  }
  if (abs(slit1D) > abs(sourceD))
    this->g_log.error(this->m_slit1Name + " position must be after source.");
  if (abs(slit2D) < abs(sampleD))
    this->g_log.error(this->m_slit2Name + " position must be before sample.");
}

/**
 * @brief GravityCorrection::finalAngle calculates the sample hit position
 * @param k :: characteristic length of the parabola in 1/m
 * @param i :: spectrum number
 * @return final angle in radians
 */
double GravityCorrection::finalAngle(const double k, size_t i,
                                     const double tanAngle,
                                     const double beamDiff) {
  // calculate parabola
  // calculate slit pointing up coordinate
  // potential divide by zero avoided by input validation m_beam1 != m_beam2
  double beamShift =
      (k * (pow(m_beam1, 2.) - pow(m_beam2, 2.)) + (beamDiff * tanAngle)) /
      (2 * k * beamDiff);
  const double up2 = -this->sgn(this->m_ws->spectrumInfo().signedTwoTheta(i)) *
                     m_beam2 * tanAngle;
  double upShift = up2 + k * pow(m_beam2 - beamShift, 2.);
  // calculate final angle
  return this->sgn(this->m_ws->spectrumInfo().signedTwoTheta(i)) *
         atan(2. * k * sqrt(abs(upShift / k)));
}

/**
 * @brief GravityCorrection::virtualInstrument defines a virtual instrument
 * with the sample at its origin x = y = z = 0 m. The original instrument and
 * its parameter map will be copied.
 */
void GravityCorrection::virtualInstrument() {

  const auto instrument = this->m_ws->getInstrument();
  MatrixWorkspace_sptr ws = create<Workspace2D>(
      instrument, this->m_ws->indexInfo().globalSize(), BinEdges(2));

  if (instrument->isParametrized()) {
    const V3D &samplePos = instrument->getSample()->getPos();
    const string sourceName{instrument->getSource()->getName()};
    const string sampleName{instrument->getSample()->getName()};

    vector<IComponent_const_sptr> comps = {
        instrument->getComponentByName(sourceName),
        instrument->getComponentByName(sampleName),
        instrument->getComponentByName(this->getProperty("FirstSlitName")),
        instrument->getComponentByName(this->getProperty("SecondSlitName"))};

    // translate instrument
    if (samplePos.distance(V3D(0., 0., 0.)) > 1e-10) {
      // move instrument to ensure sample at position x = y = z = 0 m,
      auto &componentInfo = ws->mutableComponentInfo();
      for (vector<IComponent_const_sptr>::iterator compit = comps.begin();
           compit != comps.end(); ++compit) {
        const auto compID1 = (*compit)->getComponentID();
        componentInfo.setPosition(componentInfo.indexOf(compID1),
                                  (*compit)->getPos() - samplePos);
      }
      auto &detectorInfo = ws->mutableDetectorInfo();
      for (size_t i = 0; i < detectorInfo.size(); ++i)
        detectorInfo.setPosition(i, detectorInfo.position(i) - samplePos);
    }

    const double sampleY =
        this->coordinate(sampleName, this->m_upDirection, ws->getInstrument());
    // check if the instrument is rotated: then the up direction and horizontal
    // directions are not zero: -> rotate instrument (update positions)
    double tanAngle{0.}; // will hold tan(rotation angle)
    if (sampleY != 0.) {
      // calculate vertical rotation angle:
      /*         ^ y
       *         |   /|
       *         |  / |
       *         | /a |
       *         |/___|____> z
       */
      tanAngle = sampleY / this->coordinate(sourceName, m_beamDirection,
                                            ws->getInstrument());
      auto &componentInfo = ws->mutableComponentInfo();
      for (auto compit = comps.begin(); compit != comps.end(); ++compit) {
        const auto compID2 = (*compit)->getComponentID();
        V3D position = (*compit)->getPos();
        double coordUp = this->coordinate(
            (*compit)->getName(), this->m_upDirection, ws->getInstrument());
        this->setCoordinate(position, this->m_beamDirection,
                            coordUp / tanAngle);
        componentInfo.setPosition(componentInfo.indexOf(compID2), position);
      }
      auto &detectorInfo = ws->mutableDetectorInfo();
      for (size_t i = 0; i < detectorInfo.size(); ++i) {
        V3D position = detectorInfo.position(i);
        this->setCoordinate(
            position, this->m_beamDirection,
            this->coordinate(detectorInfo, i, this->m_upDirection) / tanAngle);
        detectorInfo.setPosition(i, position);
        // rotate detectors
        V3D vvector{0., 0., 0.};
        double vangle = atan(tanAngle);
        this->setCoordinate(vvector, this->m_upDirection, sin(vangle));
        this->setCoordinate(vvector, this->m_beamDirection, cos(vangle));
        const Quat &rot = Quat(vangle, vvector);
        detectorInfo.setRotation(i, detectorInfo.rotation(i) * rot);
      }
    }
    const double sampleX = this->coordinate(
        sampleName, this->m_horizontalDirection, ws->getInstrument());
    if (sampleX != 0.) {
      // calculate horizontal rotation angle
      /*         ^ z
       *         |___
       *         |  /
       *         |a/
       *         |/_______> x
       */
      tanAngle = sampleX / this->coordinate(sampleName, this->m_beamDirection,
                                            ws->getInstrument());
      auto &componentInfo = ws->mutableComponentInfo();
      for (auto compit = comps.begin(); compit != comps.end(); ++compit) {
        const auto compID2 = (*compit)->getComponentID();
        V3D position = (*compit)->getPos();
        double coordHori =
            this->coordinate((*compit)->getName(), this->m_horizontalDirection,
                             ws->getInstrument());
        this->setCoordinate(position, this->m_beamDirection,
                            coordHori / tanAngle);
        componentInfo.setPosition(componentInfo.indexOf(compID2), position);
      }
      auto &detectorInfo = ws->mutableDetectorInfo();
      for (size_t i = 0; i < detectorInfo.size(); ++i) {
        V3D position = detectorInfo.position(i);
        this->setCoordinate(
            position, this->m_beamDirection,
            this->coordinate(detectorInfo, i, this->m_horizontalDirection) /
                tanAngle);
        detectorInfo.setPosition(i, position);
        // rotate detectors
        V3D hvector{0., 0., 0.};
        double hangle = atan(tanAngle);
        this->setCoordinate(hvector, this->m_horizontalDirection, sin(hangle));
        this->setCoordinate(hvector, this->m_beamDirection, cos(hangle));
        const Quat &rot = Quat(hangle, hvector);
        detectorInfo.setRotation(i, detectorInfo.rotation(i) * rot);
      }
    }
    this->m_virtualInstrument = ws->getInstrument();
    if (this->m_virtualInstrument->isEmptyInstrument())
      this->g_log.error("Cannot create a virtual instrument.");
  } else
    this->g_log.error("Instrument of the InputWorkspace is not parametrised.");

  if (!(m_virtualInstrument->isParametrized()))
    this->g_log.error("Cannot copy parameter map correctly from original "
                      "instrument. Virtual instrument is not parametrised.");
}

/**
 * @brief GravityCorrection::spectrumNumber
 * @param angle :: a final angle for a specific detector in radians
 * @param spectrumInfo :: a reference to spectrum information
 * @param i :: spectrum number
 * @return spectrum number closest to the given final angle
 */
size_t GravityCorrection::spectrumNumber(const double angle,
                                         const SpectrumInfo &spectrumInfo,
                                         size_t i) {
  // counts are dropping down -> only move counts up
  size_t n = i;
  const double signedCurrentAngle = spectrumInfo.signedTwoTheta(i) / 2.;
  // a starting index for an effective search that exists
  auto it = this->m_finalAngles.find(signedCurrentAngle);
  // the upper bound to be added to the updated final angle:
  /*                           |
   *                          _|_
   *                           |
   *                 angle---->|
   *                           |<---
   *                           |
   *                          _|_
   *                           |
   *                           |
   *    signedCurrentAngle---->|<---it->second
   * (lowest possible angle)   |
   *                          _|_
   *                           |
   */
  ++it;
  double tol = .5 * ((++it)->first - it->first);
  --it;
  // determine new final angle
  while (this->m_smallerThan((*(++it)).first + tol, angle) &&
         (it != this->m_finalAngles.end()))
    n = it->second;
  return n;
}

/**
 * @brief GravityCorrection::parabolaArcLength: integration range is 0 (sample
 * position) to detector position in beam direction (expressed in arg).
 * Solution of the integral (constant + f'(x)^2) dx for
 * @param arg :: gradient at end position of the integral
 * @param constant :: constant
 * @return length of the parabola arc
 */
double GravityCorrection::parabolaArcLength(const double arg,
                                            double constant) const {
  constant = abs(constant);
  double a = pow(arg, 2.);
  double b = log((arg / sqrt(constant)) + sqrt(1 + a / constant));
  return 0.5 * (arg * sqrt(constant + a) + constant * b);
}

void GravityCorrection::exec() {

  this->m_progress =
      make_unique<Progress>(this, 0.0, 1.0, 10 + this->m_ws->size());

  this->m_progress->report("Create virtual instrument ...");
  this->virtualInstrument();
  this->m_progress->report("Checking slits ...");
  this->slitCheck();
  m_beam1 = this->coordinate(this->m_slit1Name, this->m_beamDirection);
  m_beam2 = this->coordinate(this->m_slit2Name, this->m_beamDirection);
  const auto &spectrumInfo = this->m_ws->spectrumInfo();
  this->m_progress->report("Setup OutputWorkspace ...");
  MatrixWorkspace_sptr outWS = this->getProperty("OutputWorkspace");
  outWS = this->m_ws->clone();
  outWS->setTitle(this->m_ws->getTitle() + " cancelled gravitation ");
  for (size_t i = 0; i < spectrumInfo.size(); ++i) {
    if (spectrumInfo.isMonitor(i)) {
      // copy monitor data into output workspace
      outWS->mutableX(i) = this->m_ws->x(i);
      outWS->mutableY(i) = this->m_ws->y(i);
      outWS->mutableE(i) = this->m_ws->e(i);
      continue;
    }
    if (!spectrumInfo.hasDetectors(i))
      continue;

    // setup map of initial final angles (y axis, spectra)
    // this map is sorted internally by its finalAngleValue's
    double finalAngleValue = spectrumInfo.signedTwoTheta(i) / 2.;
    this->m_finalAngles.emplace_hint(this->m_finalAngles.end(),
                                     std::make_pair(finalAngleValue, i));

    // unmask bins of OutputWorkspace
    if (outWS->hasMaskedBins(i)) {
      this->g_log.debug("Unmask bins.");
      const MatrixWorkspace::MaskList &maskOut = outWS->maskedBins(i);
      MatrixWorkspace::MaskList::const_iterator maskit;
      for (maskit = maskOut.begin(); maskit != maskOut.end(); ++maskit)
        outWS->flagMasked(i, maskit->first, 0.0);
    }
    this->m_progress->report();
  }
  if (this->m_finalAngles.empty())
    this->g_log.error(
        "Map of initial final angles and its corresponding spectrum "
        "number does not exist.");
  this->m_progress->report("Perform gravity correction ...");
  const double beamDiff = m_beam1 - m_beam2;
  for (size_t i = 0; i < spectrumInfo.size(); ++i) {
    if (spectrumInfo.isMonitor(i)) {
      this->g_log.debug("Found monitor spectrum, will be ignored.");
      continue;
    }
    if (!spectrumInfo.hasDetectors(i)) {
      this->g_log.debug("No detector(s) found");
      continue;
    }

    // take neutrons that hit the detector of spectrum i
    // get a const reference to X values
    const HistogramX &tof = this->m_ws->x(i);
    const auto y = this->m_ws->y(i);
    const auto e = this->m_ws->e(i);
    // correct tof angles, velocity, characteristic length
    size_t i_tofit{0};
    const double tanAngle =
        tan(this->m_ws->spectrumInfo().signedTwoTheta(i) / 2.);
    for (HistogramX::const_iterator tofit = tof.cbegin(); tofit < tof.cend();
         ++tofit) {
      // this velocity should take the real flight path into account
      if (*tofit == 0.) {
        this->g_log.notice(
            "Zero tof detected. Cannot divide by it, skip this bin.");
        continue;
      }

      double v{((spectrumInfo.l1() + spectrumInfo.l2(i)) / *tofit) *
               1.e6};                         // unit is m/s
      double k = g * 800 / (2. * pow(v, 2.)); // unit is 1/m
      double angle =
          this->finalAngle(k, i, tanAngle, beamDiff); // unit is radians
      if (cos(angle) == 0.) {
        this->g_log.error("Cannot divide by zero for calculating new tof "
                          "values. Skip this bin.");
        continue;
      }

      // get new spectrum number for new final angle
      auto j = this->spectrumNumber(angle, spectrumInfo, i);

      // straight path from virtual sample (0, 0, 0) to updated detector
      // position:
      const std::set<detid_t> dets =
          this->m_ws->getSpectrum(j).getDetectorIDs();

      V3D detPos;
      if (dets.size() == 1)
        detPos =
            this->m_virtualInstrument->getDetector(*dets.cbegin())->getPos();
      else if (dets.size() > 1) {
        g_log.information("Found grouped detectors ... skip for the moment");
      } else
        continue;

      double detZ = this->coordinate(detPos, m_beamDirection);
      // possible trajectory from sample to detector, almost equals detZ
      double s2 = this->parabolaArcLength(2 * k * detZ) / (2 * k);

      // this value should be close to one
      // const double cOne = (detZ - offset) / (s * cos(angle));
      const double cOne = detZ / (s2 * cos(angle));
      outWS->mutableX(j)[i_tofit] = cOne * *tofit; // mu sec

      // need to set the counts to spectrum according to finalAngle & *tofit
      outWS->mutableY(j)[i_tofit] += y[i_tofit];
      outWS->mutableE(j)[i_tofit] += e[i_tofit];
      // must be always positiv
      if (this->m_ws->hasDx(i))
        outWS->mutableDx(j)[i_tofit] += this->m_ws->dx(i)[i_tofit];
      ++i_tofit;
      this->m_progress->report();
    }

    this->m_progress->report("Mask bins consideration ...");
    if (this->m_ws->hasMaskedBins(i)) {
      // store mask of InputWorkspace
      HistogramX &tof2 = outWS->mutableX(i);
      const MatrixWorkspace::MaskList &maskIn = this->m_ws->maskedBins(i);
      MatrixWorkspace::MaskList::const_iterator maskit;
      for (maskit = maskIn.begin(); maskit != maskIn.end(); ++maskit) {
        // determine offset for new bin index
        HistogramX::iterator t =
            find_if(tof2.begin(), tof2.end(), [&](const double &ii) {
              return ii > this->m_ws->x(i)[maskit->first];
            });
        if (t != tof2.end()) {
          // get left bin boundary (index) of t
          pair<size_t, double> ti = outWS->getXIndex(i, *t, true, 0);
          outWS->flagMasked(i, ti.first, maskit->second);
        }
        this->m_progress->report();
      }
    }
  }
  this->setProperty("OutputWorkspace", outWS);
}

} // namespace Algorithms
} // namespace Mantid
