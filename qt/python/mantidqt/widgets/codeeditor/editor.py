# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
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

from mantid import simpleapi
from mantidqt.utils.qt import import_qt

# CodeEditor = import_qt('..._common', 'mantidqt.widgets.codeeditor', 'ScriptEditor')

CodeEditor_cpp = import_qt('..._common', 'mantidqt.widgets.codeeditor', 'ScriptEditor')


class CodeEditor(CodeEditor_cpp):

    def __init__(self, lexer, font=QFont(), parent=None):
        super(CodeEditor, self).__init__(lexer, font, parent)
        self.enableAutoCompletion(self.AcsAPIs)
        jedi.api.preload_module('numpy', 'mantid.simpleapi', 'matplotlib.pyplot')
        self.cursorPositionChanged.connect(self._on_cursor_position_changed)

        # A dict gives O(1) lookup and ensures we have no duplicates
        self._all_completions = dict()

    def _generate_jedi_script(self, line_no, col_no):
        return jedi.api.Script(self.text(), line_no, col_no, self.fileName(),
                               sys_path=sys.path)

    def _generate_completions_list(self, line_no, col_no):
        completions = self._generate_jedi_script(line_no, col_no).completions()
        completions_list = []
        for completion in completions:
            if self._all_completions.get(completion.name, None) is None:
                self._all_completions[completion.name] = True
                completions_list.append(completion.name)
        return completions_list

    def _generate_call_tips(self, line_no, col_no):
        call_sigs = self._generate_jedi_script(line_no, col_no).call_signatures()
        call_tips = []

        for signature in call_sigs:
            args = []
            for param in signature.params:
                arg = param.description.replace('param ', '').replace('\n', ' ')
                if arg:
                    args.append(arg)

            if not args:
                continue

            call_tip = "{}({})".format(signature.name, ', '.join(args))
            if self._all_completions.get(call_tip, None) is None:
                self._all_completions[call_tip] = True
                call_tips.append(call_tip)

        return call_tips

    @Slot(int, int)
    def _on_cursor_position_changed(self, line_no, col_no):
        """
        Slot to be executed when editor's cursor position has been changed.

        Note that Qt's line numbering starts at 0, whilst jedi's starts at 1.
        """
        if col_no < 2:
            return

        line = self.text().split('\n')[line_no]

        completions = []
        if line and not (line.rstrip()[-1] == '(' and col_no >= line.rfind('(')):
            completions = self._generate_completions_list(line_no + 1, col_no)

        # Since call tips will be generated the first time an open bracket is
        # entered, we need no worry about counting the number of open/closed
        # brackets
        if line.rfind(')') < line.rfind('('):
            call_tips = self._generate_call_tips(line_no + 1, col_no)
            completions.extend(call_tips)
        if completions:
            self.addToCompletionAPI(completions)
