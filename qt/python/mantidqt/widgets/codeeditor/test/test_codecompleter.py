# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
#    This file is part of the mantid workbench.
#
#
import re
import unittest

from mantid.py3compat.mock import Mock, patch
from mantidqt.widgets.codeeditor.codecompleter import CodeCompleter


class CodeCompleterTest(unittest.TestCase):

    def setUp(self):
        self.editor_mock = Mock(fileName=lambda: "")
        self.completer = CodeCompleter(self.editor_mock, {})

    def _set_editor_text(self, text):
        self.editor_mock.text.return_value = text

    def test_completion_api_is_updated_with_numpy_completions_when_cursor_position_changed(self):
        self._set_editor_text("import numpy as np\nnp.a")
        self.completer._on_cursor_position_changed(1, 3)
        self.assertIn('abs', self.editor_mock.addToCompletionAPI.call_args[0][0])

    def test_completion_api_is_updated_with_mantid_api_when_cursor_position_changed(self):
        self._set_editor_text("from mantid.simpleapi import *\nLo")
        self.completer._on_cursor_position_changed(1, 2)
        self.assertIn('Load', self.editor_mock.addToCompletionAPI.call_args[0][0])

    def test_call_tips_are_generated_with_the_correct_args_for_Load_function(self):
        self._set_editor_text("from mantid.simpleapi import *\nLoad(")
        call_tips = self.completer._generate_call_tips(2, 5)
        self.assertIn("Load(*args, **kwargs)", call_tips)

    def test_call_tips_are_still_visible_after_argument_inserted(self):
        self._set_editor_text("import numpy as np\nnp.array(x, ")
        joined_call_tips = ' '.join(self.completer._generate_call_tips(2, 12))
        self.assertTrue(bool(re.search("array\(object, .*\)", joined_call_tips)))

    def test_call_tips_are_not_generated_when_outside_a_function_call(self):
        self._set_editor_text("import numpy as np\nnp.array(x)")
        self.assertEqual(0, len(self.completer._generate_call_tips(2, 11)))

    def test_that_generate_completions_list_is_not_called_when_cursor_is_inside_bracket(self):
        self._set_editor_text("import numpy as np\nnp.array(    ")
        with patch.object(self.completer, '_generate_completions_list'):
            self.completer._on_cursor_position_changed(1, 11)
            self.assertEqual(0, self.completer._generate_completions_list.call_count)

    def test_that_generate_completions_list_is_called_when_cursor_after_closed_brackets(self):
        self._set_editor_text("import numpy as np\nnp.array(x)    ")
        with patch.object(self.completer, '_generate_completions_list'):
            self.completer._on_cursor_position_changed(1, 11)
            self.assertEqual(1, self.completer._generate_completions_list.call_count)


if __name__ == '__main__':
    unittest.main()
