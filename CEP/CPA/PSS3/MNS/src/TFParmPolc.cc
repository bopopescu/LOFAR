//# TFParmPolc.cc: Polynomial coefficients
//#
//# Copyright (C) 2002
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#include <MNS/TFParmPolc.h>
#include <MNS/TFRequest.h>
#include <Common/Debug.h>
#include <aips/Arrays/Matrix.h>

TFParmPolc::TFParmPolc (unsigned int type, 
			unsigned int orderT, unsigned int orderF)
: TFParm      (type),
  itsNx       (orderT+1),
  itsNy       (orderF+1),
  itsCurCoeff ((orderT+1)*(orderF+1), 1),
  itsMask     ((orderT+1)*(orderF+1), true)
{
  itsInitialCoeff = itsCurCoeff;
}

TFParmPolc::TFParmPolc (unsigned int type, const Matrix<double>& values)
: TFParm  (type),
  itsNx   (values.shape()(0)),
  itsNy   (values.shape()(1)),
  itsMask (values.nelements(), true)
{
  itsCurCoeff.reserve (values.nelements());
  bool deleteD;
  const double* vdata = values.getStorage(deleteD);
  for (unsigned int i=0; i<values.nelements(); i++) {
    itsCurCoeff[i] = vdata[i];
  }
  values.freeStorage (vdata, deleteD);
  itsInitialCoeff = itsCurCoeff;
}

TFParmPolc::TFParmPolc (unsigned int type, const Matrix<double>& values,
			const Matrix<bool>& mask)
: TFParm (type),
  itsNx  (values.shape()(0)),
  itsNy  (values.shape()(1))
{
  Assert (values.shape().isEqual (mask.shape()));
  itsCurCoeff.reserve (values.nelements());
  itsMask.reserve (values.nelements());
  bool deleteD, deleteM;
  const double* vdata = values.getStorage(deleteD);
  const bool* mdata = mask.getStorage(deleteM);
  for (unsigned int i=0; i<values.nelements(); i++) {
    itsCurCoeff[i] = vdata[i];
    itsMask[i]         = mdata[i];
  }
  values.freeStorage (vdata, deleteD);
  mask.freeStorage (mdata, deleteM);
  itsInitialCoeff = itsCurCoeff;
}


int TFParmPolc::setSolvable (int spidIndex)
{
  itsPerturbedCoeff = itsCurCoeff;
  itsSpidInx.reserve (itsCurCoeff.size());
  int nr=0;
  for (unsigned int i=0; i<itsCurCoeff.size(); i++) {
    if (itsMask[i]) {
      itsSpidInx[i] = spidIndex++;
      nr++;
    } else {
      itsSpidInx[i] = -1;          // not solvable
    }
  }
  return nr;
}


TFRange TFParmPolc::getRange (const TFRequest& request)
{
  const TFDomain& domain = request.domain();
  int ndx = domain.nx();
  int ndy = domain.ny();
  vector<TFRange> coeffRange(itsNx*itsNy);
  // Evaluate the expression.
  TFRange range(request.nspid());
  Matrix<double> result(ndx, ndy);
  for (int j=0; j<ndy; j++) {
    for (int i=0; i<ndx; i++) {
      double tmp = 0;
      const MnsMatrix& xterm = request.getCrossTerms (i, j);
      int inx = 0;
      for (int cy=0; cy<itsNy; cy++) {
	for (int cx=0; cx<itsNx; cx++) {
	  tmp += itsCurCoeff[inx++] * xterm.getDouble(cx,cy);
	}
      }
      result(i,j) = tmp;
    }
  }
  range.setValue (result);
  // Evaluate (if needed) for the perturbed parameter values.
  vector<MnsMatrix*> valptr(itsCurCoeff.size());
  for (unsigned int spinx=0; spinx<itsSpidInx.size(); spinx++) {
    if (itsSpidInx[spinx] >= 0) {
      double perturbation = 1e-6;
      if (abs(itsCurCoeff[spinx]) > 1e-10) {
	perturbation = itsCurCoeff[spinx] * 1e-6;
      }
      itsPerturbedCoeff[spinx] = itsCurCoeff[spinx] + perturbation;
      for (int j=0; j<ndy; j++) {
	for (int i=0; i<ndx; i++) {
	  double tmp = 0;
	  const MnsMatrix& xterm = request.getCrossTerms (i, j);
	  int inx = 0;
	  for (int cy=0; cy<itsNy; cy++) {
	    for (int cx=0; cx<itsNx; cx++) {
	      tmp += itsPerturbedCoeff[inx++] * xterm.getDouble(cx,cy);
	    }
	  }
	  result(i,j) = tmp;
	}
      }
      range.setPerturbedValue (spinx, result);
      range.setPerturbation (spinx, perturbation);
      itsPerturbedCoeff[spinx] = itsCurCoeff[spinx];
    }
  }
  return range;
}

void TFParmPolc::update (const MnsMatrix& value)
{
  for (unsigned int i=0; i<itsSpidInx.size(); i++) {
    if (itsSpidInx[i] >= 0) {
      itsCurCoeff[i] = value.getDouble (itsSpidInx[i], 1);
    }
  }
}

void TFParmPolc::save (const TFDomain&)
{
  itsInitialCoeff = itsCurCoeff;
}
