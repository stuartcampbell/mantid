// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#ifndef MANTIDQT_CUSTOMINSTRUMENTVIEW_DLLOPTION_H_
#define MANTIDQT_CUSTOMINSTRUMENTVIEW_DLLOPTION_H_

#include "MantidKernel/System.h"

#ifdef IN_MANTIDQT_CUSTOMINSTRUMENTVIEW
#define EXPORT_OPT_MANTIDQT_CUSTOMINSTRUMENTVIEW DLLExport
#define EXTERN_MANTIDQT_CUSTOMINSTRUMENTVIEW
#else
#define EXPORT_OPT_MANTIDQT_CUSTOMINSTRUMENTVIEW DLLImport
#define EXTERN_MANTIDQT_CUSTOMINSTRUMENTVIEW extern
#endif /* IN_MANTIDQT_CUSTOMINSTRUMENTVIEW */

#endif // MANTIDQT_CUSTOMINSTRUMENTVIEW_DLLOPTION_H_
