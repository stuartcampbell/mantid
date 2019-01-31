# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
import unittest

from MultiPlotting.subplot.subPlot import subPlot
from MultiPlotting.multiPlotting_context import PlottingContext

from Muon.GUI.Common import mock_widget
try:
    from unittest import mock
except ImportError:
    import mock


def rm_logic(name):
    if name == "two":
        return False
    return True


class SubplotTest(unittest.TestCase):

    def setUp(self):
        self._qapp = mock_widget.mockQapp()
        context = PlottingContext()
        self.subplot = subPlot(context)
        self.subplot.canvas.draw = mock.MagicMock()

    def setup_rm(self):
        self.subplot._raise_rm_window = mock.Mock()
        self.subplot._raise_selector_window = mock.Mock()
        self.subplot._get_rm_window = mock.Mock()
        self.subplot._createSelectWindow = mock.MagicMock()

    def test_rmOnePlotNewWindow(self):
        self.subplot._rm_window = None
        self.subplot._selector_window = None

        self.subplot._context.subplots["one"] = mock.Mock()
        self.setup_rm()
        self.subplot._rm()

        self.assertEquals(self.subplot._raise_rm_window.call_count, 0)
        self.assertEquals(self.subplot._raise_selector_window.call_count, 0)
        self.assertEquals(self.subplot._get_rm_window.call_count, 1)
        self.assertEquals(self.subplot._createSelectWindow.call_count, 0)

    def test_rmOnePlotOldWindow(self):
        self.subplot._rm_window = mock.Mock()
        self.subplot._selector_window = None

        self.subplot._context.subplots["one"] = mock.Mock()
        self.setup_rm()
        self.subplot._rm()

        self.assertEquals(self.subplot._raise_rm_window.call_count, 1)
        self.assertEquals(self.subplot._raise_selector_window.call_count, 0)
        self.assertEquals(self.subplot._get_rm_window.call_count, 0)
        self.assertEquals(self.subplot._createSelectWindow.call_count, 0)

    def test_rmTwoPlotsNewWindow(self):
        self.subplot._rm_window = None
        self.subplot._selector_window = None

        self.subplot._context.subplots["one"] = mock.Mock()
        self.subplot._context.subplots["two"] = mock.Mock()
        self.setup_rm()
        self.subplot._rm()

        self.assertEquals(self.subplot._raise_rm_window.call_count, 0)
        self.assertEquals(self.subplot._raise_selector_window.call_count, 0)
        self.assertEquals(self.subplot._get_rm_window.call_count, 0)
        self.assertEquals(self.subplot._createSelectWindow.call_count, 1)

    def test_rmTwoPlotsOldSelectWindow(self):
        self.subplot._rm_window = None
        self.subplot._selector_window = mock.Mock()

        self.subplot._context.subplots["one"] = mock.Mock()
        self.subplot._context.subplots["two"] = mock.Mock()
        self.setup_rm()
        self.subplot._rm()

        self.assertEquals(self.subplot._raise_rm_window.call_count, 0)
        self.assertEquals(self.subplot._raise_selector_window.call_count, 1)
        self.assertEquals(self.subplot._get_rm_window.call_count, 0)
        self.assertEquals(self.subplot._createSelectWindow.call_count, 0)

    def test_rmTwoPlotsoldRmWindow(self):
        self.subplot._rm_window = mock.Mock()
        self.subplot._selector_window = None

        self.subplot._context.subplots["one"] = mock.Mock()
        self.subplot._context.subplots["two"] = mock.Mock()
        self.setup_rm()
        self.subplot._rm()

        self.assertEquals(self.subplot._raise_rm_window.call_count, 1)
        self.assertEquals(self.subplot._raise_selector_window.call_count, 0)
        self.assertEquals(self.subplot._get_rm_window.call_count, 0)
        self.assertEquals(self.subplot._createSelectWindow.call_count, 0)

    def setup_applyRm(self):
        self.subplot._rm_window = mock.Mock()
        self.subplot._rm_window.subplot = "test"
        self.subplot._context.subplots["test"] = mock.MagicMock()
        self.subplot._remove_subplot = mock.Mock()
        self.subplot._close_rm_window = mock.Mock()

    def test_applyRmAll(self):
        names = ["one", "two", "three"]
        self.setup_applyRm()
        self.subplot._rm_window.getState = mock.Mock(return_value=True)

        self.subplot._applyRm(names)

        self.assertEquals(
            self.subplot._context.subplots[
                "test"].removeLine.call_count,
            3)
        self.assertEquals(self.subplot._close_rm_window.call_count, 1)

    def test_applyRmNone(self):
        names = ["one", "two", "three"]
        self.setup_applyRm()
        self.subplot._rm_window.getState = mock.Mock(return_value=False)

        self.subplot._applyRm(names)

        self.assertEquals(
            self.subplot._context.subplots[
                "test"].removeLine.call_count,
            0)
        self.assertEquals(self.subplot._close_rm_window.call_count, 1)

    def test_applyRmSome(self):
        names = ["one", "two", "three"]
        self.setup_applyRm()
        self.subplot._rm_window.getState = mock.Mock(side_effect=rm_logic)

        self.subplot._applyRm(names)

        self.assertEquals(
            self.subplot._context.subplots[
                "test"].removeLine.call_count,
            2)
        self.assertEquals(self.subplot._close_rm_window.call_count, 1)

if __name__ == "__main__":
    unittest.main()
