# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2017 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
#  This file is part of the mantidqt package
#
#
from __future__ import (absolute_import, unicode_literals)

import sys

import jedi
from qtpy.QtCore import Slot
from qtpy.QtGui import QFont

from mantidqt.utils.qt import import_qt

CodeEditor_cpp = import_qt('..._common', 'mantidqt.widgets.codeeditor', 'ScriptEditor')


class CodeEditor(CodeEditor_cpp):

    def __init__(self, lexer, font=QFont(), parent=None):
        super(CodeEditor, self).__init__(lexer, font, parent)
        jedi.api.preload_module('numpy', 'mantid.simpleapi')
        self.cursorPositionChanged.connect(self._on_cursorPositionChanged)

    def _get_completions_list(self, line_no, col_no):
        j_script = jedi.Script(self.text(), line_no, col_no, self.fileName(), sys_path=sys.path)
        completions = [completion.name for completion in j_script.completions()]
        return completions

    @Slot(int, int)
    def _on_cursorPositionChanged(self, line_no, col_no):
        if col_no != 0:
            self.updateCompletionAPI(self._get_completions_list(line_no + 1, col_no))
