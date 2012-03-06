# Basic parameters  for  Triphylite Crystal
#Name of the workspaces to create
ws = "TOPAZ_3131"
filename = ws+"_event.nxs"
LoadEventNexus(Filename=filename,OutputWorkspace=ws,FilterByTofMin='3000',FilterByTofMax='16000')

# Load optimized DetCal file
#LoadIsawDetCal(InputWorkspace=ws,Filename="/SNS/TOPAZ/shared/Spectra/TOPAZ_8Sept11.DetCal")

# Spherical Absorption and Lorentz Corrections
AnvredCorrection(InputWorkspace=ws,OutputWorkspace=ws,LinearScatteringCoef="0.451",LinearAbsorptionCoef="0.993",Radius="0.14")

# Convert to Q space
ConvertToDiffractionMDWorkspace(InputWorkspace=ws,OutputWorkspace=ws+'_MD2',LorentzCorrection='0',
        OutputDimensions='Q (lab frame)', SplitInto='2',SplitThreshold='150')
# Find peaks
FindPeaksMD(InputWorkspace=ws+'_MD2',MaxPeaks='100',OutputWorkspace=ws+'_peaksLattice')
# 3d integration to centroid peaks
CentroidPeaksMD(InputWorkspace=ws+'_MD2',CoordinatesToUse='Q (lab frame)',
	PeakRadius='0.12',PeaksWorkspace=ws+'_peaksLattice',OutputWorkspace=ws+'_peaksLattice')
# Find the UB matrix using the peaks and known lattice parameters
FindUBUsingLatticeParameters(PeaksWorkspace=ws+'_peaksLattice',a='10.3522',b='6.0768',c='4.7276',
                alpha='90',beta='90',gamma='90', NumInitial='20', Tolerance='0.12')
# And index to HKL            
IndexPeaks(PeaksWorkspace=ws+'_peaksLattice', Tolerance='0.12')
# Integrate peaks in Q space using spheres
IntegratePeaksMD(InputWorkspace=ws+'_MD2',PeakRadius='0.12',
	BackgroundRadius='0.18',BackgroundStartRadius='0.15',
	PeaksWorkspace=ws+'_peaksLattice',OutputWorkspace=ws+'_peaksLattice')
# Save for SHELX
SaveHKL(InputWorkspace=ws+'_peaksLattice', Filename=ws+'.hkl')

# Find peaks again for FFT
FindPeaksMD(InputWorkspace=ws+'_MD2',MaxPeaks='100',OutputWorkspace=ws+'_peaksFFT')
# 3d integration to centroid peaks
CentroidPeaksMD(InputWorkspace=ws+'_MD2',	CoordinatesToUse='Q (lab frame)',
	PeakRadius='0.12',PeaksWorkspace=ws+'_peaksFFT',OutputWorkspace=ws+'_peaksFFT')
# Find the UB matrix using FFT
FindUBUsingFFT(PeaksWorkspace=ws+'_peaksFFT',MinD=3.,MaxD=14.)

## TODO conventional cell

# And index to HKL            
IndexPeaks(PeaksWorkspace=ws+'_peaksFFT', Tolerance='0.12')
# Integrate peaks in Q space using spheres
IntegratePeaksMD(InputWorkspace=ws+'_MD2',PeakRadius='0.12',
	BackgroundRadius='0.18',BackgroundStartRadius='0.15',
	PeaksWorkspace=ws+'_peaksFFT',OutputWorkspace=ws+'_peaksFFT')
# Save for SHELX
SaveHKL(InputWorkspace=ws+'_peaksFFT', Filename=ws+'FFT.hkl')


# Copy the UB matrix back to the original workspace
CopySample(InputWorkspace=ws+'_peaksLattice',OutputWorkspace=ws,
		CopyName='0',CopyMaterial='0',CopyEnvironment='0',CopyShape='0',  CopyLattice=1)
# Convert to reciprocal space, in the sample frame
ConvertToDiffractionMDWorkspace(InputWorkspace=ws,OutputWorkspace=ws+'_HKL',
		OutputDimensions='HKL',LorentzCorrection='0', SplitInto='2',SplitThreshold='150')
# Bin to a regular grid
BinMD(InputWorkspace=ws+'_HKL',AlignedDim0='H, -20, 0, 800',AlignedDim1='K, -3, 7, 50',
      AlignedDim2='L, -10, 0,  800',OutputWorkspace=ws+'_binned')
# Show in slice Viewer		
sv = plotSlice(ws+'_binned', xydim=('H','L'), slicepoint=[0, -2, 0], colorscalelog=True)
sv.setColorMapBackground(0,0,0)

# Again, lower resolution
BinMD(InputWorkspace=ws+'_HKL',AlignedDim0='H, -20, 0, 200',AlignedDim1='K, -3, 7, 50',
      AlignedDim2='L, -10, 00, 100',OutputWorkspace=ws+'_binned_lowres')
sv = plotSlice(ws+'_binned_lowres', xydim=('H','L'), slicepoint=[0, -2, 0], colorscalelog=True)
sv.setColorMapBackground(0,0,0)

# Dynamic binning
SliceMD(InputWorkspace=ws+'_HKL',AlignedDim0='H, -20, 0, 2',AlignedDim1='K, -2.2, -1.8, 1',
      AlignedDim2='L, -10, 0, 2',OutputWorkspace=ws+'_slice')
sv = plotSlice(ws+'_slice', xydim=('H','L'), slicepoint=[0, -2, 0], colorscalelog=True)
sv.setColorMapBackground(0,0,0)


