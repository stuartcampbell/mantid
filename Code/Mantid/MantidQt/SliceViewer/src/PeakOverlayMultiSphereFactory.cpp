#include "MantidQtSliceViewer/PeakOverlayMultiSphereFactory.h"
#include "MantidQtSliceViewer/PeakOverlayMultiSphere.h"
#include <boost/make_shared.hpp>

using namespace Mantid::API;

namespace MantidQt
{
  namespace SliceViewer
  {

    PeakOverlayMultiSphereFactory::PeakOverlayMultiSphereFactory(IPeaksWorkspace_sptr peaksWS, QwtPlot * plot, QWidget * parent, const size_t colourNumber) :
        PeakOverlayViewFactoryBase(plot, parent, colourNumber),
        m_peakRadius(0),
        m_backgroundInnerRadius(0),
        m_backgroundOuterRadius(0),
        m_peaksWS(peaksWS),
        m_FOM(0)
    {
      if (m_peaksWS->hasIntegratedPeaks())
      {
        m_peakRadius =
            m_peaksWS->run().getPropertyValueAsType<std::vector<double> >("PeakRadius");
        m_backgroundInnerRadius =
            m_peaksWS->run().getPropertyValueAsType<std::vector<double> >("BackgroundInnerRadius");
        m_backgroundOuterRadius =
            m_peaksWS->run().getPropertyValueAsType<std::vector<double> >("BackgroundOuterRadius");
        m_FOM = 2; // Possible to display workspaces with this factory.
      }
    }

    boost::shared_ptr<PeakOverlayView> PeakOverlayMultiSphereFactory::createView(PeakTransform_const_sptr transform) const
    {
      // Construct all physical peaks
      VecPhysicalSphericalPeak physicalPeaks(m_peaksWS->rowCount());
      for(size_t i = 0; i < physicalPeaks.size(); ++i)
      {
        const IPeak& peak = m_peaksWS->getPeak(static_cast<int>(i));
        auto position = transform->transformPeak(peak);
        physicalPeaks[i] = boost::make_shared<PhysicalSphericalPeak>(position, m_peakRadius[i], m_backgroundInnerRadius[i], m_backgroundOuterRadius[i]);
      }

      // Make the overlay widget.
      return boost::make_shared<PeakOverlayMultiSphere>(m_plot, m_parent, physicalPeaks, this->m_peakColour, this->m_backColour);
    }

    PeakOverlayMultiSphereFactory::~PeakOverlayMultiSphereFactory()
    {
    }

    int PeakOverlayMultiSphereFactory::FOM() const
    {
      return m_FOM;
    }

  }
}
