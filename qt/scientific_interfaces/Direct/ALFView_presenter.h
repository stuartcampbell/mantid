// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2014 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#ifndef MANTIDQT_CUSTOMINTERFACES_ALFVIEWPRESENTER_H_
#define MANTIDQT_CUSTOMINTERFACES_ALFVIEWPRESENTER_H_

#include "ALFView_model.h"
#include "ALFView_view.h"
#include "DllConfig.h"
#include "MantidQtWidgets/Common/UserSubWindow.h"
#include "observerPattern.h"

namespace MantidQt {
namespace CustomInterfaces {
/** ALCInterface : Custom interface for Avoided Level Crossing analysis
 */
class MANTIDQT_DIRECT_DLL ALFView_presenter : public QObject {
  Q_OBJECT

public:
  ALFView_presenter(ALFView_view *view, ALFView_model *model);
  ~ALFView_presenter(){};
  void initLayout();

private slots:
  void loadRunNumber();
  void loadBrowsedFile(const std::string);
  void extractSingleTube();
  void averageTube();

private:
  void loadAndAnalysis(const std::string &run);
  bool extractTubeConditon(std::map<std::string, bool> tabBools);
  bool averageTubeConditon(std::map<std::string, bool> tabBools);
  void initInstrument();

  ALFView_view *m_view;
  ALFView_model *m_model;
  int m_currentRun;
  int m_numberOfTubesInAverage;
  voidObserver *m_loadRunObserver;
  generalObserver *m_browseObserver;
  voidObserver *m_extractSingleTubeObserver;
  voidObserver *m_averageTubeObserver;
};
} // namespace CustomInterfaces
} // namespace MantidQt

#endif /* MANTIDQT_CUSTOMINTERFACES_ALFVIEWPRESENTER_H_ */
