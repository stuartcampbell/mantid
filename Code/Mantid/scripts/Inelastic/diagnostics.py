"""
Provide functions to perform various diagnostic tests.

The output of each function is a workspace containing a single bin where:
    0 - denotes a masked spectra and
    1 - denotes an unmasked spectra.

This workspace can be summed with other masked workspaces to accumulate
masking and also passed to MaskDetectors to match masking there.
"""
from mantidsimple import *
import CommonFunctions as common

class ArgumentParser(object):
    
    def __init__(self, keywords):
        self.start_index = None # Make this more general for anything that is missing!
        self.end_index = None
        for key, value in keywords.iteritems():
            setattr(self, key, value)
            
        

def diagnose(white_int, **kwargs):
    """
        Run diagnostics on the provided workspaces.

        Required inputs:
        
          white_int  - A workspace, run number or filepath of a white beam run. If a run/file is given it
                       simply loaded and integrated. A workspace is assumed to be prepared in it's integrated form
        
        Optional inputs:
          start_index    - The index to start the diag
          end_index    - The index to finish the diag
          background_int - A workspace, run number or filepath of a sample run that has been integrated over the background region.
                          If a run/file is given it simply loaded and integrated across the whole range
          sample_counts - A workspace containing the total integrated counts from a sample run
          second_white - If provided an additional set of tests is performed on this.
          hard_mask  - A file specifying those spectra that should be masked without testing 
          tiny        - Minimum threshold for acceptance 
          huge       - Maximum threshold for acceptance
          van_out_lo  - Lower bound defining outliers as fraction of median value 
          van_out_hi  - Upper bound defining outliers as fraction of median value 
          van_lo      - Fraction of median to consider counting low for the white beam diag 
          van_hi      - Fraction of median to consider counting high for the white beam diag 
          van_sig     - Error criterion as a multiple of error bar i.e. to fail the test, the magnitude of the
                        difference with respect to the median value must also exceed this number of error bars 
          samp_zero    - If true then zeroes in background are masked also
          samp_lo      - Fraction of median to consider counting low for the background  diag 
          samp_hi      - Fraction of median to consider counting high for the background diag
          samp_sig     - Error criterion as a multiple of error bar i.e. to fail the test, the magnitude of the\n"
                      "difference with respect to the median value must also exceed this number of error bars
          variation  - The number of medians the ratio of the first/second white beam can deviate from
                       the average by 
          bleed_test - If true then the CreatePSDBleedMask algorithm is run
          bleed_maxrate - If the bleed test is on then this is the maximum framerate allowed in a tube
          bleed_pixels - If the bleed test is on then this is the number of pixels ignored within the
                         bleed test diagnostic
          print_results - If True then the results are printed to the screen
    """
    if white_int is None and str(white_int) != '':
        raise RuntimeError("No white beam integral specified. This is the minimum required to run diagnostics")
    
    # Grab the arguments
    parser = ArgumentParser(kwargs)
    start_index = parser.start_index
    end_index = parser.end_index

    # Map the test number to the results
    # Each element is the mask workspace name then the number of failures
    test_results = [ [None, None], [None, None], [None, None], [None, None], [None, None]]

    # Load the hard mask file if necessary
    hard_mask_spectra = ''
    if 'hard_mask' in kwargs:
        hard_mask_spectra = common.load_mask(parser.hard_mask)
        test_results[0][0] = os.path.basename(parser.hard_mask)

    # Hard mask
    masking = MaskDetectors(white_int, SpectraList=hard_mask_spectra)
    # Find out how many detectors we hard masked
    hard_mask_spectra = masking['SpectraList'].value
    test_results[0][1] = len(hard_mask_spectra)

    # White beam Test
    __white_masks, num_failed = do_white_test(white_int, parser.tiny, parser.huge, 
                                              parser.van_out_lo, parser.van_out_hi,
                                              parser.van_lo, parser.van_hi, 
                                              parser.van_sig, start_index, end_index)
    test_results[1] = [str(__white_masks), num_failed]
    add_masking(white_int, __white_masks, start_index, end_index)
    DeleteWorkspace(__white_masks)

    # Second white beam test
    if 'second_white' in kwargs:
        __second_white_masks, num_failed = do_second_white_test(white_int, parser.second_white, parser.tiny, parser.huge, 
                                                   parser.van_out_lo, parser.van_out_hi,
                                                   parser.van_lo, parser.van_hi, parser.variation,
                                                   parser.van_sig, start_index, end_index)
        test_results[2] = [str(__second_white_masks), num_failed]
        add_masking(white_int, __second_white_masks, start_index, end_index)

    #
    # Zero total count check for sample counts
    #
    zero_count_failures = 0
    if kwargs.get('sample_counts',None) is not None and kwargs.get('samp_zero',False):
        add_masking(parser.sample_counts, white_int)
        checker = FindDetectorsOutsideLimits(InputWorkspace=parser.sample_counts, OutputWorkspace='maskZero',
                                             StartWorkspaceIndex=start_index, EndWorkspaceIndex=end_index,
                                             LowThreshold=1e-10, HighThreshold=1e100)
        zero_count_failures = checker['NumberOfFailures'].value 
        maskZero = checker.workspace()
        add_masking(white_int, maskZero, start_index, end_index)
        DeleteWorkspace(maskZero)

    #
    # Background check
    #
    if hasattr(parser, 'background_int'):
        add_masking(parser.background_int, white_int)
        __bkgd_mask, failures = do_background_test(parser.background_int, parser.samp_lo, 
                                                   parser.samp_hi, parser.samp_sig, parser.samp_zero, start_index, end_index)
        test_results[3] = [str(__bkgd_mask), zero_count_failures + failures]
        add_masking(white_int, __bkgd_mask, start_index, end_index)
        DeleteWorkspace(__bkgd_mask)
    
    #
    # Bleed test
    #
    if hasattr(parser, 'bleed_test') and parser.bleed_test:
        if not hasattr(parser, 'sample_run'):
            raise RuntimeError("Bleed test requested but the sample_run keyword has not been provided")
        __bleed_masks, failures = do_bleed_test(parser.sample_run, parser.bleed_maxrate, parser.bleed_pixels)
        test_results[4] = [str(__bleed_masks), failures]
        add_masking(white_int, __bleed_masks)
        DeleteWorkspace(__bleed_masks)
    
    if hasattr(parser, 'print_results') and parser.print_results:
        print_test_summary(test_results)

