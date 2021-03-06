# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
from __future__ import (absolute_import, division, print_function)

import unittest

from sans.common.enums import (SANSInstrument, SANSFacility)
from sans.state.StateObjects.StateAdjustment import (StateAdjustment)
from sans.state.StateObjects.StateCalculateTransmission import (StateCalculateTransmission)
from sans.state.StateObjects.StateConvertToQ import (StateConvertToQ)
from sans.state.StateObjects.StateData import (StateData)
from sans.state.StateObjects.StateMaskDetectors import (StateMask)
from sans.state.StateObjects.StateMoveDetectors import (StateMove)
from sans.state.StateObjects.StateNormalizeToMonitor import (StateNormalizeToMonitor)
from sans.state.StateObjects.StateReductionMode import (StateReductionMode)
from sans.state.StateObjects.StateSave import (StateSave)
from sans.state.StateObjects.StateScale import (StateScale)
from sans.state.StateObjects.StateSliceEvent import (StateSliceEvent)
from sans.state.AllStates import (AllStates)
from sans.state.StateObjects.StateWavelength import (StateWavelength)
from sans.state.StateObjects.StateWavelengthAndPixelAdjustment import (StateWavelengthAndPixelAdjustment)


# ----------------------------------------------------------------------------------------------------------------------
#  State
# ----------------------------------------------------------------------------------------------------------------------
class MockStateData(StateData):
    def validate(self):
        pass


class MockStateMove(StateMove):
    def validate(self):
        pass


class MockStateReduction(StateReductionMode):
    def get_merge_strategy(self):
        pass

    def get_detector_name_for_reduction_mode(self, reduction_mode):
        pass

    def get_all_reduction_modes(self):
        pass

    def validate(self):
        pass


class MockStateSliceEvent(StateSliceEvent):
    def validate(self):
        pass


class MockStateMask(StateMask):
    def validate(self):
        pass


class MockStateWavelength(StateWavelength):
    def validate(self):
        pass


class MockStateSave(StateSave):
    def validate(self):
        pass


class MockStateNormalizeToMonitor(StateNormalizeToMonitor):
    def validate(self):
        pass


class MockStateScale(StateScale):
    def validate(self):
        pass


class MockStateCalculateTransmission(StateCalculateTransmission):
    def validate(self):
        pass


class MockStateWavelengthAndPixelAdjustment(StateWavelengthAndPixelAdjustment):
    def validate(self):
        pass


class MockStateAdjustment(StateAdjustment):
    def validate(self):
        pass


class MockStateConvertToQ(StateConvertToQ):
    def validate(self):
        pass


class StateTest(unittest.TestCase):
    @staticmethod
    def _get_state(entries):
        state = AllStates()
        default_entries = {"data": MockStateData(), "move": MockStateMove(), "reduction": MockStateReduction(),
                           "slice": MockStateSliceEvent(), "mask": MockStateMask(), "wavelength": MockStateWavelength(),
                           "save": MockStateSave(), "scale": MockStateScale(), "adjustment": MockStateAdjustment(),
                           "convert_to_q": MockStateConvertToQ()}
        default_entries["data"].instrument = SANSInstrument.LARMOR
        default_entries["data"].facility = SANSFacility.ISIS

        for key, value in list(default_entries.items()):
            if key in entries:
                value = entries[key]
            if value is not None:  # If the value is None, then don't set it
                setattr(state, key, value)
        return state

    def check_bad_and_good_values(self, bad_state, good_state):
        # Bad values
        state = self._get_state(bad_state)
        with self.assertRaises(ValueError):
            state.validate()

        # Good values
        state = self._get_state(good_state)
        self.assertIsNone(state.validate())

    def test_that_raises_when_move_has_not_been_set(self):
        self.check_bad_and_good_values({"move": None}, {"move": MockStateMove()})

    def test_that_raises_when_reduction_has_not_been_set(self):
        self.check_bad_and_good_values({"reduction": None}, {"reduction": MockStateReduction()})

    def test_that_raises_when_slice_has_not_been_set(self):
        self.check_bad_and_good_values({"slice": None}, {"slice": MockStateSliceEvent()})

    def test_that_raises_when_mask_has_not_been_set(self):
        self.check_bad_and_good_values({"mask": None}, {"mask": MockStateMask()})

    def test_that_raises_when_wavelength_has_not_been_set(self):
        self.check_bad_and_good_values({"wavelength": None}, {"wavelength": MockStateWavelength()})

    def test_that_raises_when_save_has_not_been_set(self):
        self.check_bad_and_good_values({"save": None}, {"save": MockStateSave()})

    def test_that_raises_when_scale_has_not_been_set(self):
        self.check_bad_and_good_values({"scale": None}, {"scale": MockStateScale()})

    def test_that_raises_when_adjustment_has_not_been_set(self):
        self.check_bad_and_good_values({"adjustment": None}, {"adjustment": MockStateAdjustment()})

    def test_that_raises_when_convert_to_q_has_not_been_set(self):
        self.check_bad_and_good_values({"convert_to_q": None}, {"convert_to_q": MockStateConvertToQ()})


if __name__ == '__main__':
    unittest.main()
