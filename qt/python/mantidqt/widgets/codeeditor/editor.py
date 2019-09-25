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

from qtpy.QtGui import QFont

from mantidqt.utils.qt import import_qt

CodeEditor_cpp = import_qt('..._common', 'mantidqt.widgets.codeeditor', 'ScriptEditor')


class CodeEditor(CodeEditor_cpp):

    def __init__(self, lexer, font=QFont(), parent=None):
        super(CodeEditor, self).__init__(lexer, font, parent)
