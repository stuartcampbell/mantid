# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2017 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
#    This file is part of the mantid workbench.
#
#
from __future__ import (absolute_import, unicode_literals)

# system imports
import re
import unittest

# local imports
from mantid.py3compat.mock import patch
from mantidqt.widgets.codeeditor.editor import CodeEditor
from mantidqt.utils.qt.testing import start_qapplication

TEST_LANG = "Python"


@start_qapplication
class CodeEditorTest(unittest.TestCase):

    # ---------------------------------------------------------------
    # Success tests
    # ---------------------------------------------------------------
    def test_construction_accepts_Python_as_language(self):
        CodeEditor(TEST_LANG)

    def test_default_construction_yields_empty_filename(self):
        widget = CodeEditor(TEST_LANG)
        self.assertEqual("", widget.fileName())

    def test_set_filename_returns_expected_string(self):
        widget = CodeEditor(TEST_LANG)
        test_filename = "myscript.py"
        widget.setFileName(test_filename)
        self.assertEqual(test_filename, widget.fileName())

    def test_set_text_can_be_read_again(self):
        widget = CodeEditor(TEST_LANG)
        code_str = 'print "Hello World!"'
        widget.setText(code_str)
        self.assertEqual(code_str, widget.text())

    def test_default_construction_yields_editable_widget(self):
        widget = CodeEditor(TEST_LANG)
        self.assertFalse(widget.isReadOnly())

    def test_setReadOnly_to_true_sets_readonly_status(self):
        widget = CodeEditor(TEST_LANG)
        widget.setReadOnly(True)
        self.assertTrue(widget.isReadOnly())

    def test_get_selection_for_empty_selection(self):
        widget = CodeEditor(TEST_LANG)
        res = widget.getSelection()
        self.assertEqual((-1, -1, -1, -1), res)

    def test_get_selection_for_non_empty_selection(self):
        widget = CodeEditor(TEST_LANG)
        widget.setText("""first line
        second line
        third line
        fourth line
        """)
        selected = (0, 2, 3, 4)
        widget.setSelection(*selected)
        res = widget.getSelection()
        self.assertEqual(selected, res)

    def test_completion_api_is_updated_with_numpy_completions_when_cursor_position_changed(self):
        widget = CodeEditor(TEST_LANG)
        widget.setText("import numpy as np\nnp.")
        with patch.object(widget, 'addToCompletionAPI'):
            widget._on_cursor_position_changed(1, 3)
            self.assertIn('abs', widget.addToCompletionAPI.call_args[0][0])

    def test_completion_api_is_updated_with_mantid_api_when_cursor_position_changed(self):
        widget = CodeEditor(TEST_LANG)
        widget.setText("from mantid.simpleapi import *\nLo")
        with patch.object(widget, 'addToCompletionAPI'):
            widget._on_cursor_position_changed(1, 2)
            self.assertIn('Load', widget.addToCompletionAPI.call_args[0][0])

    def test_call_tips_are_generated_with_the_correct_args_for_Load_function(self):
        widget = CodeEditor(TEST_LANG)
        widget.setText("from mantid.simpleapi import *\nLoad(")
        call_tips = widget._generate_call_tips(2, 5)
        self.assertIn("Load(*args, **kwargs)", call_tips)

    def test_call_tips_are_still_visible_after_argument_inserted(self):
        widget = CodeEditor(TEST_LANG)
        widget.setText("import numpy as np\nnp.array(x, ")
        joined_call_tips = ' '.join(widget._generate_call_tips(2, 12))
        self.assertTrue(bool(re.search("array\(object, .*\)", joined_call_tips)))

    def test_call_tips_are_not_generated_when_outside_a_function_call(self):
        widget = CodeEditor(TEST_LANG)
        widget.setText("import numpy as np\nnp.array(x)")
        self.assertEqual(0, len(widget._generate_call_tips(2, 11)))

    def test_that_generate_completions_list_is_not_called_when_cursor_is_inside_bracket(self):
        widget = CodeEditor(TEST_LANG)
        widget.setText("import numpy as np\nnp.array(    ")
        with patch.object(widget, '_generate_completions_list'):
            widget._on_cursor_position_changed(1, 11)
            self.assertEqual(0, widget._generate_completions_list.call_count)

    def test_that_generate_completions_list_is_called_after_closed_brackets(self):
        widget = CodeEditor(TEST_LANG)
        widget.setText("import numpy as np\nnp.array(x)    ")
        with patch.object(widget, '_generate_completions_list'):
            widget._on_cursor_position_changed(1, 11)
            self.assertEqual(1, widget._generate_completions_list.call_count)

    # ---------------------------------------------------------------
    # Failure tests
    # ---------------------------------------------------------------
    def test_construction_raises_error_for_unknown_language(self):
        # self.assertRaises causes a segfault here for some reason...
        try:
            CodeEditor("MyCoolLanguage")
        except ValueError:
            pass
        except Exception as exc:
            self.fail("Expected a Value error to be raised but found a " + exc.__name__)


if __name__ == '__main__':
    unittest.main()
