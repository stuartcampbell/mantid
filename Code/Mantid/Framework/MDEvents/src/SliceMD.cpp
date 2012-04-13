/*WIKI* 
Algorithm that can take a slice out of an original [[MDEventWorkspace]] while preserving all the events contained therein.

It uses the same parameters as [[BinMD]] to determine a transformation to make from input->output workspace.
The difference is that [[BinMD]] sums events in a regular grid whereas SliceMD moves the events into the output workspace,
which boxes itself.

Please see [[BinMD]] for a detailed description of the parameters.

=== Axis-Aligned Slice ===

Events outside the range of the slice are dropped. The new output
MDEventWorkspace's dimensions only extend as far as the limit specified.

=== Non-Axis-Aligned Slice ===

The coordinates of each event are transformed according to the new basis vectors,
and placed in the output MDEventWorkspace. The dimensions of the output workspace
are along the basis vectors specified.

=== Splitting Parameters ===

The '''OutputBins''' parameter is interpreted as the "SplitInto" parameter for each dimension.
For instance, if you want the output workspace to split in 2x2x2,
you would specify OutputBins="2,2,2".

For 1D slices, it may make sense to specify a SplitInto parameter of 1 in every other dimension - that way, boxes
will only be split along the 1D direction.

=== Slicing a MDHistoWorkspace ===

It is possible to slice a [[MDHistoWorkspace]].
Each MDHistoWorkspace holds a reference to the [[MDEventWorkspace]] that created it,
as well as the coordinate transformation that was used.

In this case, the algorithm is executed on the original MDEventWorkspace, in the
proper coordinates. Perhaps surprisingly, the output of SliceMD on a MDHistoWorkspace is a
MDEventWorkspace!

Only the non-axis aligned slice method can be performed on a MDHistoWorkspace!
Of course, your basis vectors can be aligned with the dimensions, which is equivalent.


*WIKI*/

#include "MantidAPI/IMDEventWorkspace.h"
#include "MantidKernel/System.h"
#include "MantidMDEvents/MDEventFactory.h"
#include "MantidMDEvents/SliceMD.h"
#include "MantidGeometry/MDGeometry/MDImplicitFunction.h"
#include "MantidKernel/ThreadScheduler.h"
#include "MantidKernel/ThreadPool.h"
#include "MantidKernel/EnabledWhenProperty.h"
#include "MantidAPI/FileProperty.h"
#include "MantidGeometry/MDGeometry/MDHistoDimension.h"
#include "MantidKernel/BoundedValidator.h"

using namespace Mantid::Kernel;
using namespace Mantid::API;
using namespace Mantid::Geometry;

namespace Mantid
{
namespace MDEvents
{

  // Register the algorithm into the AlgorithmFactory
  DECLARE_ALGORITHM(SliceMD)
  


  //----------------------------------------------------------------------------------------------
  /** Constructor
   */
  SliceMD::SliceMD()
  {
  }
    
  //----------------------------------------------------------------------------------------------
  /** Destructor
   */
  SliceMD::~SliceMD()
  {
  }
  

  //----------------------------------------------------------------------------------------------
  /// Sets documentation strings for this algorithm
  void SliceMD::initDocs()
  {
    this->setWikiSummary("Make a MDEventWorkspace containing the events in a slice of an input MDEventWorkspace.");
    this->setOptionalMessage("Make a MDEventWorkspace containing the events in a slice of an input MDEventWorkspace.");
  }

  //----------------------------------------------------------------------------------------------
  /** Initialize the algorithm's properties.
   */
  void SliceMD::init()
  {
    declareProperty(new WorkspaceProperty<IMDWorkspace>("InputWorkspace","",Direction::Input), "An input MDWorkspace.");

    // Properties for specifying the slice to perform.
    this->initSlicingProps();

    declareProperty(new WorkspaceProperty<IMDEventWorkspace>("OutputWorkspace","",Direction::Output), "Name of the output MDEventWorkspace.");

    std::vector<std::string> exts;
    exts.push_back(".nxs");
    declareProperty(new FileProperty("OutputFilename", "", FileProperty::OptionalSave, exts),
        "Optional: Specify a NeXus file to write if you want the output workspace to be file-backed.");

    declareProperty(new PropertyWithValue<int>("Memory", -1),
        "If OutputFilename is specified to use a file back end:\n"
        "  The amount of memory (in MB) to allocate to the in-memory cache.\n"
        "  If not specified, a default of 40% of free physical memory is used.");
    //setPropertySettings("Memory", new EnabledWhenProperty("OutputFilename", IS_NOT_DEFAULT));


    declareProperty("TakeMaxRecursionDepthFromInput", true, "Copy the maximum recursion depth from the input workspace.");

    auto mustBePositiveInteger = boost::make_shared<BoundedValidator<int> >();
    mustBePositiveInteger->setLower(0);

    declareProperty("MaxRecursionDepth", 1000, mustBePositiveInteger,
    "Sets the maximum recursion depth to use. Can be used to constrain the workspaces internal structure");
    setPropertySettings("MaxRecursionDepth", new EnabledWhenProperty("TakeMaxRecursionDepthFromInput", IS_EQUAL_TO, "0"));

    setPropertyGroup("OutputFilename", "File Back-End");
    setPropertyGroup("Memory", "File Back-End");
  }


