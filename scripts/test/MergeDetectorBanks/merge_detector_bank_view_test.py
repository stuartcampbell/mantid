# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
from __future__ import (absolute_import, unicode_literals)

import unittest

from qtpy import QtGui
from mantidqt.utils.qt.testing import start_qapplication
from MergeDetectorBanks import view


@start_qapplication
class MergeDetectorBanksViewTest(unittest.TestCase):
    def setUp(self):
        self.view = view.View()

    def test_line_edits_have_all_validators(self):
        self.assertIsInstance(self.view.alpha.validator(), QtGui.QDoubleValidator)
        self.assertIsInstance(self.view.grad.validator(), QtGui.QDoubleValidator)
        self.assertIsInstance(self.view.intercept.validator(), QtGui.QDoubleValidator)
        self.assertIsInstance(self.view.q_min.validator(), QtGui.QDoubleValidator)
        self.assertIsInstance(self.view.q_max.validator(), QtGui.QDoubleValidator)


if __name__ == '__main__':
    unittest.main(buffer=False, verbosity=2)
