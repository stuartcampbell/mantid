// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#include "ALFView_presenter.h"
#include "ALFView_model.h"
#include "ALFView_view.h"

#include "MantidAPI/FileFinder.h"

#include <functional>

namespace {
const std::string instrument = "ALF";
}

namespace MantidQt {
namespace CustomInterfaces {

ALFView_presenter::ALFView_presenter(ALFView_view *view, ALFView_model *model)
    : m_view(view), m_model(model), m_currentRun(0),
      m_numberOfTubesInAverage(0), m_loadRunObserver(nullptr),
      m_browseObserver(nullptr), m_extractSingleTubeObserver(nullptr),
      m_averageTubeObserver(nullptr) {
  m_loadRunObserver = new voidObserver();
  m_browseObserver = new generalObserver();
  m_extractSingleTubeObserver = new voidObserver();
  m_averageTubeObserver = new voidObserver();
  m_model->loadEmptyInstrument();
}

void ALFView_presenter::initLayout() {
  // connect to new run
  m_view->observeLoadRun(m_loadRunObserver);
  std::function<void()> loadBinder =
      std::bind(&ALFView_presenter::loadRunNumber, this);
  m_loadRunObserver->setSlot(loadBinder);
  // connect to browse run
  m_view->observeBrowse(m_browseObserver);
  std::function<void(std::string)> browseBinder = std::bind(
      &ALFView_presenter::loadBrowsedFile, this, std::placeholders::_1);
  m_browseObserver->setSlot(browseBinder);

  initInstrument();
}

void ALFView_presenter::initInstrument() {
  // set up instrument
  std::function<bool(std::map<std::string, bool>)> extractConditionBinder =
      std::bind(&ALFView_presenter::extractTubeConditon, this,
                std::placeholders::_1);
  std::function<bool(std::map<std::string, bool>)> averageTubeConditonBinder =
      std::bind(&ALFView_presenter::averageTubeConditon, this,
                std::placeholders::_1);
  m_view->setUpInstrument(m_model->dataFileName(), extractConditionBinder,
                          averageTubeConditonBinder);

  // set up single tube extract
  m_view->observeExtractSingleTube(m_extractSingleTubeObserver);
  std::function<void()> extractSingleTubeBinder =
      std::bind(&ALFView_presenter::extractSingleTube, this);
  m_extractSingleTubeObserver->setSlot(extractSingleTubeBinder);

  // set up average tube
  m_view->observeAverageTube(m_averageTubeObserver);
  std::function<void()> averageTubeBinder =
      std::bind(&ALFView_presenter::averageTube, this);
  m_averageTubeObserver->setSlot(averageTubeBinder);
}

void ALFView_presenter::loadAndAnalysis(const std::string &run) {
  int runNumber = m_model->loadData(run);
  auto bools = m_model->isDataValid();
  if (bools["IsValidInstrument"]) {
    m_currentRun = runNumber;
  } else {
    loadAndAnalysis(instrument + std::to_string(m_currentRun));
    return;
  }
  m_numberOfTubesInAverage = 0; // reset the extracted tubes
  // if the displayed run number is out of sinc
  if (m_view->getRunNumber() != m_currentRun) {
    m_view->setRunQuietly(QString::number(m_currentRun));
  }
  if (bools["IsValidInstrument"] && !bools["IsItDSpace"]) {
    m_model->transformData();
  }
}

void ALFView_presenter::loadRunNumber() {
  int newRun = m_view->getRunNumber();
  const int currentRunInADS = m_model->currentRun();

  if (currentRunInADS == newRun) {
    return;
  }
  const std::string runNumber = instrument + std::to_string(newRun);
  // check its a valid run number
  try {
    Mantid::API::FileFinder::Instance().findRuns(runNumber)[0];
  } catch (...) {
    m_view->setRunQuietly(QString::number(m_currentRun));
    // if file has been deleted we should replace it
    if (currentRunInADS == -999) {
      loadAndAnalysis(instrument + std::to_string(m_currentRun));
    }
    return;
  }
  loadAndAnalysis(runNumber);
}

void ALFView_presenter::loadBrowsedFile(const std::string fileName) {
  m_model->loadData(fileName);
  loadAndAnalysis(fileName);
}

bool ALFView_presenter::extractTubeConditon(
    std::map<std::string, bool> tabBools) {
  try {

    bool ifCurve = (tabBools.find("plotStroed")->second ||
                    tabBools.find("hasCurve")->second);
    return (tabBools.find("isTube")->second && ifCurve);
  } catch (...) {
    return false;
  }
}

bool ALFView_presenter::averageTubeConditon(
    std::map<std::string, bool> tabBools) {
  try {

    bool ifCurve = (tabBools.find("plotStroed")->second ||
                    tabBools.find("hasCurve")->second);
    return (m_numberOfTubesInAverage > 0 && tabBools.find("isTube")->second &&
            ifCurve && m_model->hasTubeBeenExtracted(instrument+std::to_string(m_currentRun)));
  } catch (...) {
    return false;
  }
}

void ALFView_presenter::extractSingleTube() {
  m_model->storeSingleTube(instrument + std::to_string(m_currentRun));
  m_numberOfTubesInAverage = 1;
}

void ALFView_presenter::averageTube() {
  m_model->averageTube(m_numberOfTubesInAverage,
                       instrument + std::to_string(m_currentRun));
  m_numberOfTubesInAverage++;
}

} // namespace CustomInterfaces
} // namespace MantidQt