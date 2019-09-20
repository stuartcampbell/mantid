# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
# pylint: disable=invalid-name, anomalous-backslash-in-string, attribute-defined-outside-init

from mantid.api import IFunction1D, FunctionFactory
import numpy as np
from scipy import special as sp


class InternBessel(IFunction1D):

    def category(self):
        return "Muon"

    def init(self):
        self.declareParameter("AlphaFast", 0.5, 'Asymmetry of the fast part')
        self.declareParameter("AlphaSlow", 0.5, 'Asymmetry of the slow part')
        self.declareParameter("Phi", 0.0, 'Phase (rad)')
        self.declareParameter("Nu", 0.5, 'Frequency (MHz)')
        self.declareParameter("LambdaF", 1.0, 'Damping rate of fast part')
        self.declareParameter("LambdaS", 0.1, 'Damping rate of slow part')
        self.declareParameter("BetaF", 1.0, 'Stretched term of fast part')
        self.declareParameter("BetaS", 0.1, 'Stretched term of slow part')

    def function1D(self, x):
        alphaF = self.getParameterValue("AlphaFast")
        alphaS = self.getParameterValue("AlphaSlow")
        phi = self.getParameterValue("Phi")
        nu = self.getParameterValue("Nu")
        LambdaF = self.getParameterValue("LambdaF")
        LambdaS = self.getParameterValue("LambdaS")
        betaF = self.getParameterValue("BetaF")
        betaS = self.getParameterValue("BetaS")
        return alphaF * sp.j0(2 * np.pi * nu * x + phi) * np.exp(- (LambdaF * x) ** betaF) + alphaS * np.exp(- (LambdaS * x) ** betaS)

FunctionFactory.subscribe(InternBessel)