#-------------------------------------------------------------------------------

def add_masking(input_ws, mask_ws, start_index=None, end_index=None):
    """
    Mask the Detectors on the input workspace that are masked 
    on the mask_ws. Avoids a current bug in using MaskDetectors with a MaskedWorkspace in a loop 
    """
    #MaskDetectors(input_ws, MaskedWorkspace=mask_ws, StartWorkspaceIndex=start_index, EndWorkspaceIndex=end_index)
    masked = ExtractMasking(mask_ws, '__tmp')
    MaskDetectors(input_ws, DetectorList=masked.getPropertyValue("DetectorList"))
    DeleteWorkspace(masked.workspace())

#-------------------------------------------------------------------------------

def do_white_test(white_int, tiny, large, out_lo, out_hi, median_lo, median_hi, sigma, 
                  start_index=None, end_index=None):
    """
    Run the diagnostic tests on the integrated white beam run

    Required inputs:
    
      white_int - An integrated workspace
      tiny      - Minimum threshold for acceptance
      large     - Maximum threshold for acceptance
      out_lo    - Lower bound defining outliers as fraction of median value (default = 0.01)
      out_hi    - Upper bound defining outliers as fraction of median value (default = 100.)
      median_lo - Fraction of median to consider counting low
      median_hi - Fraction of median to consider counting high
      sigma     - Error criterion as a multiple of error bar
                      
    """
    mtd.sendLogMessage('Running first white beam test')

    # Make sure we are a MatrixWorkspace
    ConvertToMatrixWorkspace(white_int, white_int)
    
    # The output workspace will have the failed detectors masked
    white_masks = FindDetectorsOutsideLimits(white_int, OutputWorkspace='white_masks', StartWorkspaceIndex=start_index,
                                             EndWorkspaceIndex=end_index,
                                             HighThreshold=large, LowThreshold=tiny)
    MaskDetectors(white_int, MaskedWorkspace=white_masks.workspace(),StartWorkspaceIndex=start_index, EndWorkspaceIndex=end_index)
    num_failed = white_masks['NumberOfFailures'].value
    DeleteWorkspace(white_masks.workspace())

    white_masks = MedianDetectorTest(white_int, OutputWorkspace='white_masks', StartWorkspaceIndex=start_index,
                                     EndWorkspaceIndex=end_index, SignificanceTest=sigma,
                                     LowThreshold=median_lo, HighThreshold=median_hi, 
                                     LowOutlier=out_lo, HighOutlier=out_hi, ExcludeZeroesFromMedian=False)
    MaskDetectors(white_int, MaskedWorkspace=white_masks.workspace(), StartWorkspaceIndex=start_index, EndWorkspaceIndex=end_index)
    num_failed += white_masks['NumberOfFailures'].value

    return white_masks.workspace(), num_failed

