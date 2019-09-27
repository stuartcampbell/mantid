# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
from __future__ import (absolute_import, division, print_function)
from mantid.simpleapi import CreateWorkspace
from MergeDetectorBanks import model
import numpy as np


class Presenter(object):

    def __init__(self, view):
        self.active_bank = None
        self.bank_list = None

        self._view = view
        self.bank_collection = model.BankDetectorCollection()
        self.bank_list = self.bank_collection.get_bank_list()
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
        self._view.save_correction_files_btn.clicked.connect(self.save_correction)

    def changed_alpha(self):
        try:
            alpha = self._view.alpha.text()
            if not alpha == self.active_bank.get_alpha():
                self.active_bank.update_value(alpha=float(alpha))
                self.update_plots()
        except TypeError:
            self._view.alpha.setText(self.active_bank.get_alpha())

    def changed_grad(self):
        try:
            grad = self._view.grad.text()
            if not grad == self.active_bank.get_grad():
                self.active_bank.update_value(grad=float(grad))
                self.update_plots()
        except TypeError:
            self._view.grad.setText(self.active_bank.grad)

    def changed_intercept(self):
        try:
            intercept = self._view.intercept.text()
            if not intercept == self.active_bank.get_intercept():
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
        merged = self.bank_collection.get_merged()
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

    def output_wksp(self):
        CreateWorkspace(
            OutputWorkspace="foobar",
            DataX=self.active_bank.get_q(),
            DataY=self.bank_collection.get_merged())

    def save_correction(self):
        alf = self.bank_collection.get_alpha_table()
        lim = self.bank_collection.get_limit_table()
        lin = self.bank_collection.get_linear_table()
        print(alf)
        print(lim)
        print(lin)