//#  Collection.h: one line description
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

#ifndef LCS_PL_COLLECTION_H
#define LCS_PL_COLLECTION_H

//# Includes
#include <list>

namespace LCS
{
  namespace PL
  {
    //
    // Collection is a bag-like container class.
    //
    template<typename T>
    class Collection
    {
    public:

      // This is the type of container we're gonna store our elements in.
      typedef std::list<T> container_t;

      typedef typename container_t::iterator iterator;
      typedef typename container_t::const_iterator const_iterator;

      iterator begin() { return itsContainer.begin(); }
      const_iterator begin() const { return itsContainer.begin(); }

      iterator end() { return itsContainer.end(); }
      const_iterator end() const { return itsContainer.end(); }

      // Add the element \c t to the collection
      void add(const T& t) { itsContainer.push_back(t); }

      // Remove \e all elements from the collection that are equal to the
      // element \c t.
      void remove(const T& t) { itsContainer.remove(t); }

    private:

      // The container that will actually hold our elements.
      container_t itsContainer;

    };

  } // namespace PL

} // namespace LCS

#endif
