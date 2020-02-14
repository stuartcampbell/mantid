# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2020 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
import six


class TomlSchemaV1Validator(object):
    # As of the current TOML release there is no way to validate a schema so
    # we must provide an implementation

    # Note : To future devs, if we have a V2 schema a lot of this class could
    # be split into a SchemaValidator and an inheriting V1 and V2 schema which
    # would override the reference schema with the new one

    def __init__(self, dict_to_validate):
        self._expected_list = self._build_nested_keys(self._reference_schema())
        self._to_validate_list = self._build_nested_keys(dict_to_validate)

    def validate(self):
        unrecognised = []
        for k in self._to_validate_list:
            if k not in self._expected_list:
                unrecognised.append(k)

        if len(unrecognised) > 0:
            err = "The following keys were not recognised\n:"
            err += "".join("{0}\n".format(k) for k in unrecognised)
            raise KeyError(err)

    @staticmethod
    def _reference_schema():
        """
        Returns a dictionary layout of all supported keys
        :return: Dictionary containing all keys, and values set to None
        """
        instrument_keys = {"name": None,
                           "configuration": {"collimation_length",
                                             "gravity_extra_length",
                                             "norm_monitor",
                                             "sample_aperture_diameter",
                                             "sample_offset",
                                             "trans_monitor"}}
        detector_keys = {"configuration": {"selected_detector": None,
                                           "rear_scale": None,
                                           "front_centre": {"x", "y", "z"},
                                           "rear_centre": {"x", "y", "z"}}}

        binning_keys = {"wavelength": {"start", "step", "stop", "type"},
                        "1d_reduction": {"binning"},
                        "2d_reduction": {"step", "stop", "type"}}

        return {"instrument": instrument_keys,
                "detector": detector_keys,
                "binning": binning_keys}

    @staticmethod
    def _build_nested_keys(d, path="", current_out=None):
        if not current_out:
            current_out = []

        def make_path(current_path, new_key):
            return current_path + "." + new_key if current_path else new_key

        for key, v in six.iteritems(d):
            new_path = make_path(path, key)
            if isinstance(v, dict):
                # Recurse into dict
                current_out = TomlSchemaV1Validator._build_nested_keys(v, new_path, current_out)
            elif isinstance(v, set):
                # Pack all in from the set of names
                for name in v:
                    current_out.append(make_path(new_path, name))
            else:
                # This means its a value type with nothing special, so keep name
                current_out.append(new_path)

        return current_out
