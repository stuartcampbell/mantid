// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#include "IndirectDataTablePresenterLegacy.h"

#include "MantidQtWidgets/Common/SignalBlocker.h"

#include <boost/numeric/conversion/cast.hpp>

#include <QHeaderView>
#include <QItemDelegate>
#include <QLineEdit>
#include <QRegExpValidator>

namespace {
using MantidQt::CustomInterfaces::IDA::DiscontinuousSpectra;
using MantidQt::CustomInterfaces::IDA::SpectraLegacy;

namespace Regexes {
const QString EMPTY = "^$";
const QString SPACE = "(\\s)*";
const QString COMMA = SPACE + "," + SPACE;
const QString NATURAL_NUMBER = "(0|[1-9][0-9]*)";
const QString REAL_NUMBER = "(-?" + NATURAL_NUMBER + "(\\.[0-9]*)?)";
const QString REAL_RANGE = "(" + REAL_NUMBER + COMMA + REAL_NUMBER + ")";
const QString MASK_LIST =
    "(" + REAL_RANGE + "(" + COMMA + REAL_RANGE + ")*" + ")|" + EMPTY;
} // namespace Regexes

class ExcludeRegionDelegate : public QItemDelegate {
public:
  QWidget *createEditor(QWidget *parent,
                        const QStyleOptionViewItem & /*option*/,
                        const QModelIndex & /*index*/) const override {
    auto lineEdit = std::make_unique<QLineEdit>(parent);
    auto validator =
        std::make_unique<QRegExpValidator>(QRegExp(Regexes::MASK_LIST), parent);
    lineEdit->setValidator(validator.release());
    return lineEdit.release();
  }

  void setEditorData(QWidget *editor, const QModelIndex &index) const override {
    const auto value = index.model()->data(index, Qt::EditRole).toString();
    static_cast<QLineEdit *>(editor)->setText(value);
  }

  void setModelData(QWidget *editor, QAbstractItemModel *model,
                    const QModelIndex &index) const override {
    auto *lineEdit = static_cast<QLineEdit *>(editor);
    model->setData(index, lineEdit->text(), Qt::EditRole);
  }

  void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                            const QModelIndex & /*index*/) const override {
    editor->setGeometry(option.rect);
  }
};

QStringList defaultHeaders() {
  QStringList headers;
  headers << "Workspace"
          << "WS Index"
          << "StartX"
          << "EndX"
          << "Mask X Range";
  return headers;
}

QString makeNumber(double d) { return QString::number(d, 'g', 16); }

std::string
pairsToString(const std::vector<std::pair<std::size_t, std::size_t>> &pairs) {
  std::vector<std::string> pairStrings;
  for (auto const &value : pairs) {
    if (value.first == value.second)
      pairStrings.emplace_back(std::to_string(value.first));
    else
      pairStrings.emplace_back(std::to_string(value.first) + "-" +
                               std::to_string(value.second));
  }
  return boost::algorithm::join(pairStrings, ",");
}

boost::optional<SpectraLegacy>
pairsToSpectra(const std::vector<std::pair<std::size_t, std::size_t>> &pairs) {
  if (pairs.empty())
    return boost::none;
  else if (pairs.size() == 1)
    return boost::optional<SpectraLegacy>(pairs[0]);
  return boost::optional<SpectraLegacy>(
      DiscontinuousSpectra<std::size_t>(pairsToString(pairs)));
}

QVariant getVariant(std::size_t i) {
  return QVariant::fromValue<qulonglong>(i);
}
} // namespace

