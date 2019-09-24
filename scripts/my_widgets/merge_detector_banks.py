# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
# pylint: disable=invalid-name
from __future__ import (absolute_import, division, print_function)

import sys

from qtpy import QtWidgets, QtCore

from my_widgets import view
from my_widgets import presenter


class MergeDetectorBanks(QtWidgets.QMainWindow):
    def __init__(self, parent=None):
        super(MergeDetectorBanks, self).__init__(parent)

        self.window = QtWidgets.QMainWindow()

        my_view = view.View(parent=self)
        self.presenter = presenter.Presenter(my_view)

        # set the view for the main window
        self.setCentralWidget(my_view)
        self.setWindowTitle("Merge Detector Banks")


def qapp():
    if QtWidgets.QApplication.instance():
        _app = QtWidgets.QApplication.instance()
    else:
        _app = QtWidgets.QApplication(sys.argv)
    return _app


if __name__ == "__main__":
    app = qapp()
    window = MergeDetectorBanks()
    window.show()
    app.exec_()