  //----------------------------------------------------------------------------------------------
  /** Copy the extra data (not signal, error or coordinates) from one event to another
   * with different numbers of dimensions
   *
   * @param srcEvent :: the source event, being copied
   * @param newEvent :: the destination event
   */
  template<size_t nd, size_t ond>
  inline void copyEvent(const MDLeanEvent<nd> &srcEvent, MDLeanEvent<ond> & newEvent)
  {
    // Nothing extra copy - this is no-op
    UNUSED_ARG(srcEvent);
    UNUSED_ARG(newEvent);
  }

  //----------------------------------------------------------------------------------------------
  /** Copy the extra data (not signal, error or coordinates) from one event to another
   * with different numbers of dimensions
   *
   * @param srcEvent :: the source event, being copied
   * @param newEvent :: the destination event
   */
  template<size_t nd, size_t ond>
  inline void copyEvent(const MDEvent<nd> & srcEvent, MDEvent<ond> & newEvent)
  {
    newEvent.setDetectorId(srcEvent.getDetectorID());
    newEvent.setRunIndex(srcEvent.getRunIndex());
  }


  //----------------------------------------------------------------------------------------------
  /** Perform the slice from nd input dimensions to ond output dimensions
   *
   * @param ws :: input workspace with nd dimensions
   * @tparam OMDE :: MDEvent type for the OUTPUT workspace
   * @tparam ond :: number of dimensions in the OUTPUT workspace
   */
  template<typename MDE, size_t nd, typename OMDE, size_t ond>
  void SliceMD::slice(typename MDEventWorkspace<MDE, nd>::sptr ws)
  {
    // Create the ouput workspace
    typename MDEventWorkspace<OMDE, ond>::sptr outWS(new MDEventWorkspace<OMDE, ond>());
    for (size_t od=0; od < m_binDimensions.size(); od++)
      outWS->addDimension(m_binDimensions[od]);
    outWS->initialize();
    // Copy settings from the original box controller
    BoxController_sptr bc = ws->getBoxController();
    BoxController_sptr obc = outWS->getBoxController();
    // Use the "number of bins" as the "split into" parameter
    for (size_t od=0; od < m_binDimensions.size(); od++)
      obc->setSplitInto(od, m_binDimensions[od]->getNBins());
    obc->setSplitThreshold(bc->getSplitThreshold());

    bool bTakeDepthFromInputWorkspace = getProperty("TakeMaxRecursionDepthFromInput");
    int tempDepth =  getProperty("MaxRecursionDepth");
    size_t maxDepth = bTakeDepthFromInputWorkspace? bc->getMaxDepth() : size_t(tempDepth);
    obc->setMaxDepth(maxDepth);

    obc->resetNumBoxes();
    // Perform the first box splitting
    outWS->splitBox();

    // --- File back end ? ----------------
    std::string filename = getProperty("OutputFilename");
    if (!filename.empty())
    {
      // First save to the NXS file
      g_log.notice() << "Running SaveMD" << std::endl;
      IAlgorithm_sptr alg = createSubAlgorithm("SaveMD");
      alg->setPropertyValue("Filename", filename);
      alg->setProperty("InputWorkspace", outWS);
      alg->executeAsSubAlg();
      // And now re-load it with this file as the backing.
      g_log.notice() << "Running LoadMD" << std::endl;
      alg = createSubAlgorithm("LoadMD");
      alg->setPropertyValue("Filename", filename);
      alg->setProperty("FileBackEnd", true);
      alg->setPropertyValue("Memory", getPropertyValue("Memory"));
      alg->executeAsSubAlg();
      // Replace the workspace with the loaded, file-backed one
      IMDEventWorkspace_sptr temp;
      temp = alg->getProperty("OutputWorkspace");
      outWS = boost::dynamic_pointer_cast<MDEventWorkspace<OMDE, ond> >(temp);
    }


    // Function defining which events (in the input dimensions) to place in the output
    MDImplicitFunction * function = this->getImplicitFunctionForChunk(NULL, NULL);

    std::vector<MDBoxBase<MDE,nd>*> boxes;
    // Leaf-only; no depth limit; with the implicit function passed to it.
    ws->getBox()->getBoxes(boxes, 1000, true, function);
    // Sort boxes by file position IF file backed. This reduces seeking time, hopefully.
    if (bc->isFileBacked())
      MDBoxBase<MDE, nd>::sortBoxesByFilePos(boxes);

    Progress * prog = new Progress(this, 0.0, 1.0, boxes.size());

    // The root of the output workspace
    MDBoxBase<OMDE,ond>* outRootBox = outWS->getBox();

    uint64_t totalAdded = 0;
    uint64_t numSinceSplit = 0;

    // Go through every box for this chunk.
    //PARALLEL_FOR_IF( !bc->isFileBacked() )
    for (int i=0; i<int(boxes.size()); i++)
    {
      MDBox<MDE,nd> * box = dynamic_cast<MDBox<MDE,nd> *>(boxes[i]);
      // Perform the binning in this separate method.
      if (box)
      {
        // An array to hold the rotated/transformed coordinates
        coord_t outCenter[ond];

        const std::vector<MDE> & events = box->getConstEvents();
        typename std::vector<MDE>::const_iterator it = events.begin();
        typename std::vector<MDE>::const_iterator it_end = events.end();
        for (; it != it_end; it++)
        {
          // Cache the center of the event (again for speed)
          const coord_t * inCenter = it->getCenter();

          if (function->isPointContained(inCenter))
          {
            // Now transform to the output dimensions
            m_transformFromOriginal->apply(inCenter, outCenter);

            // Create the event
            OMDE newEvent(it->getSignal(), it->getErrorSquared(), outCenter);
            // Copy extra data, if any
            copyEvent(*it, newEvent);
            // Add it to the workspace
            outRootBox->addEvent( newEvent );

            numSinceSplit++;
          }
        }

        // Every 20 million events, or at the last box: do splitting
        if (numSinceSplit > 20000000 || (i == int(boxes.size()-1)))
        {
          // This splits up all the boxes according to split thresholds and sizes.
          Kernel::ThreadScheduler * ts = new ThreadSchedulerFIFO();
          ThreadPool tp(ts);
          outWS->splitAllIfNeeded(ts);
          tp.joinAll();
          // Accumulate stats
          totalAdded += numSinceSplit;
          numSinceSplit = 0;
        }
      }

      // Progress reporting
      prog->report();

    }// for each box in the vector

    // Refresh all cache.
    outWS->refreshCache();

    g_log.notice() << totalAdded << " " << OMDE::getTypeName() << "'s added to the output workspace." << std::endl;

    if (outWS->isFileBacked())
    {
      // Update the file-back-end
      g_log.notice() << "Running SaveMD" << std::endl;
      IAlgorithm_sptr alg = createSubAlgorithm("SaveMD");
      alg->setProperty("UpdateFileBackEnd", true);
      alg->setProperty("InputWorkspace", outWS);
      alg->executeAsSubAlg();
    }

    this->setProperty("OutputWorkspace", boost::dynamic_pointer_cast<IMDEventWorkspace>(outWS));
    delete prog;
  }


