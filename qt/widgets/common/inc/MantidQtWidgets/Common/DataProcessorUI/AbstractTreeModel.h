// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2014 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include "MantidAPI/ITableWorkspace_fwd.h"
#include "MantidQtWidgets/Common/DataProcessorUI/WhiteList.h"
#include "MantidQtWidgets/Common/DllOption.h"
#include <QAbstractItemModel>
#include <QColor>

namespace MantidQt {
namespace MantidWidgets {
namespace DataProcessor {

namespace Colour {
constexpr const char *FAILED =
    "#accbff"; // processing completed with error (blue)
constexpr const char *SUCCESS =
    "#d0f4d0"; // processing completed successfully (green)
constexpr const char *COMPLETE =
    "#f2fcf2"; // complete but no processing was required (pale green)
} // namespace Colour

class RowData;
using RowData_sptr = std::shared_ptr<RowData>;

/** AbstractTreeModel is a base class for several tree model
implementations for processing table data. Full function implementation is
provided for functions common to all data processing tree models, while
these subclasses are expected to provide implementation for the remaining
virtual functions defined here and in QAbstractItemModel.
*/
class EXPORT_OPT_MANTIDQT_COMMON AbstractTreeModel : public QAbstractItemModel {
  Q_OBJECT
public:
  AbstractTreeModel(Mantid::API::ITableWorkspace_sptr tableWorkspace,
                    const WhiteList &whitelist);
  ~AbstractTreeModel() override;

  // Functions to read data from the model

  // Column count
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  // Get flags for a cell
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  // Get the 'processed' status of a data item
  virtual bool isProcessed(int position,
                           const QModelIndex &parent = QModelIndex()) const = 0;
  // Set the 'processed' status of a data item
  virtual bool setProcessed(bool processed, int position,
                            const QModelIndex &parent = QModelIndex()) = 0;
  // Check whether reduction failed for a data item
  virtual bool
  reductionFailed(int position,
                  const QModelIndex &parent = QModelIndex()) const = 0;
  // Set the error message for a data item
  virtual bool setError(const std::string &error, int position,
                        const QModelIndex &parent = QModelIndex()) = 0;
  // Get the row metadata
  virtual RowData_sptr rowData(const QModelIndex &index) const = 0;
  // Transfer rows into the table
  virtual void
  transfer(const std::vector<std::map<QString, QString>> &runs) = 0;

protected:
  /// Collection of data for viewing.
  Mantid::API::ITableWorkspace_sptr m_tWS;
  /// Map of column indexes to names and viceversa
  WhiteList m_whitelist;
};
} // namespace DataProcessor
} // namespace MantidWidgets
} // namespace MantidQt
