# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
from __future__ import (absolute_import, division, print_function)
import numpy as np
from my_widgets import model


class Presenter(object):

    def __init__(self, view):
        self.active_bank = None
        self.bank_list = None

        self._view = view
        self.bank_list = self.get_bank_list()
        self.set_active_bank(1)

    def connect_view_signals(self):
        self._view.alpha.textEdited.connect(self.changed_alpha)
        self._view.grad.textEdited.connect(self.changed_grad)
        self._view.intercept.textEdited.connect(self.changed_intercept)
        self._view.q_min.textEdited.connect(self.changed_q_min)
        self._view.q_max.textEdited.connect(self.changed_q_max)
        self._view.detector_bank.currentIndexChanged.connect(self.changed_detector_bank)

    def alpha_changed(self, alpha):
        try:
            self.active_bank.update_value(alpha=float(alpha))
            self.update_plots()
        except ValueError:
            self._view.alpha.setText(self.active_bank.alpha)

    def changed_grad(self, grad):
        try:
            self.active_bank.update_value(grad=float(grad))
            self.update_plots()
        except ValueError:
            self._view.grad.setText(self.active_bank.grad)

    def changed_intercept(self, intercept):
        try:
            self.active_bank.update_value(intercept=float(intercept))
            self.update_plots()
        except ValueError:
            self._view.intercept.setText(self.active_bank.intercept)

    def changed_q_min(self, q_min):
        try:
            self.active_bank.update_value(q_min=float(q_min))
            self.update_plots()
        except ValueError:
            self._view.q_min.setText(self.active_bank.q_min)

    def changed_q_max(self, q_max):
        try:
            self.active_bank.update_value(q_max=float(q_max))
            self.update_plots()
        except ValueError:
            self._view.q_max.setText(self.active_bank.q_max)

    def changed_detector_bank(self, i):
        if not self.bank_list[i] == self.active_bank:
            self.set_active_bank(i)

    def set_active_bank(self, i):
        self.active_bank = self.bank_list[i]
        self._view.alpha.setText(self.active_bank.alpha)
        self._view.grad.setText(self.active_bank.grad)
        self._view.intercept.setText(self.active_bank.intercept)
        self._view.q_min.setText(self.active_bank.q_min)
        self._view.q_max.setText(self.active_bank.q_max)
        self.update_plots()

    def update_plots(self):
        ax = self._view.figure_1.add_subplot(111)
        ax.plot(self.active_bank.plot_dcs_2())
        self._view.canvas_1.draw()
        pass

    def get_bank_list(self):
        # TODO replace dummy bank data with data loader
        bank_list = []
        q = np.linspace(0.0, 60.0, 5000)
        slf = np.exp(-q)
        dcs = 100 * np.random.rand(*q.shape) + 2000
        bank_list.append(model.BankDataSet('bank 1', q, dcs, slf))
        dcs = 100 * np.random.rand(q.shape) + 2000
        bank_list.append(model.BankDataSet('bank 2', q, dcs, slf))
        dcs = 100 * np.random.rand(q.shape) + 2000
        bank_list.append(model.BankDataSet('bank 3', q, dcs, slf))
        return bank_list