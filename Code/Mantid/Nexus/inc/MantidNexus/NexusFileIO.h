#ifndef NEXUSFILEIO_H
#define NEXUSFILEIO_H
#include <napi.h>
#include "MantidDataObjects/Workspace2D.h"
#include "MantidKernel/TimeSeriesProperty.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include <limits.h>
namespace Mantid
{
  namespace NeXus
  {
    /** @class NexusFileIO NexusFileIO.h NeXus/NexusFileIO.h

    Utility method for saving NeXus format of Mantid Workspace

    Copyright &copy; 2007-8 STFC Rutherford Appleton Laboratory

    This file is part of Mantid.

    Mantid is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    Mantid is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    File change history is stored at: <https://svn.mantidproject.org/mantid/trunk/Code/Mantid>. 
    Code Documentation is available at: <http://doxygen.mantidproject.org>
    */
    class DLLExport NexusFileIO
    {
    public:
      /// Default constructor
      NexusFileIO();

      /// Destructor
      ~NexusFileIO() {}

      /// open the nexus file for writing
      int openNexusWrite(const std::string& fileName);
      /// open the nexus file for reading
      int openNexusRead(const std::string& fileName, int& workspaceNumber);
      /// write the header ifon for the Mantid workspace format
      int writeNexusProcessedHeader( const std::string& title);
      /// write sample related data
      int writeNexusProcessedSample( const std::string& title,
							  const boost::shared_ptr<Mantid::API::Sample>& sample);
      /// read sample data
      int readNexusProcessedSample( boost::shared_ptr<Mantid::API::Sample>& sample);
      /// write the workspace data
      int writeNexusProcessedData( const boost::shared_ptr<Mantid::DataObjects::Workspace2D>& localworkspace,
							const bool& uniformSpectra, const int& fromY, const int& toY);
      /// find size of open entry data section
      int getWorkspaceSize( int& numberOfSpectra, int& numberOfChannels, int& numberOfXpoints ,
               bool& uniformBounds, std::string& axesNames, std::string& yUnits );
      /// read X values for one (or the generic if uniform) spectra
      int getXValues(std::vector<double>& xValues, const int& spectra);
      /// read values and erros for spectra
      int getSpectra(std::vector<double>& values, std::vector<double>& errors, const int& spectra);

      /// read the Nexus Processed Data
      bool readNexusProcessedData( boost::shared_ptr<Mantid::DataObjects::Workspace2D>& localworkspace,
							bool& uniformSpectra, int& m_spec_min, int& m_spec_max);
      /// write the algorithm and environment information
      int writeNexusProcessedProcess(const boost::shared_ptr<Mantid::DataObjects::Workspace2D>& localworkspace);
      /// write the source XML file used, if it exists
      bool writeNexusInstrumentXmlName(const std::string& instrumentXml,const std::string& date,
                            const std::string& version);
      /// read the source XML file used
      bool readNexusInstrumentXmlName(std::string& instrumentXml,std::string& date,
                            std::string& version);
      /// write an instrument section - currently only the name
      bool writeNexusInstrument(const boost::shared_ptr<API::IInstrument>& instrument);
      /// write any spectra map information to Nexus file
      bool writeNexusProcessedSpectraMap(const boost::shared_ptr<Mantid::API::SpectraDetectorMap>& spectraMap,
                            const int& m_spec_min, const int& m_spec_max);
      /// read spectra map information
      bool readNexusProcessedSpectraMap(boost::shared_ptr<Mantid::API::SpectraDetectorMap>& spectraMap,
          const boost::shared_ptr<Mantid::API::Workspace> workspace, const int& m_spec_min, const int& m_spec_max);
      /// close the nexus file
      int closeNexusFile();

    private:
      /// Nexus file handle
      NXhandle fileID;
      /// Nexus format to use for writing
      NXaccess m_nexusformat;
      /// Nexus compression method
      int m_nexuscompression;
      /// write a text value plus possible attribute
      bool writeNxText( const std::string& name, const std::string& value, const std::vector<std::string>& attributes,
				 const std::vector<std::string>& avalues);
      /// read a text value plus possible attribute
      bool readNxText(const std::string& name, std::string& value, std::vector<std::string>& attributes,
				 std::vector<std::string>& avalues);
      /// write an NXnote with standard fields (but NX_CHAR rather than NX_BINARY data)
      bool writeNxNote(const std::string& noteName, const std::string& author, const std::string& date,
                         const std::string& description, const std::string& pairValues);
      /// write a flost value plus possible attributes
      bool writeNxFloat(const std::string& name, const double& value, const std::vector<std::string>& attributes,
	                           const std::vector<std::string>& avalues);
      /// read a float value and possible attributes
      bool readNxFloat(const std::string& name, double& value, std::vector<std::string>& attributes,
	                           std::vector<std::string>& avalues);
      bool writeNxFloatArray(const std::string& name, const std::vector<double>& values, const std::vector<std::string>& attributes,
	                           const std::vector<std::string>& avalues);
      /// Write NXlog data for given double TimeSeriesProperty
      void writeNexusDoubleLog(const Kernel::TimeSeriesProperty<double> *d_timeSeries);
      /// Write NXlog data for given string TimeSeriesProperty
      void writeNexusStringLog(const Kernel::TimeSeriesProperty<std::string> *s_timeSeries);
      /// read the named NXlog and create TimeSeriesProp in sample
      void readNXlog(const char* nxname, boost::shared_ptr<Mantid::API::Sample>& sample);
      /// check if the gievn item exists in the current level
      bool checkEntryAtLevel(const std::string& item) const;
      /// write test field
      int writeNexusTextField( const NXhandle& h, const std::string& name, const std::string& value);
      /// search for exisiting MantidWorkpace_n entries in opened file
      int findMantidWSEntries();
      /// convert posix time to time_t
      std::time_t to_time_t(boost::posix_time::ptime t) ///< convert posix time to time_t
      {
			/*!
			Take the input Posix time, subtract the unix epoch, and return the seconds
			as a std::time_t value.
			@param t :: time of interest as ptime
			@return :: time_t value of t
			*/
            if( t == boost::posix_time::neg_infin )
             return 0;
            else if( t == boost::posix_time::pos_infin )
             return LONG_MAX;
            boost::posix_time::ptime start(boost::gregorian::date(1970,1,1));
            return (t-start).total_seconds();
       } 

      /// nexus file name
      std::string m_filename;
      ///static reference to the logger class
      static Kernel::Logger& g_log;

    };

  } // namespace NeXus
} // namespace Mantid

#endif /* NEXUSFILEIO_H */
