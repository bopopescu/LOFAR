//#  WH_BeamFormer.h:
//#
//#  Copyright (C) 2002
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
//#
//#  Chris Broekema, january 2003
//#


#ifndef STATIONSIM_WH_BEAMFORMER_H
#define STATIONSIM_WH_BEAMFORMER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <BaseSim/WorkHolder.h>
#include <Common/lofar_vector.h>
#include <Common/Lorrays.h>
#include <StationSim/DH_SampleC.h>
#include <StationSim/GnuPlotInterface.h>

class WH_BeamFormer: public WorkHolder
{
public:
  /// Construct the work holder and give it a name.
  /// It is possible to specify how many input and output data holders
  /// are created and how many elements there are in the buffer.
  /// The first WorkHolder should have nin=0.
  WH_BeamFormer (const string& name,
				 unsigned int nin, unsigned int nout, unsigned int nrcu,
				 unsigned int nsubband, bool tapstream);
		 

  virtual ~WH_BeamFormer();

  /// Static function to create an object.
  static WorkHolder* construct (const string& name, int ninput, int noutput, 
								const ParamBlock&);

  /// Make a fresh copy of the WH object.
  virtual WH_BeamFormer* make (const string& name) const;

  /// Do a process step.
  virtual void process();

  /// Show the work holder on stdout.
  virtual void dump() const;

  /// Get a pointer to the i-th input DataHolder.
  /// The first one is the sampled data.
  /// The second one is the selected subbands.
  virtual DataHolder* getInHolder (int channel);

  /// Get a pointer to the i-th output DataHolder.
  virtual DataHolder* getOutHolder (int channel);

private:
  /// Forbid copy constructor.
  WH_BeamFormer (const WH_BeamFormer&);

  /// Forbid assignment.
  WH_BeamFormer& operator= (const WH_BeamFormer&);

  void beamplot (gnuplot_ctrl* handle, const LoVec_dcomplex& w, 
		 const int ncru);

  void spectrumplot (gnuplot_ctrl* handle, const LoVec_dcomplex& look_buf, 
		     const LoVec_dcomplex& null_buf, const int pos);

  void ml_trans_edge (gnuplot_ctrl* handle, const LoVec_dcomplex& w, LoVec_dcomplex& ref, 
		      const int time, const int nrcu, const int N, LoMat_double& x_sum);

  LoMat_double beam_pattern (const LoVec_dcomplex& w, const int nrcu, const int N);

  DH_SampleC** itsInHolders;
  DH_SampleC** itsOutHolders;
  DH_SampleC   itsWeight; 
 
  int itsNrcu;       // Number of (active) antennas
  int itsNsubband;   // Number of subbands
  LoMat_dcomplex itsW;
  bool itsTapStream;
  int itsCount;
};

#endif
