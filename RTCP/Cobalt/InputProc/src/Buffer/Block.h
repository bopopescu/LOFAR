/* Block.h
 * Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
 * P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
 *
 * This file is part of the LOFAR software suite.
 * The LOFAR software suite is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The LOFAR software suite is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
 *
 * $Id: $
 */

#ifndef LOFAR_INPUT_PROC_BLOCK_H
#define LOFAR_INPUT_PROC_BLOCK_H

#include <vector>

#include <CoInterface/RSPTimeStamp.h>
#include <CoInterface/SparseSet.h>

namespace LOFAR
{

  namespace Cobalt
  {

    /*
     * Contains information about a block of data to be
     * read from a circular buffer.
     */
    template<typename T>
    struct Block {
      virtual ~Block() {}

      TimeStamp from;
      TimeStamp to;

      struct Beamlet {
        // Actual beamlet number on station
        unsigned stationBeamlet;

        // Copy as one or two ranges of [from, to).
        struct Range {
          const T* from;
          const T* to;
        } ranges[2];

        unsigned nrRanges;

        // The offset at which the data is accessed.
        ssize_t offset;

        SparseSet<int64> flagsAtBegin;
      };

      std::vector<struct Beamlet> beamlets; // [beamlet]

      /*
       * Read the flags for a specific beamlet. Readers should read the flags
       * after reading the data. The valid data is then indicated by
       * the intersection of (beamlets[i].flagsAtBegin & flags(i))
       */
      virtual SparseSet<int64> flags( size_t beamletIdx ) const {
        return beamlets[beamletIdx].flagsAtBegin;
      }
    };

  }

}

#endif
