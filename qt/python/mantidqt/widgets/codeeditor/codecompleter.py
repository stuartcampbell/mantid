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

from mantidqt.widgets.codeeditor.editor import CodeEditor


class CodeCompleter:

    def __init__(self, editor, env_globals):
        self.editor = editor
        self.env_globals = env_globals

        self.editor.enableAutoCompletion(CodeEditor.AcsAll)
        jedi.api.preload_module('numpy', 'mantid.simpleapi', 'matplotlib.pyplot')
        self.editor.cursorPositionChanged.connect(self._on_cursor_position_changed)

        # A dict gives O(1) lookup and ensures we have no duplicates
        self._all_completions = dict()

    def _generate_jedi_interpreter(self, line_no=None, col_no=None):
        return jedi.Interpreter(self.editor.text(), [self.env_globals], line=line_no,
                                column=col_no, path=self.editor.fileName(), sys_path=sys.path)

    def _generate_completions_list(self, line_no=None, col_no=None):
        completions = self._generate_jedi_interpreter(line_no, col_no).completions()
        completions_list = []
        for completion in completions:
            if self._all_completions.get(completion.name, None) is None:
                self._all_completions[completion.name] = True
                completions_list.append(completion.name)
        return completions_list

    def _generate_call_tips(self, line_no=None, col_no=None):
        call_sigs = self._generate_jedi_interpreter(line_no, col_no).call_signatures()
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

    def _on_cursor_position_changed(self, line_no, col_no):
        """
        Slot to be executed when editor's cursor position has been changed.

        Note that Qt's line numbering starts at 0, whilst jedi's starts at 1.

        :param int line_no: The cursor's new line number
        :param int col_no: The cursor's new column number
        """
        if col_no < 2:
            return

        line = self.editor.text().split('\n')[line_no]

        completions = []
        if line.strip() and not (line.rstrip()[-1] == '(' and col_no >= line.rfind('(')):
            completions = self._generate_completions_list(line_no + 1, col_no)

        # Since call tips will be generated the first time an open bracket is
        # entered, we need not worry about counting the number of open/closed
        # brackets
        if line.rfind(')') < line.rfind('('):
            call_tips = self._generate_call_tips(line_no + 1, col_no)
            completions.extend(call_tips)
        if completions:
            self.editor.addToCompletionAPI(completions)