#-------------------------------------------------------------------------------

def do_second_white_test(white_counts, comp_white_counts, tiny, large, out_lo, out_hi, 
                         median_lo, median_hi, sigma, variation,
                         start_index=None, end_index=None):
    """
    Run additional tests comparing given another white beam count workspace, comparing
    to the first

    Required inputs:
    
      white_counts  - A workspace containing the integrated counts from a
                      white beam vanadium run
      comp_white_counts  - A workspace containing the integrated counts from a
                      white beam vanadium run
      tiny          - Minimum threshold for acceptance
      large         - Maximum threshold for acceptance
      median_lo     - Fraction of median to consider counting low
      median_hi     - Fraction of median to consider counting high
      signif          - Counts within this number of multiples of the 
                      standard dev will be kept
      variation     - Defines a range within which the ratio of the two counts is
                      allowed to fall in terms of the number of medians
    """ 
    mtd.sendLogMessage('Running second white beam test')

    # What shall we call the output
    lhs_names = lhs_info('names')
    if len(lhs_names) > 0:
        ws_name = lhs_names[0]
    else:
        ws_name = '__do_second_white_test'
    
    # Make sure we are a MatrixWorkspace
    ConvertToMatrixWorkspace(white_counts, white_counts)
    ConvertToMatrixWorkspace(comp_white_counts, comp_white_counts)
    
    # Do the white beam test
    __second_white_tests, failed = do_white_test(comp_white_counts, tiny, large, median_lo, median_hi, 
                                                 sigma, start_index, end_index)
    # and now compare it with the first
    effic_var = DetectorEfficiencyVariation(white_counts, comp_white_counts, ws_name, Variation=variation,
                                            StartWorkspaceIndex=start_index, EndWorkspaceIndex=end_index,)
    # Total number of failures
    num_failed = effic_var['NumberOfFailures'].value + failed

    DeleteWorkspace(str(__second_white_tests))
    # Mask those that failed
    maskWS = effic_var.workspace()
    MaskDetectors(white_counts, MaskedWorkspace=maskWS)
    MaskDetectors(comp_white_counts, MaskedWorkspace=maskWS)
  
    return maskWS, num_failed

#------------------------------------------------------------------------------
def normalise_background(background_int, white_int, second_white_int=None):
    """Normalize the background integrals
    
       If two white beam files are provided then the background integrals
       are normalized by the harmonic mean of the two:
       
       hmean = 2.0/((1/v1) + (1/v2)) = 2v1*v2/(v1+v2)
       
       If only a single white
       beam is provided then the background is normalized by the white beam itself 
    
    """
    if second_white_int is None:
        background_int /= white_int
    else:
        hmean = 2.0*white_int*second_white_int/(white_int+second_white_int)
        background_int /= hmean
        DeleteWorkspace(hmean)