namespace MantidQt {
namespace CustomInterfaces {
namespace IDA {

IndirectDataTablePresenterLegacy::IndirectDataTablePresenterLegacy(
    IndirectFittingModelLegacy *model, QTableWidget *dataTable)
    : IndirectDataTablePresenterLegacy(model, dataTable, defaultHeaders()) {}

IndirectDataTablePresenterLegacy::IndirectDataTablePresenterLegacy(
    IndirectFittingModelLegacy *model, QTableWidget *dataTable,
    const QStringList &headers)
    : m_model(model), m_dataTable(dataTable) {
  setHorizontalHeaders(headers);
  m_dataTable->setItemDelegateForColumn(
      headers.size() - 1, std::make_unique<ExcludeRegionDelegate>().release());
  m_dataTable->verticalHeader()->setVisible(false);

  connect(m_dataTable, SIGNAL(cellChanged(int, int)), this,
          SLOT(setModelFittingRange(int, int)));
  connect(m_dataTable, SIGNAL(cellChanged(int, int)), this,
          SLOT(updateAllFittingRangeFrom(int, int)));
}

bool IndirectDataTablePresenterLegacy::tableDatasetsMatchModel() const {
  if (m_dataPositions.size() != m_model->numberOfWorkspaces())
    return false;

  for (auto i = 0u; i < m_dataPositions.size(); ++i) {
    if (m_model->getWorkspace(i)->getName() !=
        getWorkspaceName(m_dataPositions[i]))
      return false;
  }
  return true;
}

bool IndirectDataTablePresenterLegacy::isTableEmpty() const {
  return m_dataPositions.empty();
}

int IndirectDataTablePresenterLegacy::workspaceIndexColumn() const { return 1; }

int IndirectDataTablePresenterLegacy::startXColumn() const { return 2; }

int IndirectDataTablePresenterLegacy::endXColumn() const { return 3; }

int IndirectDataTablePresenterLegacy::excludeColumn() const { return 4; }

double IndirectDataTablePresenterLegacy::startX(int row) const {
  return getDouble(row, startXColumn());
}

double IndirectDataTablePresenterLegacy::endX(int row) const {
  return getDouble(row, endXColumn());
}

std::string IndirectDataTablePresenterLegacy::getExcludeString(int row) const {
  return getString(row, excludeColumn());
}

std::string IndirectDataTablePresenterLegacy::getWorkspaceName(int row) const {
  return getString(row, 0);
}

std::size_t IndirectDataTablePresenterLegacy::getWorkspaceIndex(int row) const {
  const auto item = m_dataTable->item(row, workspaceIndexColumn());
  return static_cast<std::size_t>(item->text().toULongLong());
}

double IndirectDataTablePresenterLegacy::getDouble(int row, int column) const {
  return getText(row, column).toDouble();
}

std::string IndirectDataTablePresenterLegacy::getString(int row,
                                                        int column) const {
  return getText(row, column).toStdString();
}

QString IndirectDataTablePresenterLegacy::getText(int row, int column) const {
  return m_dataTable->item(row, column)->text();
}

int IndirectDataTablePresenterLegacy::getNextPosition(std::size_t index) const {
  if (m_dataPositions.size() > index + 1)
    return m_dataPositions[index + 1];
  return m_dataTable->rowCount();
}

int IndirectDataTablePresenterLegacy::getFirstRow(std::size_t dataIndex) const {
  if (m_dataPositions.size() > dataIndex)
    return m_dataPositions[dataIndex];
  return -1;
}

std::size_t IndirectDataTablePresenterLegacy::getDataIndex(int row) const {
  return m_dataTable->item(row, 0)->data(Qt::UserRole).toULongLong();
}

boost::optional<SpectraLegacy>
IndirectDataTablePresenterLegacy::getSpectra(std::size_t dataIndex) const {
  if (m_dataPositions.size() > dataIndex)
    return getSpectra(m_dataPositions[dataIndex], getNextPosition(dataIndex));
  return boost::none;
}

boost::optional<SpectraLegacy>
IndirectDataTablePresenterLegacy::getSpectra(int start, int end) const {
  std::vector<std::pair<std::size_t, std::size_t>> spectraPairs;
  while (start < end) {
    std::size_t minimum = getWorkspaceIndex(start);
    std::size_t maximum = minimum;

    while (++start < end && getWorkspaceIndex(start) == maximum + 1)
      ++maximum;
    spectraPairs.emplace_back(minimum, maximum);
  }
  return pairsToSpectra(spectraPairs);
}

boost::optional<int>
IndirectDataTablePresenterLegacy::getRowIndex(std::size_t dataIndex,
                                              int spectrumIndex) const {
  const auto position = m_dataPositions[dataIndex] + spectrumIndex;
  if (getNextPosition(dataIndex) > position)
    return position;
  return boost::none;
}

void IndirectDataTablePresenterLegacy::setStartX(double startX,
                                                 std::size_t dataIndex,
                                                 int spectrumIndex) {
  if (FittingModeLegacy::SEQUENTIAL == m_model->getFittingMode())
    setStartX(startX);
  else if (auto row = getRowIndex(dataIndex, spectrumIndex))
    setStartX(startX, *row);
}

void IndirectDataTablePresenterLegacy::setEndX(double endX,
                                               std::size_t dataIndex,
                                               int spectrumIndex) {
  if (FittingModeLegacy::SEQUENTIAL == m_model->getFittingMode())
    setEndX(endX);
  else if (auto row = getRowIndex(dataIndex, spectrumIndex))
    setEndX(endX, *row);
}

void IndirectDataTablePresenterLegacy::setExclude(const std::string &exclude,
                                                  std::size_t dataIndex,
                                                  int spectrumIndex) {
  if (FittingModeLegacy::SEQUENTIAL == m_model->getFittingMode())
    setExcludeRegion(exclude);
  else if (auto row = getRowIndex(dataIndex, spectrumIndex))
    setExcludeRegion(exclude, *row);
}

void IndirectDataTablePresenterLegacy::addData(std::size_t index) {
  if (m_dataPositions.size() > index)
    updateData(index);
  else
    addNewData(index);
}

void IndirectDataTablePresenterLegacy::addNewData(std::size_t index) {
  MantidQt::API::SignalBlocker blocker(m_dataTable);
  const auto start = m_dataTable->rowCount();

  const auto addRow = [&](std::size_t spectrum) {
    addTableEntry(index, spectrum);
  };
  m_model->applySpectra(index, addRow);

  if (m_model->numberOfWorkspaces() > m_dataPositions.size())
    m_dataPositions.emplace_back(start);
}

void IndirectDataTablePresenterLegacy::updateData(std::size_t index) {
  if (m_dataPositions.size() > index)
    updateExistingData(index);
  else
    addNewData(index);
}

void IndirectDataTablePresenterLegacy::updateExistingData(std::size_t index) {
  MantidQt::API::SignalBlocker blocker(m_dataTable);
  auto position = m_dataPositions[index];
  const auto nextPosition = getNextPosition(index);
  const auto initialSize = nextPosition - position;

  const auto updateRow = [&](std::size_t spectrum) {
    if (position < nextPosition)
      updateTableEntry(index, spectrum, position++);
    else
      addTableEntry(index, spectrum, position++);
  };
  m_model->applySpectra(index, updateRow);

  collapseData(position, nextPosition, initialSize, index);
}

void IndirectDataTablePresenterLegacy::collapseData(int from, int to,
                                                    int initialSize,
                                                    std::size_t dataIndex) {
  const auto shift = from - to;
  if (shift != 0) {
    for (auto i = from; i < to; ++i)
      removeTableEntry(from);

    if (initialSize + shift == 0 && m_dataPositions.size() > dataIndex) {
      m_dataPositions.erase(m_dataPositions.begin() + dataIndex);
      shiftDataPositions(shift, dataIndex, m_dataPositions.size());
      updateDataPositionsInCells(dataIndex, m_dataPositions.size());
    } else
      shiftDataPositions(shift, dataIndex + 1, m_dataPositions.size());
  }
}

void IndirectDataTablePresenterLegacy::removeSelectedData() {
  MantidQt::API::SignalBlocker blocker(m_dataTable);
  auto selectedIndices = m_dataTable->selectionModel()->selectedIndexes();
  const auto modifiedIndicesAndCount = removeTableRows(selectedIndices);
  const auto &modifiedCount = modifiedIndicesAndCount.second;
  auto &modifiedIndices = modifiedIndicesAndCount.first;

  for (auto i = 0u; i < modifiedIndices.size(); ++i)
    shiftDataPositions(-static_cast<int>(modifiedCount[i]),
                       modifiedIndices[i] + 1, m_dataPositions.size());

  if (!modifiedIndices.empty()) {
    updateFromRemovedIndices(modifiedIndices);
    updateDataPositionsInCells(
        modifiedIndices.back() > 0 ? modifiedIndices.back() - 1 : 0,
        m_dataPositions.size());
  }
}

void IndirectDataTablePresenterLegacy::updateFromRemovedIndices(
    const std::vector<std::size_t> &indices) {
  for (const auto &index : indices) {
    const auto existingSpectra = getSpectra(index);
    if (existingSpectra)
      m_model->setSpectra(*existingSpectra, index);
    else {
      const auto originalNumberOfWorkspaces = m_model->numberOfWorkspaces();
      m_model->removeWorkspace(index);
      m_dataPositions.erase(m_dataPositions.begin() + index);

      if (m_model->numberOfWorkspaces() == originalNumberOfWorkspaces - 2)
        m_dataPositions.erase(m_dataPositions.begin() + index);
    }
  }
}

std::pair<std::vector<std::size_t>, std::vector<std::size_t>>
IndirectDataTablePresenterLegacy::removeTableRows(
    QModelIndexList &selectedRows) {
  std::vector<std::size_t> modifiedIndices;
  std::vector<std::size_t> modifiedCount;
  int previous = -1;

  qSort(selectedRows);
  for (auto i = selectedRows.count() - 1; i >= 0; --i) {
    const auto current = selectedRows[i].row();
    if (current != previous) {
      auto modifiedIndex = removeTableEntry(current);

      if (!modifiedIndices.empty() && modifiedIndices.back() == modifiedIndex)
        ++modifiedCount.back();
      else {
        modifiedIndices.emplace_back(modifiedIndex);
        modifiedCount.emplace_back(1);
      }
      previous = current;
    }
  }
  return {modifiedIndices, modifiedCount};
}

void IndirectDataTablePresenterLegacy::setModelFittingRange(int row,
                                                            int column) {
  const auto workspaceIndex = getWorkspaceIndex(row);
  const auto dataIndex = getDataIndex(row);

  if (startXColumn() == column)
    setModelStartXAndEmit(getDouble(row, column), dataIndex, workspaceIndex);
  else if (endXColumn() == column)
    setModelEndXAndEmit(getDouble(row, column), dataIndex, workspaceIndex);
  else if (excludeColumn() == column)
    setModelExcludeAndEmit(getString(row, column), dataIndex, workspaceIndex);
}

void IndirectDataTablePresenterLegacy::setModelStartXAndEmit(
    double startX, std::size_t dataIndex, std::size_t workspaceIndex) {
  m_model->setStartX(startX, dataIndex, workspaceIndex);
  emit startXChanged(startX, dataIndex, workspaceIndex);
}

void IndirectDataTablePresenterLegacy::setModelEndXAndEmit(
    double endX, std::size_t dataIndex, std::size_t workspaceIndex) {
  m_model->setEndX(endX, dataIndex, workspaceIndex);
  emit endXChanged(endX, dataIndex, workspaceIndex);
}

void IndirectDataTablePresenterLegacy::setModelExcludeAndEmit(
    const std::string &exclude, std::size_t dataIndex,
    std::size_t workspaceIndex) {
  m_model->setExcludeRegion(exclude, dataIndex, workspaceIndex);
  emit excludeRegionChanged(exclude, dataIndex, workspaceIndex);
}

void IndirectDataTablePresenterLegacy::setGlobalFittingRange(bool global) {
  if (global)
    enableGlobalFittingRange();
  else
    disableGlobalFittingRange();
}

void IndirectDataTablePresenterLegacy::updateAllFittingRangeFrom(int row,
                                                                 int column) {
  MantidQt::API::SignalBlocker blocker(m_dataTable);
  if (startXColumn() == column)
    setStartX(getDouble(row, column));
  else if (endXColumn() == column)
    setEndX(getDouble(row, column));
  else if (excludeColumn() == column)
    setExcludeRegion(getText(row, column));
}

void IndirectDataTablePresenterLegacy::enableGlobalFittingRange() {
  MantidQt::API::SignalBlocker blocker(m_dataTable);
  const auto range = m_model->getFittingRange(0, 0);
  setStartX(range.first);
  setEndX(range.second);
  setExcludeRegion(m_model->getExcludeRegion(0, 0));
  connect(m_dataTable, SIGNAL(cellChanged(int, int)), this,
          SLOT(updateAllFittingRangeFrom(int, int)));
}

void IndirectDataTablePresenterLegacy::disableGlobalFittingRange() {
  disconnect(m_dataTable, SIGNAL(cellChanged(int, int)), this,
             SLOT(updateAllFittingRangeFrom(int, int)));
}

void IndirectDataTablePresenterLegacy::enableTable() {
  m_dataTable->setEnabled(true);
}

void IndirectDataTablePresenterLegacy::disableTable() {
  m_dataTable->setDisabled(true);
}

void IndirectDataTablePresenterLegacy::clearTable() {
  m_dataTable->setRowCount(0);
  m_dataPositions.clear();
}

void IndirectDataTablePresenterLegacy::setStartX(double startX, int index) {
  MantidQt::API::SignalBlocker blocker(m_dataTable);
  if (FittingModeLegacy::SEQUENTIAL == m_model->getFittingMode())
    setStartX(startX);
  else
    m_dataTable->item(index, startXColumn())->setText(makeNumber(startX));
}

void IndirectDataTablePresenterLegacy::setEndX(double endX, int index) {
  MantidQt::API::SignalBlocker blocker(m_dataTable);
  if (FittingModeLegacy::SEQUENTIAL == m_model->getFittingMode())
    setEndX(endX);
  else
    m_dataTable->item(index, endXColumn())->setText(makeNumber(endX));
}

void IndirectDataTablePresenterLegacy::setExcludeRegion(
    const std::string &exclude, int index) {
  MantidQt::API::SignalBlocker blocker(m_dataTable);
  if (FittingModeLegacy::SEQUENTIAL == m_model->getFittingMode())
    setExcludeRegion(exclude);
  else
    m_dataTable->item(index, excludeColumn())
        ->setText(QString::fromStdString(exclude));
}

void IndirectDataTablePresenterLegacy::setStartX(double startX) {
  setColumnValues(startXColumn(), makeNumber(startX));
}

void IndirectDataTablePresenterLegacy::setEndX(double endX) {
  setColumnValues(endXColumn(), makeNumber(endX));
}

void IndirectDataTablePresenterLegacy::setExcludeRegion(
    const std::string &exclude) {
  setExcludeRegion(QString::fromStdString(exclude));
}

void IndirectDataTablePresenterLegacy::setExcludeRegion(
    const QString &exclude) {
  setColumnValues(excludeColumn(), exclude);
}

void IndirectDataTablePresenterLegacy::setColumnValues(int column,
                                                       const QString &value) {
  MantidQt::API::SignalBlocker blocker(m_dataTable);
  for (int i = 0; i < m_dataTable->rowCount(); ++i)
    m_dataTable->item(i, column)->setText(value);
}

void IndirectDataTablePresenterLegacy::setHorizontalHeaders(
    const QStringList &headers) {
  m_dataTable->setColumnCount(headers.size());
  m_dataTable->setHorizontalHeaderLabels(headers);

  auto header = m_dataTable->horizontalHeader();
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  header->setResizeMode(0, QHeaderView::Stretch);
#elif QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  header->setSectionResizeMode(0, QHeaderView::Stretch);
#endif
}

void IndirectDataTablePresenterLegacy::addTableEntry(std::size_t dataIndex,
                                                     std::size_t spectrum) {
  const auto row = m_dataTable->rowCount();
  addTableEntry(dataIndex, spectrum, row);
  m_dataTable->item(row, 0)->setData(Qt::UserRole, getVariant(dataIndex));
}

void IndirectDataTablePresenterLegacy::addTableEntry(std::size_t dataIndex,
                                                     std::size_t spectrum,
                                                     int row) {
  m_dataTable->insertRow(row);

  const auto &name = m_model->getWorkspace(dataIndex)->getName();
  auto cell = std::make_unique<QTableWidgetItem>(QString::fromStdString(name));
  auto flags = cell->flags();
  flags ^= Qt::ItemIsEditable;
  cell->setFlags(flags);
  setCell(std::move(cell), row, 0);

  cell = std::make_unique<QTableWidgetItem>(QString::number(spectrum));
  cell->setFlags(flags);
  setCell(std::move(cell), row, workspaceIndexColumn());

  const auto range = m_model->getFittingRange(dataIndex, spectrum);
  cell = std::make_unique<QTableWidgetItem>(makeNumber(range.first));
  setCell(std::move(cell), row, startXColumn());

  cell = std::make_unique<QTableWidgetItem>(makeNumber(range.second));
  setCell(std::move(cell), row, endXColumn());

  const auto exclude = m_model->getExcludeRegion(dataIndex, spectrum);
  cell = std::make_unique<QTableWidgetItem>(QString::fromStdString(exclude));
  setCell(std::move(cell), row, excludeColumn());
}

void IndirectDataTablePresenterLegacy::setCell(
    std::unique_ptr<QTableWidgetItem> cell, int row, int column) {
  m_dataTable->setItem(row, column, cell.release());
}

void IndirectDataTablePresenterLegacy::updateTableEntry(std::size_t dataIndex,
                                                        std::size_t spectrum,
                                                        int row) {
  const auto &name = m_model->getWorkspace(dataIndex)->getName();
  setCellText(QString::fromStdString(name), row, 0);
  setCellText(QString::number(spectrum), row, workspaceIndexColumn());

  const auto range = m_model->getFittingRange(dataIndex, spectrum);
  setCellText(makeNumber(range.first), row, startXColumn());
  setCellText(makeNumber(range.second), row, endXColumn());

  const auto exclude = m_model->getExcludeRegion(dataIndex, spectrum);
  setCellText(QString::fromStdString(exclude), row, excludeColumn());
}

void IndirectDataTablePresenterLegacy::setCellText(const QString &text, int row,
                                                   int column) {
  m_dataTable->item(row, column)->setText(text);
}

std::size_t IndirectDataTablePresenterLegacy::removeTableEntry(int row) {
  const auto dataIndex = m_dataTable->item(row, 0)->data(Qt::UserRole);
  m_dataTable->removeRow(row);
  return dataIndex.toULongLong();
}

void IndirectDataTablePresenterLegacy::shiftDataPositions(int shift,
                                                          std::size_t from,
                                                          std::size_t to) {
  for (auto i = from; i < to; ++i)
    m_dataPositions[i] += shift;
}

void IndirectDataTablePresenterLegacy::updateDataPositionsInCells(
    std::size_t from, std::size_t to) {
  for (auto i = from; i < to; ++i) {
    const auto nextPosition = getNextPosition(i);
    for (auto row = m_dataPositions[i]; row < nextPosition; ++row)
      m_dataTable->item(row, 0)->setData(Qt::UserRole, getVariant(i));
  }
}

} // namespace IDA
} // namespace CustomInterfaces
} // namespace MantidQt