//#  filename.cc: one line description
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

#include <PL/Query/BinaryExprNode.h>
#include <iostream>

namespace LOFAR
{
  namespace PL
  {
    namespace Query
    {

      BinaryExprNode::BinaryExprNode(const std::string& oper, 
                                     const Expr& lhs, const Expr& rhs) :
        itsOperation(oper), 
        itsLeft(lhs), itsRight(rhs)
      {
      }

      BinaryExprNode::~BinaryExprNode()
      {
      }

      void BinaryExprNode::print(std::ostream& os) const
      {
        if (isNull()) return;
//         os << "(";
        itsLeft.print(os);
        os << itsOperation;
        itsRight.print(os);
//         os << ")";
      }

      Expr BinaryExprNode::getConstraint() const
      {
        return itsLeft.getConstraint() && itsRight.getConstraint();
      }

      bool BinaryExprNode::isNull() const
      {
        return itsLeft.isNull() || itsRight.isNull();
      }

    } // namespace Query

  } // namespace PL

} // namespace LOFAR