#------------------------------------------------------------------------------
def do_background_test(background_int, median_lo, median_hi, sigma, mask_zero, 
                        start_index=None, end_index=None):
    """
    Run the background tests

    Required inputs:
      background_int - An integrated workspace
      median_lo - Fraction of median to consider counting low
      median_hi - Fraction of median to consider counting high
      sigma     - Error criterion as a multiple of error bar
      mask_zero - If True, zero background counts will be considered a fail

    """
    mtd.sendLogMessage('Running background count test')

    # What shall we call the output
    lhs_names = lhs_info('names')
    if len(lhs_names) > 0:
        ws_name = lhs_names[0]
    else:
        ws_name = '__do_background_test'

    mask_bkgd = MedianDetectorTest(InputWorkspace=background_int,  OutputWorkspace='mask_bkgd', 
                                   StartWorkspaceIndex=start_index, EndWorkspaceIndex=end_index,
                                   SignificanceTest=sigma, 
                                   LowThreshold=median_lo, HighThreshold=median_hi, 
                                   LowOutlier=0.0, HighOutlier=1e100, ExcludeZeroesFromMedian=True)

    return mask_bkgd.workspace(), mask_bkgd['NumberOfFailures'].value

#-------------------------------------------------------------------------------

def do_bleed_test(sample_run, max_framerate, ignored_pixels):
    """Runs the CreatePSDBleedMask algorithm

    Input:
    sample_run  -  The run number of the sample
    max_framerate - The maximum allowed framerate in a tube. If None, the instrument defaults are used.
    ignored_pixels - The number of central pixels to ignore. If None, the instrument defaults are used.
    """
    mtd.sendLogMessage('Running PSD bleed test')
    # Load the sample run
    data_ws = common.load_run(sample_run)

    if max_framerate is None:
        max_framerate = float(data_ws.getInstrument().getNumberParameter('max-tube-framerate')[0])
    if ignored_pixels is None:
        ignored_pixels = int(data_ws.getInstrument().getNumberParameter('num-ignored-pixels')[0])
    else:
        # Make sure it is an int
        ignored_pixels = int(ignored_pixels)

    # What shall we call the output
    lhs_names = lhs_info('names')
    if len(lhs_names) > 0:
        ws_name = lhs_names[0]
    else:
        ws_name = '__do_bleed__test'

    bleed_test = CreatePSDBleedMask(data_ws, ws_name, MaxTubeFramerate=max_framerate,\
                                    NIgnoredCentralPixels=ignored_pixels)

    num_failed = bleed_test['NumberOfFailures'].value
    return bleed_test.workspace(), num_failed

#-------------------------------------------------------------------------------

def print_test_summary(test_results):
    """Print a summary of the failures per test run.

    Input:
    test_results - A list or tuple containing either the number of failed spectra or None
                   indicating that the test was not run
    """
    num_diags = 5
    if len(test_results) != num_diags:
        raise ValueError("Invalid input for print_test_summary. A list of %d numbers is expected." % num_diags)

    tests_run=False
    for failures in test_results:
        if failures is not None:
            tests_run = True

    if tests_run == False:
        print "No tests have been run!"
        return

    summary = (
        ['Hard mask:',test_results[0]], \
        ['First white beam test:',test_results[1]], \
        ['Second white beam test:',test_results[2]], \
        ['Background test:',test_results[3]], \
        ['PSD Bleed test :',test_results[4]] \
        )

    print '==== Diagnostic Test Summary ===='

    max_name_length = -1
    max_ws_length = -1
    for key in range(num_diags):
        result = summary[key]
        name_length = len(str(result[0]))
        ws_length = len(str(result[1][0]))
        if name_length > max_name_length:
            max_name_length = name_length
        if ws_length > max_ws_length:
            max_ws_length = ws_length

    max_name_length += 2
    max_ws_length += 2
    for result in summary:
        test_name = str(result[0])
        workspace = str(result[1][0])
        nfailed = str(result[1][1])
        line = test_name + ' '*(max_name_length-len(test_name)) + \
               workspace + ' '*(max_ws_length-len(workspace)) + str(nfailed)
        print line
    # Append a new line
    print ''

#-------------------------------------------------------------------------------

def get_failed_spectra_list(diag_workspace):
    """Compile a list of spectra numbers that are marked as
    masked in the given workspace

    Input:

     diag_workspace  -  A workspace containing masking
    """
    if type(diag_workspace) == str:
        diag_workspace = mtd[diag_workspace]

    failed_spectra = []
    for i in range(diag_workspace.getNumberHistograms()):
	try:
            det = diag_workspace.getDetector(i)
	except RuntimeError:
            continue
	if det.isMasked():
            failed_spectra.append(diag_workspace.getSpectrum(i).getSpectrumNo())

    return failed_spectra
