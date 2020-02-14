# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2020 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
import unittest

from mantid.py3compat import mock
from sans.common.enums import SANSInstrument, SANSFacility, DetectorType, ReductionMode, RangeStepType
from sans.state.StateObjects.StateData import get_data_builder
from sans.test_helper.file_information_mock import SANSFileInformationMock
from sans.user_file.toml_parsers.toml_v1_parser import TomlV1Parser


class TomlV1ParserTest(unittest.TestCase):
    @staticmethod
    def _get_mock_data_info():
        # TODO I really really dislike having to do this in a test, but
        # TODO de-coupling StateData is required to avoid it
        file_information = SANSFileInformationMock(instrument=SANSInstrument.SANS2D, run_number=22024)
        data_builder = get_data_builder(SANSFacility.ISIS, file_information)
        data_builder.set_sample_scatter("SANS2D00022024")
        data_builder.set_sample_scatter_period(3)
        return data_builder.build()

    def _setup_parser(self, dict_vals):
        self._mocked_data_info = self._get_mock_data_info()
        if "instrument" not in dict_vals:
            # instrument key needs to generally be present
            dict_vals["instrument"] = {}
        if "name" not in dict_vals["instrument"]:
            dict_vals["instrument"]["name"] = "LOQ"

        return TomlV1Parser(dict_vals, data_info=self._mocked_data_info)

    def test_instrument(self):
        parser = self._setup_parser(dict_vals={"instrument": {"name": SANSInstrument.SANS2D.value}})
        inst = parser._implementation.instrument
        self.assertTrue(inst is SANSInstrument.SANS2D, msg="Got %r instead" % inst)

    def test_validate_is_called_on_init(self):
        schema_validator = mock.Mock()

        # No implementation needed
        with mock.patch("sans.user_file.toml_parsers.toml_v1_parser.TomlV1Parser._get_impl"):
            TomlV1Parser(dict_to_parse=None, data_info=mock.NonCallableMock(), schema_validator=schema_validator)
            self.assertTrue(schema_validator.validate.called)

    def _loop_over_supported_keys(self, supported_keys, top_level_keys):
        top_level_dict = {}
        dict_view = top_level_dict

        # We need to append a mock object using the bottom key in the below loop
        for key in top_level_keys[:-1]:
            dict_view[key] = {}
            dict_view = dict_view[key]

        for k, func in supported_keys:
            expected = mock.NonCallableMock()
            dict_view[top_level_keys[-1]] = {k: expected}
            parser = self._setup_parser(top_level_dict)

            val = func(parser)
            self.assertEqual(expected, val, "Failed to get key {0}".format(k))

    def test_instrument_configuration_parsed(self):
        supported_keys = [
            ("collimation_length", lambda x: x.get_state_convert_to_q().q_resolution_collimation_length),
            ("gravity_extra_length", lambda x: x.get_state_convert_to_q().gravity_extra_length),
            ("norm_monitor", lambda x: x.get_state_calculate_transmission().incident_monitor),
            ("trans_monitor", lambda x: x.get_state_calculate_transmission().transmission_monitor),
            ("sample_aperture_diameter", lambda x: x.get_state_convert_to_q().q_resolution_a2),
            ("sample_offset", lambda x: x.get_state_move_detectors().sample_offset)
        ]

        self._loop_over_supported_keys(supported_keys=supported_keys, top_level_keys=["instrument", "configuration"])

    def test_detector_configuration_parsed(self):
        supported_keys = [
            ("rear_scale", lambda x: x.get_state_scale().scale),
            # ("front_scale", lambda x: x.get_state_scale()) TODO this is issue # 27948
        ]
        self._loop_over_supported_keys(supported_keys=supported_keys, top_level_keys=["detector", "configuration"])

        # Manually do more interesting keys
        def get_bank_translation(parser, det_type):
            bank_values = parser.get_state_move_detectors().detectors[det_type.value]
            x = bank_values.x_translation_correction
            y = bank_values.y_translation_correction
            z = bank_values.z_translation_correction
            return x, y, z

        top_level_dict = {"detector": {"configuration": {}}}
        config_dict = top_level_dict["detector"]["configuration"]

        expected_reduction_mode = ReductionMode.ALL
        config_dict["selected_detector"] = expected_reduction_mode.value
        config_dict["front_centre"] = {"x": 1, "y": 2, "z": 3}
        config_dict["rear_centre"] = {"x": 2, "y": 3, "z": 4}

        parser = self._setup_parser(dict_vals=top_level_dict)
        self.assertTrue(parser.get_state_reduction_mode().reduction_mode is expected_reduction_mode)
        self.assertEqual((1, 2, 3), get_bank_translation(parser, DetectorType.HAB))
        self.assertEqual((2, 3, 4), get_bank_translation(parser, DetectorType.LAB))

    def test_binning_commands_parsed(self):
        # Wavelength
        for bin_type in ["Lin", "Log"]:
            wavelength_dict = {"binning": {"wavelength": {"start": 1.1, "step": 0.1, "stop": 2.2, "type": bin_type}}}
            wavelength = self._setup_parser(wavelength_dict).get_state_wavelength()
            self.assertEqual(1.1, wavelength.wavelength_low)
            self.assertEqual(0.1, wavelength.wavelength_step)
            self.assertEqual(2.2, wavelength.wavelength_high)
            self.assertEqual(RangeStepType(bin_type), wavelength.wavelength_step_type)

        one_d_reduction_q_dict = {"binning": {"1d_reduction": {"binning": "1.0, 0.1, 2.0, -0.2, 3.0"}}}
        one_d_convert_to_q = self._setup_parser(one_d_reduction_q_dict).get_state_convert_to_q()
        self.assertEqual(1.0, one_d_convert_to_q.q_min)
        self.assertEqual(3.0, one_d_convert_to_q.q_max)
        self.assertEqual("0.1, 2.0, -0.2", one_d_convert_to_q.q_1d_rebin_string.strip())

        two_d_reduction_q_dict = {"binning": {"2d_reduction": {"step": 1.0, "stop": 5.0, "type": "Lin"}}}
        two_d_convert_to_q = self._setup_parser(two_d_reduction_q_dict).get_state_convert_to_q()
        self.assertEqual(5.0, two_d_convert_to_q.q_xy_max)
        self.assertEqual(1.0, two_d_convert_to_q.q_xy_step)
        self.assertTrue(two_d_convert_to_q.q_xy_step_type is RangeStepType.LIN)


if __name__ == '__main__':
    unittest.main()
