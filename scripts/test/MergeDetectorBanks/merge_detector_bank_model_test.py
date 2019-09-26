# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
from __future__ import (absolute_import, unicode_literals)

import math
import numpy as np
import unittest

from mantidqt.utils.qt.testing import start_qapplication
from MergeDetectorBanks import model


@start_qapplication
class BankDataSetTest(unittest.TestCase):
    def setUp(self):
        n = 600
        q = np.zeros(n)
        dcs = np.zeros(n)
        slf = np.zeros(n)
        for i in np.arange(n):
            q[i] = i/10.0
            dcs[i] = math.sin(i/10.0)
            slf[i] = 0.01 * i + 0.4
        self.bank = model.BankDataSet("TestBank", q, dcs, slf)

    def test_init_sets_default_correction(self):
        self.assertEqual(1.0, self.bank.get_alpha())
        self.assertEqual(0.0, self.bank.get_grad())
        self.assertEqual(0.0, self.bank.get_intercept())
        self.assertEqual(0.0, self.bank.get_q_min())
        self.assertEqual(59.9, self.bank.get_q_max())


if __name__ == '__main__':
    unittest.main(buffer=False, verbosity=2)
