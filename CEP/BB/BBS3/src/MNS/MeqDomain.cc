//# MeqDomain.cc: The domain for an expression
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

#include <lofar_config.h>
#include <BBS3/MNS/MeqDomain.h>
#include <Common/LofarLogger.h>

namespace LOFAR {

MeqDomain::MeqDomain()
: itsOffsetX (0),
  itsScaleX  (1),
  itsOffsetY (0),
  itsScaleY  (1)
{}

MeqDomain::MeqDomain (double startX, double endX,
		      double startY, double endY)
{
  ASSERTSTR (startX < endX, "MeqDomain: startX " << startX <<
	     " must be < endX " << endX);
  ASSERTSTR (startY < endY, "MeqDomain: startY " << startY <<
	     " must be < endY " << endY);
  itsOffsetX = (endX + startX) * .5;
  itsScaleX  = (endX - startX) * .5;
  itsOffsetY = (endY + startY) * .5;
  itsScaleY  = (endY - startY) * .5;
}

MeqDomain::MeqDomain (const ParmDB::ParmDomain& pdomain)
{
  ASSERTSTR (pdomain.getStart().size() == 2,
	     "BBS only supports 2-dim funklets and domains");
  itsOffsetX = (pdomain.getEnd()[0] + pdomain.getStart()[0]) * .5;
  itsScaleX  = (pdomain.getEnd()[0] - pdomain.getStart()[0]) * .5;
  itsOffsetY = (pdomain.getEnd()[1] + pdomain.getStart()[1]) * .5;
  itsScaleY  = (pdomain.getEnd()[1] - pdomain.getStart()[1]) * .5;
}

ParmDB::ParmDomain MeqDomain::toParmDomain() const
{
  return ParmDB::ParmDomain (startX(), endX(), startY(), endY());
}

std::ostream& operator<< (std::ostream& os, const MeqDomain& domain)
{
  os << "[(" << domain.startX() << ',' << domain.endX() << "),("
     << domain.startY() << ',' << domain.endY() << ")]";
  return os;
}

}
