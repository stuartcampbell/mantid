// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#include "ALFView_view.h"
#include "MantidQtWidgets/InstrumentView/InstrumentWidgetPickTab.h"

#include <QFileDialog>
#include <QGridLayout>
#include <QLineEdit>
#include <QRegExpValidator>
#include <QVBoxLayout>

namespace MantidQt {
namespace CustomInterfaces {

ALFView_view::ALFView_view(QWidget *parent)
    : QSplitter(Qt::Vertical, parent), m_mainLayout(nullptr), m_run(nullptr),
      m_loadRunObservable(nullptr), m_browseObservable(nullptr),
      m_extractSingleTubeObservable(nullptr),
      m_averageTubeObservable(nullptr),
      m_instrument(nullptr) {

  QWidget *loadBar = new QWidget();
  m_loadRunObservable = new observable();
  m_browseObservable = new observable();
  m_extractSingleTubeObservable = new observable();
  m_averageTubeObservable = new observable();
  generateLoadWidget(loadBar);

  this->addWidget(loadBar);
}

void ALFView_view::generateLoadWidget(QWidget *loadBar) {
  m_run = new QLineEdit("0");
  m_run->setValidator(new QRegExpValidator(QRegExp("[0-9]*"), m_run));
  connect(m_run, SIGNAL(editingFinished()), this, SLOT(runChanged()));

  m_browse = new QPushButton("Browse");
  connect(m_browse, SIGNAL(clicked()), this, SLOT(browse()));

  QHBoxLayout *loadLayout = new QHBoxLayout(loadBar);
  loadLayout->addWidget(m_run);
  loadLayout->addWidget(m_browse);
}

int ALFView_view::getRunNumber() { return m_run->text().toInt(); }

void ALFView_view::setRunQuietly(const QString runNumber) {
  m_run->blockSignals(true);
  m_run->setText(runNumber);
  m_run->blockSignals(false);
}

void ALFView_view::runChanged() { m_loadRunObservable->notify(); }

void ALFView_view::browse() {
  auto file =
      QFileDialog::getOpenFileName(this, "Open a file", "", "File (*.nxs)");
  if (file.isEmpty()) {
    return;
  }
  m_browseObservable->notify(file.toStdString());
}

void ALFView_view::setUpInstrument(
    std::string fileName,
    std::function<bool(std::map<std::string, bool>)> &extractBinder,
    std::function<bool(std::map<std::string, bool>)> &averageBinder) {
  m_instrument = new MantidWidgets::InstrumentWidget("ALFData");
  m_instrument->removeTab("Instrument");
  m_instrument->removeTab("Draw");
  this->addWidget(m_instrument);

  // set up extract single tube
  m_extractAction = new QAction("Extract Single Tube", this);
  connect(m_extractAction, SIGNAL(triggered()), this,
          SLOT(extractSingleTube())),
  m_instrument->addToContextMenu("Pick", m_extractAction, extractBinder);

  // set up add to average
  m_averageAction = new QAction("Add Tube To Average", this);
  connect(m_averageAction, SIGNAL(triggered()), this,
          SLOT(averageTube())),
      m_instrument->addToContextMenu("Pick", m_averageAction, averageBinder);
}

void ALFView_view::extractSingleTube() {
  m_instrument->getPickTab()->savePlotToWorkspace();
	  
  m_extractSingleTubeObservable->notify();
}

void ALFView_view::averageTube() {
  m_instrument->getPickTab()->savePlotToWorkspace();
  m_averageTubeObservable->notify();
}

} // namespace CustomInterfaces
} // namespace MantidQt