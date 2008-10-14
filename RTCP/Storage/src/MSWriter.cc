// MSWriter.cc: interface class for filling a MeasurementSet
//
//  Copyright (C) 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//////////////////////////////////////////////////////////////////////

#include <lofar_config.h>

#if defined HAVE_AIPSPP
#include <Storage/MSWriter.h>
#include <Storage/MSWriterImpl.h>
#include <casa/Exceptions/Error.h>
#include <Common/lofar_iostream.h>
#include <Common/LofarLogger.h>

namespace LOFAR
{
  namespace RTCP
  {
    using namespace casa;

    MSWriter::MSWriter (const char* msName, double startTime, double timeStep,
                        int nChan, int nPol, uint nAntennas, const vector<double>& antPos,
			const vector<string>& storageStationNames,
			uint timesToIntegrate)
      : itsWriter (0)
    {
      ASSERTSTR(antPos.size() == 3*nAntennas, antPos.size() << " == " << 3*nAntennas <<
                "Antenna position vector does not have the right size!");
      try {
        itsWriter = new MSWriterImpl (msName, startTime, timeStep, nChan, nPol, 
                                      nAntennas, antPos, storageStationNames, timesToIntegrate);
      } catch (AipsError x) {
        cerr << "MSWriter exception: " << x.getMesg() << endl;
        exit(0);
      } catch (...) {
        cerr << "Unexpected MSWriter exception while creating MSWriterImpl" << endl;
        exit(0);
      }
    }

    MSWriter::~MSWriter()
    {
      delete itsWriter;
    }

    int MSWriter::addBand (int npolarizations, int nchannels,
                           double refFreq, double chanWidth)
    {
      int res;
      try {
        res = itsWriter->addBand (npolarizations, nchannels, refFreq, chanWidth);
      } catch (AipsError x) {
        cerr << "AIPS exception in MSWriterImpl: " << x.getMesg() << endl;
        exit(0);
      } catch (...) {
        cerr << "Unexpected MSWriter exception during addBand" << endl;
        exit(0);
      }
      return res;
    }

    int MSWriter::addBand (int npolarizations, int nchannels,
                           double refFreq, const double* chanFreqs,
                           const double* chanWidths)
    {
      int res;
      try {
        res = itsWriter->addBand (npolarizations, nchannels, refFreq,
                                  chanFreqs, chanWidths);
      } catch (AipsError x) {
        cerr << "AIPS exception in MSWriterImpl: " << x.getMesg() << endl;
        exit(0);
      } catch (...) {
        cerr << "Unexpected MSWriter exception during addBand" << endl;
        exit(0);
      }
      return res;
    }

    void MSWriter::addField (double RA, double DEC, unsigned beamIndex)
    {
      try {
        itsWriter->addField (RA, DEC, beamIndex);
      } catch (AipsError x) {
        cerr << "AIPS exception in MSWriterImpl: " << x.getMesg() << endl;
        exit(0);
      } catch (...) {
        cerr << "Unexpected MSWriter exception during addField" << endl;
        exit(0);
      }
    }

    void MSWriter::write (int bandId, int channelId, 
                          int nrChannels, int timeCounter, int nrdata, 
                          const fcomplex* data, const bool* flags,
                          const float* weights)
    {
      try {
        itsWriter->write (bandId, channelId, nrChannels, timeCounter,
                          nrdata, data, flags, weights);
      } catch (AipsError x) {
        cerr << "AIPS exception in MSWriterImpl: " << x.getMesg() << endl;
        exit(0);
      } catch (std::exception &ex)  {
        cerr << "Unexpected MSWriter exception during write:" << ex.what() << endl;
	perror("perror returns: ");
        exit(0);
      }
    }

    int MSWriter::nrAntennas() const
    { return itsWriter->nrAntennas(); }

    int MSWriter::nrBands() const
    { return itsWriter->nrBands(); }

    int MSWriter::nrFields() const
    { return itsWriter->nrFields(); }

    int MSWriter::nrPolarizations() const
    { return itsWriter->nrPolarizations(); }

    int MSWriter::nrTimes() const
    { return itsWriter->nrTimes(); }


  } // namespace RTCP

} // namespace LOFAR

#endif // defined HAVE_AIPSPP
