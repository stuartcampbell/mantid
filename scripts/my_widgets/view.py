# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
from __future__ import (absolute_import, division, print_function)

from qtpy import QtWidgets
from mantidqt.utils.qt import load_ui
import matplotlib.pyplot as plt
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas

Ui_MyWidget, _ = load_ui(__file__, "merge_banks.ui")


class View(QtWidgets.QWidget, Ui_MyWidget):

    def __init__(self, parent=None):
        super(View, self).__init__(parent)
        self.setupUi(self)

        self.figure_1 = plt.figure()
        self.canvas_1 = FigureCanvas(self.figure_1)
        self.plt_1.addWidget(self.canvas_1)

        self.figure_2 = plt.figure()
        self.canvas_2 = FigureCanvas(self.figure_2)
        self.plt_2.addWidget(self.canvas_2)

        self.figure_3 = plt.figure()
        self.canvas_3 = FigureCanvas(self.figure_3)
        self.plt_3.addWidget(self.canvas_3)
