# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
from __future__ import (absolute_import, division, print_function)
import csv
import numpy as np


class BankDetectorCollection(object):

    def __init__(self):
        self.bank_list = []
        self.load_bank_list()
        pass

    def load_bank_list(self):
        # TODO replace dummy bank data with data loader

        DCS_read = csv.reader(open("C:\\Users\\wey38795\\Documents\\DCS.csv", "rb"), delimiter=",")
        DCS_list = list(DCS_read)
        DCS = np.array(DCS_list).astype("float")

        reader = csv.reader(open("C:\\Users\\wey38795\\Documents\\SLF.csv", "rb"), delimiter=",")
        x = list(reader)
        SLF = np.array(x).astype("float")

        q = DCS[:, 0]
        for i in np.arange(DCS[0, :].size - 1):
            dcs = DCS[:, i+1]
            slf = SLF[:, i+1]
            name = 'bank' + str(i)
            self.bank_list.append(BankDataSet(name, q, dcs, slf))

    def get_bank_list(self):
        return self.bank_list

    def get_merged(self):
        q = self.bank_list[0].get_q()
        merged = np.zeros(q.size)
        for i in np.arange(q.size):
            merge_x = 0
            n_detector = 0
            for bank in self.bank_list:
                q_min = bank.get_q_min()
                q_max = bank.get_q_max()
                if q_min <= q[i] <= q_max:
                    n_detector += 1
                    merge_x += bank.get_distinct_s(i)
            if not n_detector == 0:
                merged[i] = merge_x / n_detector
        return merged

    def get_alpha_table(self):
        table = [len(self.bank_list)]
        for i in np.arange(len(self.bank_list)):
            table.append([i, self.bank_list[i].get_alpha()])
        return table

    def get_limit_table(self):
        table = [len(self.bank_list)]
        for i in np.arange(len(self.bank_list)):
            q_min = self.bank_list[i].get_q_min()
            q_max = self.bank_list[i].get_q_max()
            if q_min < q_max:
                table.append([i, 1, q_min, q_max])
            else:
                table.append([i, 0])
        return table

    def get_linear_table(self):
        table = [len(self.bank_list)]
        for i in np.arange(len(self.bank_list)):
            grad = self.bank_list[i].get_grad()
            intercept = self.bank_list[i].get_intercept()
            if grad == 0 and intercept == 0:
                table.append([i, 0])
            else:
                table.append([i, 1, grad, intercept])
        return table


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