  //----------------------------------------------------------------------------------------------
  /// Helper method
  template<typename MDE, size_t nd>
  void SliceMD::doExec(typename MDEventWorkspace<MDE, nd>::sptr ws)
  {
    if (m_outD==0)
      throw std::runtime_error("No output dimensions specified!");

    // Templated method needs to call another templated method depending on the # of output dimensions.
    if (MDE::getTypeName() == "MDLeanEvent")
    {
      if (m_outD==1)      this->slice<MDE,nd,MDLeanEvent<1>,1>(ws);
      else if (m_outD==2) this->slice<MDE,nd,MDLeanEvent<2>,2>(ws);
      else if (m_outD==3) this->slice<MDE,nd,MDLeanEvent<3>,3>(ws);
      else if (m_outD==4) this->slice<MDE,nd,MDLeanEvent<4>,4>(ws);
      else
        throw std::runtime_error("Number of output dimensions > 4. This is not currently handled.");
    }
    else if (MDE::getTypeName() == "MDEvent")
    {
      if (m_outD==1)      this->slice<MDE,nd,MDEvent<1>,1>(ws);
      else if (m_outD==2) this->slice<MDE,nd,MDEvent<2>,2>(ws);
      else if (m_outD==3) this->slice<MDE,nd,MDEvent<3>,3>(ws);
      else if (m_outD==4) this->slice<MDE,nd,MDEvent<4>,4>(ws);
      else
        throw std::runtime_error("Number of output dimensions > 4. This is not currently handled.");
    }
    else
      throw std::runtime_error("Unexpected MDEvent type '" + MDE::getTypeName() + "'. This is not currently handled.");
  }



  //----------------------------------------------------------------------------------------------
  /** Execute the algorithm.
   */
  void SliceMD::exec()
  {
    // Input MDEventWorkspace
    m_inWS = getProperty("InputWorkspace");

    // Run through the properties to create the transform you need
    createTransform();

    CALL_MDEVENT_FUNCTION(this->doExec, m_inWS);
  }



} // namespace Mantid
} // namespace MDEvents

