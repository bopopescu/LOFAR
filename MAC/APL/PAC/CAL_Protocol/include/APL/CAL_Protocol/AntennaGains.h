//#  -*- mode: c++ -*-
//#  AntennaGains.h: class definition of the AntennaGains class which reprsentents
//#               the calibration of a subarray
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef ANTENNAGAINS_H_
#define ANTENNAGAINS_H_

#include <math.h>
#include <blitz/array.h>
#ifdef USE_CAL_THREAD
#include <pthread.h>
#endif

namespace LOFAR {
  namespace CAL {

    /**
     * This class holds the results of a remote station calibration: the
     * calibrated antenna gains. Along with the gains a quality measure
     * is computed by the calibration algorithm. This quality measure indicates
     * the confidence in the computed antenna gain.
     */
    class AntennaGains
    {
    public:

      /*@{*/
      /**
       * Constructors
       */
      AntennaGains();
      AntennaGains(int nantennas, int npol, int nsubbands);
      /*@}*/

      /**
       * Destructor
       */
      virtual ~AntennaGains();

      /**
       * Get reference to the array with calibrated antenna gains.
       * @return a reference to the calibrated gains. A three dimensional array of
       * complex doubles with dimensions: nantennas x npol x nsubbands
       */
      const blitz::Array<std::complex<double>, 3>& getGains() const { return m_gains; }

      /**
       * Get reference to the array with quality measure.
       * @return a reference to the quality measure array. A 3-dimensional array
       * of doubles with nantennas x npol x nsubbands elements.
       */
      const blitz::Array<double, 3>& getQuality() const { return m_quality; }

      /**
       * has the calibration algorithm producing this result completed?
       */
      inline bool isDone() const { bool done; lock(); done = m_done; unlock(); return done; }

      /**
       * set the complete status.
       */
      inline void setDone(bool value = true) { lock(); m_done = value; unlock(); }

      /**
       * assignment operator
       * @param rhs Right-hand-side of assignment. It is not const
       * because the rhs must be locked during the assignment.
       */
      AntennaGains& operator=(const AntennaGains& rhs);

      /**
       * lock/unlock
       */
#ifdef USE_CAL_THREAD
      inline int lock()   const { return pthread_mutex_lock((pthread_mutex_t*)m_mutex);   }
      inline int unlock() const { return pthread_mutex_unlock((pthread_mutex_t*)m_mutex); }
#else
      inline int lock()   const { return 0; }
      inline int unlock() const { return 0; }
#endif

    public:
      /*@{*/
      /**
       * marshalling methods
       */
      unsigned int getSize();
      unsigned int pack   (void* buffer);
      unsigned int unpack (void* buffer);
      /*@}*/

    private:
      /**
       * Prevent copy constructor
       */
      AntennaGains(const AntennaGains & copy);

    private:
      blitz::Array<std::complex<double>, 3> m_gains;
      blitz::Array<double, 3>               m_quality;
      bool                                  m_done; // has the calibration finished

#ifdef USE_CAL_THREAD
      const pthread_mutex_t* m_mutex; // control access to the m_done flag
#endif
    };

  }; // namespace CAL
}; // namespace LOFAR

#endif /* CALIBRATIONRESULT_H_ */

