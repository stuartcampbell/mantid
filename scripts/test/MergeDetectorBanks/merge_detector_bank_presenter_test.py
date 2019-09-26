# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
from __future__ import (absolute_import, unicode_literals)

import unittest

from mantidqt.utils.qt.testing import start_qapplication
from mantid.py3compat import mock
from MergeDetectorBanks import presenter


@start_qapplication
class MergeDetectorBanksPresenterTest(unittest.TestCase):
    def setUp(self):
        self.view = mock.Mock()
        self.gui = presenter.Presenter(self.view)

    def test_changed_alpha_does_not_replot_if_value_is_same_as_bank_data(self):
        self.gui.active_bank = mock.Mock()
        self.gui.active_bank.get_alpha.return_value = 5
        self.gui._view.alpha.text.return_value = 5
        self.gui.update_plots = mock.Mock()

        self.gui.changed_alpha()

        self.assertFalse(self.gui.update_plots.called)

    def test_changed_grad_does_not_replot_if_value_is_same_as_bank_data(self):
        self.gui.active_bank = mock.Mock()
        self.gui.active_bank.get_grad.return_value = 5
        self.gui._view.grad.text.return_value = 5
        self.gui.update_plots = mock.Mock()

        self.gui.changed_grad()

        self.assertFalse(self.gui.update_plots.called)

    def test_changed_intercept_does_not_replot_if_value_is_same_as_bank_data(self):
        self.gui.active_bank = mock.Mock()
        self.gui.active_bank.get_intercept.return_value = 5
        self.gui._view.intercept.text.return_value = 5
        self.gui.update_plots = mock.Mock()

        self.gui.changed_intercept()

        self.assertFalse(self.gui.update_plots.called)


if __name__ == '__main__':
    unittest.main(buffer=False, verbosity=2)
