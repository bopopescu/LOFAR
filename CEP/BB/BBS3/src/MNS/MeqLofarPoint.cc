//# MeqLofarPoint.cc: The total baseline prediction of point sources in the LOFAR model
//#
//# Copyright (C) 2003
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

#include <lofar_config.h>
#include <Common/Profiling/PerfProfile.h>

#include <BBS3/MNS/MeqLofarPoint.h>
#include <BBS3/MNS/MeqLofarStatSources.h>
#include <BBS3/MNS/MeqRequest.h>
#include <BBS3/MNS/MeqResult.h>
#include <BBS3/MNS/MeqHist.h>
#include <BBS3/MNS/MeqMatrix.h>
#include <BBS3/MNS/MeqMatrixTmp.h>
#include <Common/LofarLogger.h>
#include <casa/Arrays/Matrix.h>

using namespace casa;

namespace LOFAR {

MeqLofarPoint::MeqLofarPoint (MeqSourceList* sources,
			      MeqLofarStatSources* left,
			      MeqLofarStatSources* right,
			      MeqHist* celltHistogram, MeqHist* cellfHistogram)
: itsSources   (sources),
  itsLeft      (left),
  itsRight     (right),
  itsCelltHist (celltHistogram),
  itsCellfHist (cellfHistogram)
{}

MeqLofarPoint::~MeqLofarPoint()
{}


// moved this loop out of MeqLofarPoint::calcResult() to circumvent icc
// compiler bug

static void doCalcResult(
  double s1, dcomplex s2, dcomplex s3, double s4,
  const dcomplex *l11, const dcomplex *l12,
  const dcomplex *l21, const dcomplex *l22,
  const dcomplex *r11, const dcomplex *r12,
  const dcomplex *r21, const dcomplex *r22,
  dcomplex *dxx, dcomplex *dxy,
  dcomplex *dyx, dcomplex *dyy, Int ncell
)
{
  // Multiply some matrices.
  // Outer product  A = L x R*   and then  AM   where
  //   M = S (I Q U V) = (s1 s2 s3 s4)
  // For linear polarisation
  //   S = 1  1  0  0
  //       0  0  1  i
  //       0  0  1 -i
  //       1 -1  0  0
  // so in total:   (l11 l12)   (r11 r12) (s1)
  //                (       ) x (       ) (s2)
  //                (l21 l22)   (r21 r22) (s3)
  //                                      (s4)
  //            =   (l11r11 l11r12 l12r11 l12r12) (s1)
  //                (l11r21 l11r22 l12r21 l12r22) (s2)
  //                (l21r11 l21r12 l22r11 l22r12) (s3)
  //                (l21r21 l21r22 l22r21 l22r22) (s4)
  for (int i=0; i<ncell; i++) {
    // Possible make bit faster by having conjugate of s2 and s3
    // and using: sf11 = (conj(s1*r11[i] + s2*r12[i]); etc.
    // because conj(a*b) = conj(a)*conj(b)
    dcomplex cr11 = conj(r11[i]);
    dcomplex cr12 = conj(r12[i]);
    dcomplex cr21 = conj(r21[i]);
    dcomplex cr22 = conj(r22[i]);
    dcomplex sf11 = s1*cr11 + s2*cr12;
    dcomplex sf12 = s1*cr21 + s2*cr22;
    dcomplex sf21 = s3*cr11 + s4*cr12;
    dcomplex sf22 = s3*cr21 + s4*cr22;
    dxx[i] = l11[i]*sf11 + l12[i]*sf21;
    dxy[i] = l11[i]*sf12 + l12[i]*sf22;
    dyx[i] = l21[i]*sf11 + l22[i]*sf21;
    dyy[i] = l21[i]*sf12 + l22[i]*sf22;
    //    dcomplex sf11 = s1*l11 + s3*l12;
    //    dcomplex sf12 = s1*l21 + s3*l22;
    //    dcomplex sf21 = s2*l11 + s4*l12;
    //    dcomplex sf22 = s2*l21 + s4*l22;
    //    dxx[i] = sf11*conj(r11[i]) + sf21*conj(r12[i]);
    //    dxy[i] = sf11*conj(r21[i]) + sf21*conj(r22[i]);
    //    dyx[i] = sf12*conj(r11[i]) + sf22*conj(r12[i]);
    //    dyy[i] = sf12*conj(r21[i]) + sf22*conj(r22[i]);
  }
}


void MeqLofarPoint::calcResult (const MeqRequest& request)
{
  PERFPROFILE_L(__PRETTY_FUNCTION__, PP_LEVEL_1);

  // We can only calculate for a single time bin.
  const MeqDomain& domain = request.domain();
  ASSERT (request.ny() == 1);
  // Find the maximum nr of cells needed.
  // Use 1 cell only until more cells are needed.
  int ncellt = 1;
  int ncellf = 1;
//   for (unsigned int i=0; i<itsSources.size(); i++) {
//     vector<int> ncell = itsDFT->ncells (i, request);
//     if (ncell[0] > ncellt) {
//       ncellt = ncell[0];
//     }
//     if (ncell[1] > ncellf) {
//       ncellf = ncell[1];
//     }
//   }
  itsNcell.resize (2);
  itsNcell[0] = ncellt;
  itsNcell[1] = ncellf;
  itsCelltHist->update (ncellt);
  itsCellfHist->update (ncellf);
  ncellf *= request.nx();

  // The domain is divided into the required number of cells.
  MeqRequest dftReq (domain, ncellf, ncellt, request.nspid());
  Int ncell = ncellt*ncellf;
  MeqResult& resXX = result11();
  MeqResult& resXY = result12();
  MeqResult& resYX = result21();
  MeqResult& resYY = result22();
  resXX = MeqResult(request.nspid());
  resXY = MeqResult(request.nspid());
  resYX = MeqResult(request.nspid());
  resYY = MeqResult(request.nspid());
  Matrix<dcomplex> value(ncellt, ncellf, makedcomplex(0,0));
  resXX.setValue (MeqMatrix (value));
  resXY.setValue (MeqMatrix (value));
  resYX.setValue (MeqMatrix (value));
  resYY.setValue (MeqMatrix (value));
  // Allocate matrices to hold the result of a single source.
  MeqMatrix xx, xy, yx, yy;
  dcomplex* dxx = xx.setDComplex (ncellt, ncellf);
  dcomplex* dxy = xy.setDComplex (ncellt, ncellf);
  dcomplex* dyx = yx.setDComplex (ncellt, ncellf);
  dcomplex* dyy = yy.setDComplex (ncellt, ncellf);

  // Step through all sources and calculate the contribution for each of them.
  int nrsrc = itsSources->size();
  for (int srcnr=0; srcnr<nrsrc; srcnr++) {
    dftReq.setSourceNr (srcnr);
    MeqPointSource& src = (*itsSources)[srcnr];
    // Calculate the source fluxes.
    MeqResult ik = src.getI()->getResult (dftReq);
    MeqResult qk = src.getQ()->getResult (dftReq);
    MeqResult uk = src.getU()->getResult (dftReq);
    MeqResult vk = src.getV()->getResult (dftReq);
    const MeqResult& nk = itsLeft->getN (dftReq);
    // Calculate the left and right station Jones matrix elements.
    const MeqResult& resl11  = itsLeft->getResult11 (dftReq);
    const MeqResult& resl12  = itsLeft->getResult12 (dftReq);
    const MeqResult& resl21  = itsLeft->getResult21 (dftReq);
    const MeqResult& resl22  = itsLeft->getResult22 (dftReq);
    const MeqResult& resr11  = itsRight->getResult11 (dftReq);
    const MeqResult& resr12  = itsRight->getResult12 (dftReq);
    const MeqResult& resr21  = itsRight->getResult21 (dftReq);
    const MeqResult& resr22  = itsRight->getResult22 (dftReq);
    // Get the source fluxes and calculate the Stokes values from them.
    // We assume (and check) that the fluxes are independent of frequency.
    ASSERT (ik.getValue().nelements() == 1);
    ASSERT (qk.getValue().nelements() == 1);
    ASSERT (uk.getValue().nelements() == 1);
    ASSERT (vk.getValue().nelements() == 1);
    ASSERT (nk.getValue().nelements() == 1);
    double ival = ik.getValue().getDouble();
    double qval = qk.getValue().getDouble();
    double uval = uk.getValue().getDouble();
    double vval = vk.getValue().getDouble();
    double fact = 1 / (2*nk.getValue().getDouble());
    // For linear polarisation the S matrix is as follows.
    double s1 = (ival + qval)  * fact;
    dcomplex s2 = makedcomplex(uval*fact, vval*fact);
    dcomplex s3 = LOFAR::conj(s2);
    double s4 = (ival - qval) * fact;
    // Get pointers to storage of the left and right station Jones elements.
    const dcomplex* l11 = resl11.getValue().dcomplexStorage();
    const dcomplex* l12 = resl12.getValue().dcomplexStorage();
    const dcomplex* l21 = resl21.getValue().dcomplexStorage();
    const dcomplex* l22 = resl22.getValue().dcomplexStorage();
    const dcomplex* r11 = resr11.getValue().dcomplexStorage();
    const dcomplex* r12 = resr12.getValue().dcomplexStorage();
    const dcomplex* r21 = resr21.getValue().dcomplexStorage();
    const dcomplex* r22 = resr22.getValue().dcomplexStorage();
    // Calculate XX, etc.
    doCalcResult(s1,s2,s3,s4,l11,l12,l21,l22,r11,r12,r21,r22,dxx,dxy,dyx,dyy,ncell);
    ///    cout << "MeqLofarPoint " << srcnr << ' ' << resl11.getValue()
    ///	 << resr11.getValue() << xx << endl;
    // Evaluate (if needed) for the perturbed parameter values.
    MeqMatrix perturbation;
    for (int spinx=0; spinx<request.nspid(); spinx++) {
      // See if a source flux parameter is perturbed.
      // If so, calculate the relevant perturbed Stokes values.
      bool evaliq = false;
      bool evaluv = false;
      if (ik.isDefined(spinx)) {
	evaliq = true;
	perturbation = ik.getPerturbation(spinx);
      } else if (qk.isDefined(spinx)) {
	evaliq = true;
	perturbation = qk.getPerturbation(spinx);
      }
      if (uk.isDefined(spinx)) {
	evaluv = true;
	perturbation = uk.getPerturbation(spinx);
      } else if (vk.isDefined(spinx)) {
	evaluv = true;
	perturbation = vk.getPerturbation(spinx);
      }
      double ps1 = s1;
      double ps4 = s4;
      dcomplex ps2 = s2;
      dcomplex ps3 = s3;
      double pfact = fact;
      if (nk.isDefined(spinx)) {
	pfact = 1 / (2*nk.getPerturbedValue(spinx).getDouble());
	evaliq = true;
	evaluv = true;
      }
      bool eval = false;
      if (evaliq) {
	eval = true;
	double ival = ik.getPerturbedValue(spinx).getDouble();
	double qval = qk.getPerturbedValue(spinx).getDouble();
	ps1 = (ival + qval) * pfact;
	ps4 = (ival - qval) * pfact;
      }
      if (evaluv) {
	eval = true;
	double uval = uk.getPerturbedValue(spinx).getDouble();
	double vval = vk.getPerturbedValue(spinx).getDouble();
	ps2 = makedcomplex(uval*pfact, vval*pfact);
	ps3 = LOFAR::conj(ps2);
      }
      // See if an element in the station Jones is perturbed.
      // If so, determine which value to recalculate.
      bool evalxx = false;
      bool evalxy = false;
      bool evalyx = false;
      bool evalyy = false;
      if (resl11.isDefined(spinx)) {
	evalxx = true;
	evalxy = true;
	perturbation = resl11.getPerturbation(spinx);
      } else if (resl12.isDefined(spinx)) {
	evalxx = true;
	evalxy = true;
	perturbation = resl12.getPerturbation(spinx);
      }
      if (resl21.isDefined(spinx)) {
	evalyx = true;
	evalyy = true;
	perturbation = resl21.getPerturbation(spinx);
      } else if (resl22.isDefined(spinx)) {
	evalyx = true;
	evalyy = true;
	perturbation = resl22.getPerturbation(spinx);
      }
      if (resr11.isDefined(spinx)) {
	evalxx = true;
	evalyx = true;
	perturbation = resr11.getPerturbation(spinx);
      } else if (resr12.isDefined(spinx)) {
	evalxx = true;
	evalyx = true;
	perturbation = resr12.getPerturbation(spinx);
      }
      if (resr21.isDefined(spinx)) {
	evalxy = true;
	evalyy = true;
	perturbation = resr21.getPerturbation(spinx);
      } else if (resr22.isDefined(spinx)) {
	evalxy = true;
	evalyy = true;
	perturbation = resr22.getPerturbation(spinx);
      }
      // If a source flux is perturbed, all values have to be recalculated.
      if (eval) {
	evalxx = true;
	evalxy = true;
	evalyx = true;
	evalyy = true;
      }
      // Recalculate xx, xy, yx, and yy as needed.
      if (evalxx || evalxy || evalyx || evalyy) {
	MeqMatrix pxx, pxy, pyx, pyy;
	dcomplex* dpxx = 0;
	dcomplex* dpxy = 0;
	dcomplex* dpyx = 0;
	dcomplex* dpyy = 0;
	// Allocate the matrices and get pointers to their data.
	// If a perturbed value has not been calculated before
	// (for another source), initialize it to the unperturbed sum.
	if (evalxx) {
	  dpxx = pxx.setDComplex (ncellt, ncellf);
	  if (! resXX.isDefined(spinx)) {
	    resXX.setPerturbedValue (spinx, resXX.getValue().clone());
	    resXX.setPerturbation (spinx, perturbation);
	  }
	}
	if (evalxy) {
	  dpxy = pxy.setDComplex (ncellt, ncellf);
	  if (! resXY.isDefined(spinx)) {
	    resXY.setPerturbedValue (spinx, resXY.getValue().clone());
	    resXY.setPerturbation (spinx, perturbation);
	  }
	}
	if (evalyx) {
	  dpyx = pyx.setDComplex (ncellt, ncellf);
	  if (! resYX.isDefined(spinx)) {
	    resYX.setPerturbedValue (spinx, resYX.getValue().clone());
	    resYX.setPerturbation (spinx, perturbation);
	  }
	}
	if (evalyy) {
	  dpyy = pyy.setDComplex (ncellt, ncellf);
	  if (! resYY.isDefined(spinx)) {
	    resYY.setPerturbedValue (spinx, resYY.getValue().clone());
	    resYY.setPerturbation (spinx, perturbation);
	  }
	}
	// Get pointers the data of the stations Jones elements.
	l11 = resl11.getPerturbedValue(spinx).dcomplexStorage();
	l12 = resl12.getPerturbedValue(spinx).dcomplexStorage();
	l21 = resl21.getPerturbedValue(spinx).dcomplexStorage();
	l22 = resl22.getPerturbedValue(spinx).dcomplexStorage();
	r11 = resr11.getPerturbedValue(spinx).dcomplexStorage();
	r12 = resr12.getPerturbedValue(spinx).dcomplexStorage();
	r21 = resr21.getPerturbedValue(spinx).dcomplexStorage();
	r22 = resr22.getPerturbedValue(spinx).dcomplexStorage();
	// Do the calculations as efficient as possible by sharing
	// calculations, but not having if-s in the tight loops.
	// It means a bit more coding.
	if (evalxx && evalyx) {
	  for (int i=0; i<ncell; i++) {
	    dcomplex sf11 = ps1*LOFAR::conj(r11[i]) + ps2*LOFAR::conj(r12[i]);
	    dcomplex sf21 = ps3*LOFAR::conj(r11[i]) + ps4*LOFAR::conj(r12[i]);
	    dpxx[i] = l11[i]*sf11 + l12[i]*sf21;
	    dpyx[i] = l21[i]*sf11 + l22[i]*sf21;
	  }
	  resXX.getPerturbedValueRW(spinx) += pxx;
	  resYX.getPerturbedValueRW(spinx) += pyx;
	} else if (evalxx) {
	  for (int i=0; i<ncell; i++) {
	    dcomplex sf11 = ps1*LOFAR::conj(r11[i]) + ps2*LOFAR::conj(r12[i]);
	    dcomplex sf21 = ps3*LOFAR::conj(r11[i]) + ps4*LOFAR::conj(r12[i]);
	    dpxx[i] = l11[i]*sf11 + l12[i]*sf21;
	  }
	  resXX.getPerturbedValueRW(spinx) += pxx;
	} else if (evalyx) {
	  for (int i=0; i<ncell; i++) {
	    dcomplex sf11 = ps1*LOFAR::conj(r11[i]) + ps2*LOFAR::conj(r12[i]);
	    dcomplex sf21 = ps3*LOFAR::conj(r11[i]) + ps4*LOFAR::conj(r12[i]);
	    dpyx[i] = l21[i]*sf11 + l22[i]*sf21;
	  }
	  resYX.getPerturbedValueRW(spinx) += pyx;
	}
	if (evalxy && evalyy) {
	  for (int i=0; i<ncell; i++) {
	    dcomplex sf12 = ps1*LOFAR::conj(r21[i]) + ps2*LOFAR::conj(r22[i]);
	    dcomplex sf22 = ps3*LOFAR::conj(r21[i]) + ps4*LOFAR::conj(r22[i]);
	    dpxy[i] = l11[i]*sf12 + l12[i]*sf22;
	    dpyy[i] = l21[i]*sf12 + l22[i]*sf22;
	  }
	  resXY.getPerturbedValueRW(spinx) += pxy;
	  resYY.getPerturbedValueRW(spinx) += pyy;
	} else if (evalxy) {
	  for (int i=0; i<ncell; i++) {
	    dcomplex sf12 = ps1*LOFAR::conj(r21[i]) + ps2*LOFAR::conj(r22[i]);
	    dcomplex sf22 = ps3*LOFAR::conj(r21[i]) + ps4*LOFAR::conj(r22[i]);
	    dpxy[i] = l11[i]*sf12 + l12[i]*sf22;
	  }
	  resXY.getPerturbedValueRW(spinx) += pxy;
	} else if (evalyy) {
	  for (int i=0; i<ncell; i++) {
	    dcomplex sf12 = ps1*LOFAR::conj(r21[i]) + ps2*LOFAR::conj(r22[i]);
	    dcomplex sf22 = ps3*LOFAR::conj(r21[i]) + ps4*LOFAR::conj(r22[i]);
	    dpyy[i] = l21[i]*sf12 + l22[i]*sf22;
	  }
	  resYY.getPerturbedValueRW(spinx) += pyy;
	}
      }
      // See if there is a perturbed values in result for this parameter.
      // Add unperturbed value if previous results were perturbed.
      if (!evalxx  &&  resXX.isDefined(spinx)) {
	resXX.getPerturbedValueRW(spinx) += xx;
      }
      if (!evalxy  &&  resXY.isDefined(spinx)) {
	resXY.getPerturbedValueRW(spinx) += xy;
      }
      if (!evalyx  &&  resYX.isDefined(spinx)) {
	resYX.getPerturbedValueRW(spinx) += yx;
      }
      if (!evalyy  &&  resYY.isDefined(spinx)) {
	resYY.getPerturbedValueRW(spinx) += yy;
      }
//       cout << srcnr << ' ' << spinx
// 	   << ' ' << evalxx << evalxy << evalyx << evalyy
// 	   << ' ' << resXX.isDefined(spinx) << resXY.isDefined(spinx)
// 	   << ' ' << resYX.isDefined(spinx) << resYY.isDefined(spinx)
// 	   << endl;
    }

    // Now add the source contribution to the unperturbed value.
    resXX.getValueRW() += xx;
    resXY.getValueRW() += xy;
    resYX.getValueRW() += yx;
    resYY.getValueRW() += yy;
  }
   for (int spinx=0; spinx<request.nspid(); spinx++) {
//      cout << "XX" << resXX.isDefined(spinx)
//  	 << ' ' << resXX.getPerturbedValue(spinx)
//  	 << "XY" << resXY.isDefined(spinx)
//  	 << ' ' << resXY.getPerturbedValue(spinx)
//  	 << "YX" << resYX.isDefined(spinx)
//  	 << ' ' << resYX.getPerturbedValue(spinx)
//  	 << "YY" << resYY.isDefined(spinx)
//  	 << ' ' << resYY.getPerturbedValue(spinx)
//  	 << endl;
   }

}

}
