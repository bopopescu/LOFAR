/***************************************************************************
 *   Copyright (C) 2006 by ASTRON, Adriaan Renting                         *
 *   renting@astron.nl                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef __FLAGGER_MS_FILE_H__
#define __FLAGGER_MS_FILE_H__

#include <ms/MeasurementSets.h>
#include <vector>
#include <string>
#include <tables/Tables.h>
#include <tables/Tables/TableIter.h>

namespace LOFAR
{
  class MS_File
  {
  public:
     MS_File(const std::string& msname);
    ~MS_File();

    int                     itsNumSamples;
    int                     itsNumAntennae;
    int                     itsNumFields;
    int                     itsNumBands;
    int                     itsNumChannels;
    int                     itsNumPolarizations;
    int                     itsNumPairs;
    int                     itsNumTimeslots;
    std::vector<casa::String> itsAntennaNames;
    casa::Vector<casa::Int>   itsPolarizations;

    casa::TableIterator TimeslotIterator();
    casa::TableIterator TimeAntennaIterator();
    casa::MSAntenna     antenna();
    void                WriteDataPointFlags(casa::TableIterator* flag_iter,
                                            casa::Matrix<casa::Bool>* Flags,
                                            bool FlagcompleteRow,
                                            bool ExistingFlags);

  protected:
    string                MSName;
    casa::MeasurementSet* MS;
    void init();
  private:
  }; // MS_File
}; // namespace LOFAR

#endif // __FLAGGER_MS_FILE_H__
