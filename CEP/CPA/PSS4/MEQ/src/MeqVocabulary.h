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
#pragma aid Node Class Name State Child Children Request Result VellSet Rider Id
#pragma aid Cells Domain Freq Time Times Step Steps Calc Deriv Vells VellSets
#pragma aid NodeIndex Table Name Default Value Index Num Cache Code
#pragma aid Parm Spid Perturbed Perturbations Names Pert Relative Mask
#pragma aid Cells Results Fail Origin Line Message Contagious  Normalized
#pragma aid Solvable Config Groups All By List Polcs
#pragma aid Epsilon UseSVD
#pragma aid Metrics Rank Fit Errors CoVar Flag Mu StdDev Chi


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
    FVellSets        = AidVellSets,
    FResult          = AidResult,
    FChildren        = AidChildren,
    FChildrenNames   = AidChildren|AidName,
    FName            = AidName,
    FNodeIndex       = AidNodeIndex,
    
    FConfigGroups    = AidConfig|AidGroups,
    FAll             = AidAll,
    FByNodeIndex     = AidBy|AidNodeIndex,
    FByList          = AidBy|AidList,
    FState           = AidState,
    
    FCacheResult     = AidCache|AidResult,
    FCacheResultCode = AidCache|AidResult|AidCode,
    
    FDomain          = AidDomain,
    FTimes           = AidTimes,
    FTimeSteps       = AidTime|AidSteps,
    FNumFreq         = AidNum|AidFreq,
    
    FDefault         = AidDefault,
    FValue           = AidValue,
    FTableName       = AidTable|AidName,
    FParmName        = AidParm|AidName,
    FDomainId        = AidDomain|AidId,
    
    FPertRelative    = AidPert|AidRelative,
    FFreq0           = AidFreq|0,
    FTime0           = AidTime|0,
    FMask            = AidMask,
    FNormalized      = AidNormalized,
    
    FSpids           = AidSpid|AidIndex,
    FPerturbedValues = AidPerturbed|AidValue,
    FPerturbations   = AidPerturbations,
    FPolcs           = AidPolcs,

    FFail            = AidFail,
    FOrigin          = AidOrigin,
    FOriginLine      = AidOrigin|AidLine,
    FMessage         = AidMessage,
    
    FSolvable        = AidSolvable,
    FSolvableParm    = AidSolvable|AidParm,
    FNumSteps        = AidNum|AidSteps,
    FEpsilon         = AidEpsilon,
    FUseSVD          = AidUseSVD,

    FMetrics         = AidMetrics,
    FRank            = AidRank,
    FFit             = AidFit,
    FErrors          = AidErrors,
    FCoVar           = AidCoVar,
    FFlag            = AidFlag,
    FMu              = AidMu,
    FStdDev          = AidStdDev,
    FChi             = AidChi,

    FContagiousFail  = AidContagious|AidFail,
    
    FIndex           = AidIndex;
    

};

#endif
