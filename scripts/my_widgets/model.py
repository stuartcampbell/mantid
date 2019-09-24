# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
from __future__ import (absolute_import, division, print_function)
import numpy as np


class BankDataSet(object):

    def __init__(self, name, q, total_s, self_s):
        self.alpha = None
        self.grad = None
        self.intercept = None
        self.q_min = None
        self.q_max = None
        self.total_s_2 = np.array([])
        self.distinct_s = np.array([])

        self.name = name
        self.q = q
        self.total_s = total_s
        self.self_s = self_s

        self.update_value(alpha=1.0, grad=0.0, intercept=0.0, q_min=min(q), q_max=max(q))

    def update_value(self, alpha=None, grad=None, intercept=None, q_min=None, q_max=None):
        if alpha is not None:
            self.alpha = alpha
        if grad is not None:
            self.grad = grad
        if intercept is not None:
            self.intercept = intercept
        if q_min is not None:
            self.q_min = q_min
        if q_max is not None:
            self.q_max = q_max

        self.total_s_2 = np.array([])
        self.distinct_s = np.array([])
        for x, total_s, self_s in zip(self.q, self.total_s, self.self_s):
            total_s_2 = self.alpha * total_s + self.grad * x + self.intercept
            distinct_s = total_s_2 - self_s

            self.total_s_2 = np.append(self.total_s_2, total_s_2)
            self.distinct_s = np.append(self.distinct_s, distinct_s)

    def get_alpha(self):
        return self.alpha

    def get_grad(self):
        return self.grad

    def get_intercept(self):
        return self.intercept

    def get_q_min(self):
        return self.q_min

    def get_q_max(self):
        return self.q_max

    def get_q(self):
        return self.q

    def get_self_s(self):
        return self.self_s

    def get_total_s_2(self, index=None):
        if not index:
            return self.total_s_2
        else:
            return self.total_s_2[index]

    def get_distinct_s(self, index=-1):
        if index < 0:
            return self.distinct_s
        else:
            return self.distinct_s[index]
