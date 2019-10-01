# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
from __future__ import (absolute_import, division, print_function)
import csv
from MergeDetectorBanks import model
import numpy as np
from qtpy import QtWidgets, QtGui

from mantid.simpleapi import CreateWorkspace


class Presenter(object):

    def __init__(self, view):
        self.active_bank = None
        self.bank_list = None
        self.bank_collection = None

        self._view = view
        self.connect_view_signals()

    def connect_view_signals(self):
        self._view.alpha.editingFinished.connect(self.changed_alpha)
        self._view.grad.editingFinished.connect(self.changed_grad)
        self._view.intercept.editingFinished.connect(self.changed_intercept)
        self._view.q_min.editingFinished.connect(self.changed_q_min)
        self._view.q_max.editingFinished.connect(self.changed_q_max)
        self._view.detector_bank.currentIndexChanged.connect(self.changed_detector_bank)
        self._view.load_btn.clicked.connect(self.load_data)
        self._view.output_merged_workspace_btn.clicked.connect(self.output_wksp)
        self._view.save_correction_files_btn.clicked.connect(self.save_correction)

    def changed_alpha(self):
        try:
            alpha = self._view.alpha.text()
            if not alpha == self.active_bank.get_alpha():
                self.active_bank.update_value(alpha=float(alpha))
                self.update_plots()
        except ValueError:
            self._view.alpha.setText(self.active_bank.get_alpha())

    def changed_grad(self):
        try:
            grad = self._view.grad.text()
            if not grad == self.active_bank.get_grad():
                self.active_bank.update_value(grad=float(grad))
                self.update_plots()
        except ValueError:
            self._view.grad.setText(self.active_bank.get_grad())

    def changed_intercept(self):
        try:
            intercept = self._view.intercept.text()
            if not intercept == self.active_bank.get_intercept():
                self.active_bank.update_value(intercept=float(intercept))
                self.update_plots()
        except ValueError:
            self._view.intercept.setText(self.active_bank.get_intercept())

    def changed_q_min(self):
        try:
            q_min = self._view.q_min.text()
            if not q_min == self.active_bank.get_q_min():
                self.active_bank.update_value(q_min=float(q_min))
                self.update_plots()
        except ValueError:
            self._view.q_min.setText(self.active_bank.get_q_min())

    def changed_q_max(self):
        try:
            q_max = self._view.q_max.text()
            if not q_max == self.active_bank.get_q_max():
                self.active_bank.update_value(q_max=float(q_max))
                self.update_plots()
        except ValueError:
            self._view.q_max.setText(self.active_bank.get_q_max())

    def changed_detector_bank(self, i):
        if not self.bank_list[i] == self.active_bank:
            self.set_active_bank(i)

    def load_data(self):
        # TODO replace with suitable loader for other format
        try:
            dcs_read = csv.reader(open(str(self._view.dcs_directory.text()), 'rb'), delimiter=',')
            dcs_all = np.array(list(dcs_read)).astype('float')
        except IOError:
            self.load_error(str(self._view.dcs_directory.text()))
            return
        try:
            slf_read = csv.reader(open(str(self._view.slf_directory.text()), 'rb'), delimiter=',')
            slf_all = np.array(list(slf_read)).astype('float')
        except IOError:
            self.load_error(str(self._view.slf_directory.text()))
            return

        self.bank_collection = model.BankDetectorCollection(dcs_all, slf_all)
        self.bank_list = self.bank_collection.get_bank_list()
        for bank in self.bank_list:
            self._view.detector_bank.addItem(bank.name)
        self.set_active_bank(0)
        self.enable_functions()

    def enable_functions(self):
        self._view.detector_bank.setEnabled(True)
        self._view.alpha.setEnabled(True)
        self._view.grad.setEnabled(True)
        self._view.intercept.setEnabled(True)
        self._view.q_min.setEnabled(True)
        self._view.q_max.setEnabled(True)
        self._view.output_merged_workspace_btn.setEnabled(True)
        self._view.save_correction_files_btn.setEnabled(True)
        # set output name to be the filename on the DCS file as default
        run_name = str(self._view.dcs_directory.text()).split('\\')[-1].split('.')[0]
        self._view.output_name.setText(run_name)

    @staticmethod
    def load_error(directory):
        message_box = QtWidgets.QMessageBox()
        message_box.setText('IOError: Cannot load ' + directory)
        message_box.setWindowTitle('IOError')
        message_box.setStandardButtons(QtWidgets.QMessageBox.Ok)
        message_box.exec_()

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
        wksp_name = str(self._view.output_name.text()) + '_merged'
        CreateWorkspace(
            OutputWorkspace=wksp_name,
            DataX=self.active_bank.get_q(),
            DataY=self.bank_collection.get_merged())

    def save_correction(self):
        alf = self.bank_collection.get_alpha_table()
        lim = self.bank_collection.get_limit_table()
        lin = self.bank_collection.get_linear_table()
        # save the files to the same directory as the DCS file
        directory = ''
        for i in str(self._view.dcs_directory.text()).split('\\')[:-1]:
            directory += i + '\\'
        filename = str(self._view.output_name.text())
        self.save_table(alf, directory, filename, '.alf')
        self.save_table(lim, directory, filename, '.lim')
        self.save_table(lin, directory, filename, '.lin')

    @staticmethod
    def save_table(table, directory, filename, extension):
        with open(directory + filename + extension, 'w') as f:
            file_string = ''
            for line in table:
                if type(line) == list:
                    for item in line:
                        file_string += (' %s            ' % item)
                    file_string += '\n'
                else:
                    file_string += (' %s \n' % line)
            f.write(file_string)
