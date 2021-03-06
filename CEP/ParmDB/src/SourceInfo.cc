//# SourceInfo.cc: Info about a source
//#
//# Copyright (C) 2008
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

// @file
// @brief Info about a source
// @author Ger van Diepen (diepen AT astron nl)

//# Includes
#include <lofar_config.h>

#include <ParmDB/SourceInfo.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobArray.h>

#include <casa/Arrays/Array.h>


namespace LOFAR {
namespace BBS {

  SourceInfo::SourceInfo (const string& name, Type type,
                          const string& refType,
                          uint spectralIndexNTerms,
                          double spectralIndexRefFreqHz,
                          bool useRotationMeasure)
    : itsName           (name),
      itsType           (type),
      itsRefType        (refType),
      itsSpInxNTerms    (spectralIndexNTerms),
      itsSpInxRefFreq   (spectralIndexRefFreqHz),
      itsUseRotMeas     (useRotationMeasure),
      itsShapeletScaleI (0),
      itsShapeletScaleQ (0),
      itsShapeletScaleU (0),
      itsShapeletScaleV (0)
  {}

  SourceInfo::SourceInfo (const SourceInfo& that)
  {
    operator= (that);
  }

  SourceInfo& SourceInfo::operator= (const SourceInfo& that)
  {
    if (this != &that) {
      itsName           = that.itsName;
      itsType           = that.itsType;
      itsRefType        = that.itsRefType;
      itsSpInxNTerms    = that.itsSpInxNTerms;
      itsSpInxRefFreq   = that.itsSpInxRefFreq;
      itsUseRotMeas     = that.itsUseRotMeas;
      itsShapeletScaleI = that.itsShapeletScaleI;
      itsShapeletScaleQ = that.itsShapeletScaleQ;
      itsShapeletScaleU = that.itsShapeletScaleU;
      itsShapeletScaleV = that.itsShapeletScaleV;
      itsShapeletCoeffI.assign (that.itsShapeletCoeffI);
      itsShapeletCoeffQ.assign (that.itsShapeletCoeffQ);
      itsShapeletCoeffU.assign (that.itsShapeletCoeffU);
      itsShapeletCoeffV.assign (that.itsShapeletCoeffV);
    }
    return *this;
  }

  void SourceInfo::setShapeletCoeff (const casa::Array<double>& I,
                                     const casa::Array<double>& Q,
                                     const casa::Array<double>& U,
                                     const casa::Array<double>& V)
  {
    itsShapeletCoeffI.assign (I);
    itsShapeletCoeffQ.assign (Q);
    itsShapeletCoeffU.assign (U);
    itsShapeletCoeffV.assign (V);
  }

  void SourceInfo::setShapeletScale (double scaleI, double scaleQ,
                                     double scaleU, double scaleV)
  {
    itsShapeletScaleI = scaleI;
    itsShapeletScaleQ = scaleQ;
    itsShapeletScaleU = scaleU;
    itsShapeletScaleV = scaleV;
  }

  void SourceInfo::write (BlobOStream& bos) const
  {
    int16 version = 1;
    bos << version << itsName << int16(itsType) << itsRefType
        << itsSpInxNTerms << itsSpInxRefFreq << itsUseRotMeas;
    if (itsType == SHAPELET) {
      bos << itsShapeletScaleI << itsShapeletScaleQ
          << itsShapeletScaleU << itsShapeletScaleV
          << itsShapeletCoeffI << itsShapeletCoeffQ
          << itsShapeletCoeffU << itsShapeletCoeffV;
    }
  }

  // If ever version info is needed, 
  void SourceInfo::read (BlobIStream& bis)
  {
    int16 version, type;
    bis >> version >> itsName >> type >> itsRefType
        >> itsSpInxNTerms >> itsSpInxRefFreq >> itsUseRotMeas;
    ASSERT (version == 1);
    // Convert to enum.
    itsType = Type(type);
    if (itsType == SHAPELET) {
      bis >> itsShapeletScaleI >> itsShapeletScaleQ
          >> itsShapeletScaleU >> itsShapeletScaleV
          >> itsShapeletCoeffI >> itsShapeletCoeffQ
          >> itsShapeletCoeffU >> itsShapeletCoeffV;
    } else {
      itsShapeletScaleI = 0;
      itsShapeletScaleQ = 0;
      itsShapeletScaleU = 0;
      itsShapeletScaleV = 0;
      itsShapeletCoeffI.resize();
      itsShapeletCoeffQ.resize();
      itsShapeletCoeffU.resize();
      itsShapeletCoeffV.resize();
    }
  }

} // namespace BBS
} // namespace LOFAR
