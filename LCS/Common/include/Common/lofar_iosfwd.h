//#  lofar_iosfwd.h:
//#
//#  Copyright (C) 2002
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

#ifndef LOFAR_COMMON_IOSFWD_H
#define LOFAR_COMMON_IOSFWD_H

// \file lofar_iosfwd.h

#include <iosfwd>

namespace LOFAR
{
  using std::istream;
  using std::ostream;
  using std::iostream;
  using std::istringstream;
  using std::ostringstream;
  using std::stringstream;
  using std::ifstream;
  using std::ofstream;
  using std::fstream;
}

#ifdef MAKE_LOFAR_SYMBOLS_GLOBAL
#include <Common/lofar_global_symbol_warning.h>
using LOFAR::istream;
using LOFAR::ostream;
using LOFAR::iostream;
using LOFAR::istringstream;
using LOFAR::ostringstream;
using LOFAR::stringstream;
using LOFAR::ifstream;
using LOFAR::ofstream;
using LOFAR::fstream;
#endif

#endif
