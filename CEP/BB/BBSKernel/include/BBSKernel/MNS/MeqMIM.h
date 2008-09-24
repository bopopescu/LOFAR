//# MeqMIM.h: Azimuth and elevation for a direction (ra,dec) on the sky.
//#
//# Copyright (C) 2007
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
//# $Id: MeqMIM.h 10458 2007-08-22 09:45:30Z zwieten $

#ifndef MNS_MEQMIM_H
#define MNS_MEQMIM_H

#include <BBSKernel/MNS/MeqJonesNode.h>
#include <BBSKernel/MNS/MeqParmFunklet.h>
#include <ParmDB/ParmDB.h>

#ifdef EXPR_GRAPH
#include <Common/lofar_string.h>
#endif

namespace LOFAR
{
namespace BBS
{
using LOFAR::ParmDB::ParmDB;
class MeqSource;
class MeqStation;
class MeqRequest;
class MeqMatrix;

// \ingroup BBSKernel
// \addtogroup MNS
// @{

  class MeqMIM: public MeqJonesExprRep
  {
  public:
    static  const uint NPARMS=5;
    
    MeqMIM(MeqExpr &pp, vector<MeqExpr> &MIMparms,MeqExpr &ref_pp);
    virtual ~MeqMIM();
    
    static vector<MeqExpr> create_MIMParms(MeqParmGroup &parmGroup,ParmDB *instrumentDBase){
      vector<MeqExpr> MIMParms(NPARMS);
      for(int i=0;i<NPARMS;i++)
	{ ostringstream name;
	  name<<"MIM:"<<i;
	  MIMParms[i]= MeqExpr(MeqParmFunklet::create(name.str(),
							  parmGroup, instrumentDBase));
	}
      return MIMParms;
    };
  // Calculate the result of its members.
  virtual MeqJonesResult getJResult (const MeqRequest&);

   
private:
    void evaluate(const MeqRequest& request, const MeqMatrix &in_x, const MeqMatrix &in_y,
		  const MeqMatrix &in_z, const MeqMatrix &in_alpha, const vector<const MeqMatrix*> Mimparms,
		  const MeqMatrix &in_refx, const MeqMatrix &in_refy, const MeqMatrix &in_refz,
		  MeqMatrix &out_11,MeqMatrix &out_22);

    double calculate_mim_function(const vector<double> & parms,double x,double y,double z,
				  double alpha,double freq,double ref_x,double ref_y,double ref_z);
#ifdef EXPR_GRAPH
    virtual std::string getLabel();
#endif
};

// @}

} // namespace BBS
} // namespace LOFAR

#endif
