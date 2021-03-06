// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2007 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
/***************************************************************************
    File                 : CustomActionDialog.h
    Project              : QtiPlot
    --------------------------------------------------------------------
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#pragma once

#include <QDialog>
#include <QXmlDefaultHandler>

class QGroupBox;
class QPushButton;
class QRadioButton;
class QComboBox;
class QListWidget;
class QLineEdit;
class QMenu;
class QToolBar;

class CustomActionDialog : public QDialog {
  Q_OBJECT

public:
  //! Constructor
  /**
   * @param parent :: parent widget (must be the application window!=
   * @param fl :: window flags
   */
  CustomActionDialog(QWidget *parent, Qt::WFlags fl = nullptr);

private slots:
  void chooseIcon();
  void chooseFile();
  void chooseFolder();
  QAction *addAction();
  void removeAction();
  void setCurrentAction(int);
  void saveCurrentAction();

private:
  void init();
  void updateDisplayList();
  QAction *actionAt(int row);
  void saveAction(QAction *action);
  void customizeAction(QAction *action);
  bool validUserInput();
  QStringList d_app_shortcut_keys;

  QList<QMenu *> d_menus;
  QList<QToolBar *> d_app_toolbars;

  QListWidget *itemsList;
  QPushButton *buttonCancel, *buttonAdd, *buttonRemove, *buttonSave;
  QPushButton *folderBtn, *fileBtn, *iconBtn;
  QLineEdit *folderBox, *fileBox, *iconBox, *textBox, *toolTipBox, *shortcutBox;
  QRadioButton *menuBtn, *toolBarBtn;
  QComboBox *menuBox, *toolBarBox;
};

class CustomActionHandler : public QXmlDefaultHandler {
public:
  explicit CustomActionHandler(QAction *action);

  bool startElement(const QString &namespaceURI, const QString &localName,
                    const QString &qName,
                    const QXmlAttributes &attributes) override;
  bool endElement(const QString &namespaceURI, const QString &localName,
                  const QString &qName) override;
  bool characters(const QString &str) override {
    currentText += str;
    return true;
  };
  bool fatalError(const QXmlParseException &) override { return false; };
  QString errorString() const override { return errorStr; };
  QString parentName() { return d_widget_name; };

private:
  bool metFitTag;
  QString currentText;
  QString errorStr;
  QString filePath;
  QString d_widget_name;
  QAction *d_action;
};