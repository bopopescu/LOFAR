//#  MeqVocabulary.h: provide some standard field names
//#
//#  Copyright (C) 2002-2003
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

#ifndef MEQ_VOCABULARY_H
#define MEQ_VOCABULARY_H

#include <MEQ/AID-Meq.h>


#pragma aidgroup Meq
#pragma aid Node Class Name State Child Children Request Result Rider Id
#pragma aid Cells Domain Freq Time Times Step Steps Calc Deriv
#pragma aid Table Name Default Value Index Num
#pragma aid Parm Spid Index Perturbed Perturbations
#pragma aid Cells Results Fail Origin Line Message Contagious 
#pragma aid Solvable

namespace Meq
{
  const HIID 
      
    FRequestId       = AidRequest|AidId,
    FCells           = AidCells,
    FCalcDeriv       = AidCalc|AidDeriv,
    FRider           = AidRider,
    FNodeName        = AidNode|AidName,
    FNodeState       = AidNode|AidState,
    FClass           = AidClass,
    FClassName       = AidClass|AidName,
    FResults         = AidResults,
    FResult          = AidResult,
    FChildren        = AidChildren,
    FName            = AidName,

    FDomain          = AidDomain,
    FTimes           = AidTimes,
    FTimeSteps       = AidTime|AidSteps,
    FNumFreq         = AidNum|AidFreq,
    
    FDefault         = AidDefault,
    FTableName       = AidTable|AidName,
    
    FValue           = AidValue,
    FSpids           = AidSpid|AidIndex,
    FPerturbedValues = AidPerturbed|AidValue,
    FPerturbations   = AidPerturbations,

    FFail            = AidFail,
    FOrigin          = AidOrigin,
    FOriginLine      = AidOrigin|AidLine,
    FMessage         = AidMessage,
    
    FSolvable        = AidSolvable,
    FSolvableParm    = AidSolvable|AidParm,
    FNumSteps        = AidNum|AidSteps,
    FEpsilon         = FNumSteps,
    FUseSVD          = FNumSteps,

    FContagiousFail  = AidContagious|AidFail,
    
    FIndex           = AidIndex;
    

};

#endif
