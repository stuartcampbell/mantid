============
MuSR Changes
============

.. contents:: Table of Contents
   :local:

New
###
Fitting functions that have been added:
- :ref:`CombGaussLorentszKT<algm-CombLorentzKT>`
- :ref:`StretchedKT<algm-StretchedKT>`
- :ref:`FmuF<algm-FmuF>`
- :ref:`Bessel<algm-Bessel>`
- :ref:`Meier<algm-Meier>`
- :ref:`MuH<algm-MuH>`
- :ref:`StandardSC<algm-StandardSC>`
- :ref:`SilverBaseline<algm-SilverBaseline>`
- :ref:`TFMuonium<algm-TFMuonium>`
- :ref:`HighTFMuonium<algm-HighTFMuonium>`
- :ref:`TriangleOsc<algm-TriangleOsc>`
- :ref:`StaticLorentzianKT<algm-StaticLorentzianKT>`
- :ref:`SpinGlass<algm-SpinGlass>`
- :ref:`RFresonance<algm-RFresonance>`
- :ref:`ZFMuonium<algm-ZFMuonium>`
- :ref:`DampedBessel<algm-DampedBessel>`
- :ref:`AFM_LF<algm-AFM_LF>`
- :ref:`AFM_ZF<algm-AFM_ZF>`
- :ref:`GausBroadGausKT<algm-GausBroadGausKT>`
- :ref:`PCRmagRedfield<algm-PCRmagRedfield>`
- :ref:`PCRmagnet<algm-PCRmagnet>`
- :ref:`PCRmagnetfnorm<algm-PCRmagnetfnorm>`
- :ref:`RedfielsCutoff<algm-RedfielsCutoff>`
- :ref:`SCGapSwave<algm-SCGapSwave>`
- :ref:`SCgapAnisoSWave<algm-ScgapAnisoSWave>`
- :ref:`SCgapDwave<algm-SCgapDwave>`
- :ref:`SCgapPwave<algm-SCgapPwave>`
- :ref:`ZFelectronDipole<algm-ZFelectronDiple>`
- :ref:`ZFprotonDipole<algm-ZFprotoDipole>`
- :ref:`PCRmagnetZFKT<algm-PCRmagnetZFKT>`

Improvements
############
  * Improved the speed of plotting during sequential fits.

Removed
#######


Bug Fixes
#########
  * Fixed an issue where changeing the normalisation on a plot with autoscale disabled throws an exception.
  * Fixed an issue where warnings about adding workspaces to workspace groups multiple times were appearing in the log.
  * Fixed an issue where logs in TF asymmetry mode were not being propogated to the results tab.

Known Issues
############

Algorithms
----------

Improvements
############

- Improve the handling of :ref:`LoadPSIMuonBin<algm-LoadPSIMuonBin-v1>` where a poor date is provided.
- In TF asymmetry mode now rescales the fit to match the rescaled data.

Interfaces
----------

Muon Analysis 2 and Frequency Domain Analysis
#############################################

- When loading PSI data if the groups given are poorly stored in the file, it should now produce unique names in the grouping tab for groups.
- When switching between data sets groups selected to fit are remembered.
- The FFT tab now uses the group pair selection to make a guess at the users selection for workspaces.
- Can now plot FFT's of PhaseQuad data.
- No longer produces an error if using multiple runs and the user plots all the FFT results when no imaginary data was used.
- Muon Analysis (new) and Frequency Domain Analysis (new) work with project recovery. 

Algorithms
----------

Improvements
############

- :ref:`LoadPSIMuonBin <algm-LoadPSIMuonBin>` has been improved to correctly load data other than data from Dolly at the SmuS/PSI.
- When there is a T0 for each spectrum, :ref:`LoadPSIMuonBin <algm-LoadPSIMuonBin>` chooses the max value out of the array instead of the first value.

:ref:`Release 4.2.0 <v4.2.0>`
