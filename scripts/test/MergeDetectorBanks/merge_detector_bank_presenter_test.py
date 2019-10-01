# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
from __future__ import (absolute_import, unicode_literals)

import numpy as np
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

    def test_changed_alpha_handles_invalid_input(self):
        self.gui.active_bank = mock.Mock()
        self.gui.active_bank.get_alpha.return_value = 5
        self.gui._view.alpha.text.return_value = 'string'
        self.gui._view.alpha.setText = mock.Mock()
        self.gui.update_plots = mock.Mock()
        self.gui.update_plots.side_effect = ValueError

        self.gui.changed_alpha()

        self.gui._view.alpha.setText.assert_called_once_with(5)

    def test_changed_grad_does_not_replot_if_value_is_same_as_bank_data(self):
        self.gui.active_bank = mock.Mock()
        self.gui.active_bank.get_grad.return_value = 5
        self.gui._view.grad.text.return_value = 5
        self.gui.update_plots = mock.Mock()

        self.gui.changed_grad()

        self.assertFalse(self.gui.update_plots.called)

    def test_changed_grad_handles_invalid_input(self):
        self.gui.active_bank = mock.Mock()
        self.gui.active_bank.get_grad.return_value = 5
        self.gui._view.grad.text.return_value = 'string'
        self.gui._view.grad.setText = mock.Mock()
        self.gui.update_plots = mock.Mock()
        self.gui.update_plots.side_effect = ValueError

        self.gui.changed_grad()

        self.gui._view.grad.setText.assert_called_once_with(5)

    def test_changed_intercept_does_not_replot_if_value_is_same_as_bank_data(self):
        self.gui.active_bank = mock.Mock()
        self.gui.active_bank.get_intercept.return_value = 5
        self.gui._view.intercept.text.return_value = 5
        self.gui.update_plots = mock.Mock()

        self.gui.changed_intercept()

        self.assertFalse(self.gui.update_plots.called)

    def test_changed_intercept_handles_invalid_input(self):
        self.gui.active_bank = mock.Mock()
        self.gui.active_bank.get_intercept.return_value = 5
        self.gui._view.intercept.text.return_value = 'string'
        self.gui._view.intercept.setText = mock.Mock()
        self.gui.update_plots = mock.Mock()
        self.gui.update_plots.side_effect = ValueError

        self.gui.changed_intercept()

        self.gui._view.intercept.setText.assert_called_once_with(5)

    def test_changed_q_min_does_not_replot_if_value_is_same_as_bank_data(self):
        self.gui.active_bank = mock.Mock()
        self.gui.active_bank.get_q_min.return_value = 5
        self.gui._view.q_min.text.return_value = 5
        self.gui.update_plots = mock.Mock()

        self.gui.changed_q_min()

        self.assertFalse(self.gui.update_plots.called)

    def test_changed_q_min_handles_invalid_input(self):
        self.gui.active_bank = mock.Mock()
        self.gui.active_bank.get_q_min.return_value = 5
        self.gui._view.q_min.text.return_value = 'string'
        self.gui._view.q_min.setText = mock.Mock()
        self.gui.update_plots = mock.Mock()
        self.gui.update_plots.side_effect = ValueError

        self.gui.changed_q_min()

        self.gui._view.q_min.setText.assert_called_once_with(5)

    def test_changed_q_max_does_not_replot_if_value_is_same_as_bank_data(self):
        self.gui.active_bank = mock.Mock()
        self.gui.active_bank.get_q_max.return_value = 5
        self.gui._view.q_max.text.return_value = 5
        self.gui.update_plots = mock.Mock()

        self.gui.changed_q_max()

        self.assertFalse(self.gui.update_plots.called)

    def test_changed_q_max_handles_invalid_input(self):
        self.gui.active_bank = mock.Mock()
        self.gui.active_bank.get_q_max.return_value = 5
        self.gui._view.q_max.text.return_value = 'string'
        self.gui._view.q_max.setText = mock.Mock()
        self.gui.update_plots = mock.Mock()
        self.gui.update_plots.side_effect = ValueError

        self.gui.changed_q_max()

        self.gui._view.q_max.setText.assert_called_once_with(5)

    def test_changed_detector_bank_changes_bank_if_new_bank_is_different(self):
        self.gui.active_bank = mock.Mock()
        self.gui.bank_list = [self.gui.active_bank, mock.Mock(), mock.Mock()]
        self.gui.set_active_bank = mock.Mock()

        self.gui.changed_detector_bank(1)

        self.assertTrue(self.gui.set_active_bank.called)

    def test_changed_detector_bank_doesnt_change_bank_if_new_bank_is_same(self):
        self.gui.active_bank = mock.Mock()
        self.gui.bank_list = [self.gui.active_bank, mock.Mock(), mock.Mock()]
        self.gui.set_active_bank = mock.Mock()

        self.gui.changed_detector_bank(0)

        self.assertFalse(self.gui.set_active_bank.called)

    # TODO write tests for loader once finished

    def test_enable_functions_set_output_name(self):
        self.gui._view.dcs_directory.text.return_value = 'dir_1\\dir_2\\dir_3\\filename.dcs'
        self.gui._view.output_name.setText('filename')

    def test_set_active_bank_sets_all_values_and_updates_plots(self):
        self.gui.bank_list = [mock.Mock(), mock.Mock(), mock.Mock()]
        self.gui.bank_list[0].get_alpha.return_value = 1
        self.gui.bank_list[0].get_grad.return_value = 2
        self.gui.bank_list[0].get_intercept.return_value = 3
        self.gui.bank_list[0].get_q_min.return_value = 4
        self.gui.bank_list[0].get_q_max.return_value = 5
        self.gui.update_plots = mock.Mock()

        self.gui.set_active_bank(0)

        self.gui._view.alpha.setText.assert_called_once_with('1')
        self.gui._view.grad.setText.assert_called_once_with('2')
        self.gui._view.intercept.setText.assert_called_once_with('3')
        self.gui._view.q_min.setText.assert_called_once_with('4')
        self.gui._view.q_max.setText.assert_called_once_with('5')
        self.assertTrue(self.gui.update_plots.called)

    def test_update_plots(self):
        self.gui.active_bank = mock.Mock()
        self.gui.active_bank.get_q.return_value = np.arange(60)
        self.gui.active_bank.get_total_s_2.return_value = np.arange(60)
        self.gui.active_bank.get_self_s.return_value = np.arange(60)
        self.gui.active_bank.get_distinct_s.return_value = np.arange(60)
        self.gui.active_bank.get_q_min.return_value = 15
        self.gui.active_bank.get_q_max.return_value = 44
        self.gui.bank_collection = mock.Mock()
        self.gui.bank_collection.get_merged.return_value = np.arange(60)
        plot_values = np.arange(start=15, stop=45)

        self.gui.update_plots()

        outputs = [self.gui._view.axis_1.plot.call_args_list, self.gui._view.axis_2.plot.call_args_list]
        for output in outputs:
            np.testing.assert_almost_equal(output[0][0][:2], [plot_values, plot_values])
            self.assertTrue(output[0][0][2], 'b')
            np.testing.assert_almost_equal(output[1][0][:2], [plot_values, plot_values])
            self.assertTrue(output[1][0][2], 'r')
        output = self.gui._view.axis_3.plot.call_args_list
        np.testing.assert_almost_equal(output[0][0][:2], [np.arange(60), np.arange(60)])
        self.assertTrue(output[0][0][2], 'r')

    @mock.patch('MergeDetectorBanks.presenter.CreateWorkspace')
    def test_output_wksp_runs_CreateWorkspace(self, mock_create_workspace):
        self.gui._view.output_name.text.return_value = 'output_name'
        self.gui.active_bank = mock.Mock()
        self.gui.active_bank.get_q.return_value = np.arange(20)
        self.gui.bank_collection = mock.Mock()
        self.gui.bank_collection.get_merged.return_value = np.arange(20)
        call_args = mock.call(OutputWorkspace='output_name',
                              DataX=np.arange(20),
                              DataY=np.arange(20))
        mock_create_workspace.called_with(call_args)

    def test_save_correction_runs_save_table_for_each_file(self):
        self.gui.bank_collection = mock.Mock()
        alf = [3, [1, 1], [2, 1], [3, 1]]
        lim = [3, [1, 1, 0.1, 60], [2, 1, 0.1, 60], [3, 1, 0.1, 60]]
        lin = [3, [1, 1, 1, 5], [2, 1, 1, 5], [3, 1, 1, 5]]
        dcs_string = 'dir_1\\dir_2\\dir_3\\filename.dcs'
        dir_string = 'dir_1\\dir_2\\dir_3\\'
        output_name = 'output_name'
        self.gui.bank_collection.get_alpha_table.return_value = alf
        self.gui.bank_collection.get_limit_table.return_value = lim
        self.gui.bank_collection.get_linear_table.return_value = lin
        self.gui._view.dcs_directory.text.return_value = dcs_string
        self.gui._view.output_name.text.return_value = output_name
        self.gui.save_table = mock.Mock()
        call_list = [mock.call(alf, dir_string, output_name, '.alf'),
                     mock.call(lim, dir_string, output_name, '.lim'),
                     mock.call(lin, dir_string, output_name, '.lin')]

        self.gui.save_correction()

        self.gui.save_table.assert_has_calls(call_list)

    def test_save_table_saves_file_to_correct_dir(self):
        with mock.patch('MergeDetectorBanks.presenter.open', create=True) as mock_open:
            mock_file = mock.MagicMock(spec=file)
            mock_open.return_value = mock_file
            alf = [3, [1, 1], [2, 1], [3, 1]]
            dir_string = 'dir_1\\dir_2\\dir_3\\'
            output_name = 'output_name'

            self.gui.save_table(alf, dir_string, output_name, '.alf')

            mock_open.assert_has_calls(mock.call('dir_1\\dir_2\\dir_3\\output_name.alf', 'w'))


if __name__ == '__main__':
    unittest.main(buffer=False, verbosity=2)
