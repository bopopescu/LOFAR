//#  ExprNode.h: The node of a query expression.
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

#ifndef LOFAR_PL_QUERY_EXPRNODE_H
#define LOFAR_PL_QUERY_EXPRNODE_H

//# Includes
#include <lofar_config.h>
#include <iosfwd>

namespace LOFAR
{
  namespace PL
  {
    namespace Query
    {
      // @defgroup ExprNode Expression Nodes
      //
      // ExprNode is an abstract base class that represents the node of an
      // expression query. We will need to derive specific expression node
      // classes (e.g. ExprNodeBinary) from it.
      class ExprNode
      {
      public:
        // We need a virtual destructor, because this in an abstract base
        // class.
        virtual ~ExprNode() {}

        // Print the expression node into an output stream.
        virtual void print(std::ostream& os) const = 0;
      };

    } // namespace Query

  } // namespace PL

} // namespace LOFAR

#endif
