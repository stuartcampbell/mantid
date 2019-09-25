# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
from __future__ import (absolute_import, division, print_function)
from mantid.simpleapi import CreateWorkspace
from my_widgets import model
import csv
import numpy as np


class Presenter(object):

    def __init__(self, view):
        self.active_bank = None
        self.bank_list = None

        self._view = view
        self.bank_list = self.get_bank_list()
        for bank in self.bank_list:
            self._view.detector_bank.addItem(bank.name)
        self.connect_view_signals()
        self.set_active_bank(0)

    def connect_view_signals(self):
        self._view.alpha.editingFinished.connect(self.changed_alpha)
        self._view.grad.editingFinished.connect(self.changed_grad)
        self._view.intercept.editingFinished.connect(self.changed_intercept)
        self._view.q_min.editingFinished.connect(self.changed_q_min)
        self._view.q_max.editingFinished.connect(self.changed_q_max)
        self._view.detector_bank.currentIndexChanged.connect(self.changed_detector_bank)
        self._view.output_merged_workspace_btn.clicked.connect(self.output_wksp)

    def changed_alpha(self):
        try:
            alpha = self._view.alpha.text()
            self.active_bank.update_value(alpha=float(alpha))
            self.update_plots()
        except TypeError:
            self._view.alpha.setText(self.active_bank.alpha)

    def changed_grad(self):
        try:
            grad = self._view.grad.text()
            self.active_bank.update_value(grad=float(grad))
            self.update_plots()
        except TypeError:
            self._view.grad.setText(self.active_bank.grad)

    def changed_intercept(self):
        try:
            intercept = self._view.intercept.text()
            self.active_bank.update_value(intercept=float(intercept))
            self.update_plots()
        except TypeError:
            self._view.intercept.setText(self.active_bank.intercept)

    def changed_q_min(self):
        try:
            q_min = self._view.q_min.text()
            self.active_bank.update_value(q_min=float(q_min))
            self.update_plots()
        except TypeError:
            self._view.q_min.setText(self.active_bank.q_min)

    def changed_q_max(self):
        try:
            q_max = self._view.q_max.text()
            self.active_bank.update_value(q_max=float(q_max))
            self.update_plots()
        except TypeError:
            self._view.q_max.setText(self.active_bank.q_max)

    def changed_detector_bank(self, i):
        if not self.bank_list[i] == self.active_bank:
            self.set_active_bank(i)

    def set_active_bank(self, i):
        self.active_bank = self.bank_list[i]
        self._view.alpha.setText(str(self.active_bank.get_alpha()))
        self._view.grad.setText(str(self.active_bank.get_grad()))
        self._view.intercept.setText(str(self.active_bank.get_intercept()))
        self._view.q_min.setText(str(self.active_bank.get_q_min()))
        self._view.q_max.setText(str(self.active_bank.get_q_max()))
        self.update_plots()

    def update_plots(self):
        q = self.active_bank.get_q()
        total_s_2 = self.active_bank.get_total_s_2()
        self_s_valid = self.active_bank.get_self_s()
        distinct_s = self.active_bank.get_distinct_s()
        merged = self.get_merged()
        q_min = self.active_bank.get_q_min()
        q_max = self.active_bank.get_q_max()

        valid_index = np.where([(q_min <= x <= q_max) for x in q])

        self._view.axis_1.clear()
        self._view.axis_1.plot(q[valid_index], total_s_2[valid_index], 'b')
        self._view.axis_1.plot(q[valid_index], self_s_valid[valid_index], 'r')
        self._view.canvas_1.draw()

        self._view.axis_2.clear()
        self._view.axis_2.plot(q[valid_index], distinct_s[valid_index], 'b')
        self._view.axis_2.plot(q[valid_index], merged[valid_index], 'r')
        self._view.canvas_2.draw()

        self._view.axis_3.clear()
        self._view.axis_3.plot(q, merged, 'r')
        self._view.canvas_3.draw()

    def get_merged(self):
        q = self.active_bank.get_q()
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

    def get_bank_list(self):
        # TODO replace dummy bank data with data loader

        DCS_read = csv.reader(open("C:\\Users\\wey38795\\Documents\\DCS.csv", "rb"), delimiter=",")
        DCS_list = list(DCS_read)
        DCS = np.array(DCS_list).astype("float")

        reader = csv.reader(open("C:\\Users\\wey38795\\Documents\\SLF.csv", "rb"), delimiter=",")
        x = list(reader)
        SLF = np.array(x).astype("float")

        bank_list = []
        q = DCS[:, 0]
        for i in np.arange(DCS[0, :].size - 1):
            dcs = DCS[:, i+1]
            slf = SLF[:, i+1]
            name = 'bank' + str(i)
            bank_list.append(model.BankDataSet(name, q, dcs, slf))
        return bank_list

    def output_wksp(self):
        CreateWorkspace(
            OutputWorkspace="foobar",
            DataX=self.active_bank.get_q(),
            DataY=self.get_merged())
