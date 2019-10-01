// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2014 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#ifndef MANTIDQT_CUSTOMINTERFACES_ALFVIEW_VIEW_H_
#define MANTIDQT_CUSTOMINTERFACES_ALFVIEW_VIEW_H_

#include "DllConfig.h"
#include "MantidQtWidgets/InstrumentView/InstrumentWidget.h"
#include "observerPattern.h"

#include <QAction>
#include <QLineEdit>
#include <QObject>
#include <QPushButton>
#include <QSplitter>
#include <string>

namespace MantidQt {
namespace CustomInterfaces {

class ALFView_view : public QSplitter {
  Q_OBJECT

public:
  explicit ALFView_view(QWidget *parent = nullptr);
  int getRunNumber();
  void setRunQuietly(const QString runNumber);
  void observeLoadRun(observer *listener) {
    m_loadRunObservable->attach(listener);
  };
  void observeBrowse(observer *listner) {
    m_browseObservable->attach(listner);
  };
  void observeExtractSingleTube(observer *listner) {
    m_extractSingleTubeObservable->attach(listner);
  }
  void observeAverageTube(observer *listner) {
    m_averageTubeObservable->attach(listner);
  }
  void setUpInstrument(
      const std::string fileName,
      std::function<bool(std::map<std::string, bool>)> &extractBinder,
      std::function<bool(std::map<std::string, bool>)> &averageBinder);
public slots:
  void runChanged();
  void browse();
  void extractSingleTube();
  void averageTube();

      signals : void newRun();
  void browsedToRun(std::string);

private:
  void generateLoadWidget(QWidget *loadBar);
  QSplitter *m_mainLayout;
  QLineEdit *m_run;
  QPushButton *m_browse;
  observable *m_loadRunObservable;
  observable *m_browseObservable;
  observable *m_extractSingleTubeObservable;
  observable *m_averageTubeObservable;
  MantidWidgets::InstrumentWidget *m_instrument;
  QAction *m_extractAction;
  QAction *m_averageAction;
};
} // namespace CustomInterfaces
} // namespace MantidQt

#endif /* MANTIDQT_CUSTOMINTERFACES_ALFVIEW_VIEW_H_ */
