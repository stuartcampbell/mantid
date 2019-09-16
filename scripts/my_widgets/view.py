# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
from __future__ import (absolute_import, division, print_function)

from qtpy import QtWidgets
from qtpy.QtCore import Signal
from mantidqt.utils.qt import load_ui

Ui_MyWidget, _ = load_ui(__file__, "merge_banks.ui")


class View(QtWidgets.QWidget, Ui_MyWidget):
    # signals for use by parent widgets

    def __init__(self, parent=None):
        super(View, self).__init__(parent)
        self.setupUi(self)
