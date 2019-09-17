# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
from __future__ import (absolute_import, division, print_function)
import numpy as np


class BankDataSet(object):

    def __init__(self, name, q, dcs, slf):
        self.alpha = None
        self.grad = None
        self.intercept = None
        self.q_min = None
        self.q_max = None
        self.dcs_2 = None
        self.int = None

        self.name = name
        self.q = q
        self.dcs = dcs
        self.slf = slf

        self.update_value(alpha=1.0, grad=0.0, intercept=0.0, q_min=min(q), q_max=max(q))

    def update_value(self, alpha=None, grad=None, intercept=None, q_min=None, q_max=None):
        if alpha:
            self.alpha = alpha
        if grad:
            self.grad = grad
        if intercept:
            self.intercept = intercept
        if q_min:
            self.q_min = q_min
        if q_max:
            self.q_max = q_max

        for x in self.q:
            if x >= self.q_min & x <= self.q_max:
                self.dcs_2 = np.add(self.alpha * self.dcs, self.grad * self.q + self.intercept)
                self.int = np.subtract(self.dcs_2, self.slf)

    def plot_dcs_2(self):
        x = np.array([])
        y = np.array([])
        for i in np.arange(self.q.size()):
            if self.q[i] >= self.q_min & self.q[i] <= self.q_max:
                np.append(x, self.q[i])
                np.append(y, self.dcs_2[i])
