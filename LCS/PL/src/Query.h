//#  Query.h: one line description
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

#ifndef LCS_PL_QUERY_H
#define LCS_PL_QUERY_H

//# Includes

//# Forward Declarations

namespace LCS
{
  namespace PL
  {
    //
    // The query class provides a user-friendly interface for composing
    // queries.
    //
    // \todo This class really needs to be designed from the ground. Of
    // course we \e do want to hide all the details related to the table 
    // and column layout of the relational database. We need to devise a 
    // way to compose a query in terms of user-defined class members.
    // The translation to SQL can (and probably will) be done by a helper
    // class. The getSql() method can then "simply" return the string
    // that was generated by the helper class. We need this helper class
    // because we do not want to couple the Query class with any class
    // or method in our relational mapping library (e.g. DTL).
    //
    class Query
    {
    public:
      // Default constructor. Creates an empty query object.
      Query() {}

      // Constructor that takes an SQL string. 
      // \todo Do we want to do some sanity checking on \c aString ?
      Query(const std::string& aString) : itsSqlString(aString) {}

      // Return the composed query as an SQL string.
      // \todo In a future version, we will probably not store the query
      // as plain SQL; at least not to begin with. So, getSql() will then
      // have to generate (and cache) the SQL string based on the information
      // stored in this object.
      std::string getSql() const;

    private:
      std::string itsSqlString;

    };

  } // namespace PL

} // namespace LCS

#endif
