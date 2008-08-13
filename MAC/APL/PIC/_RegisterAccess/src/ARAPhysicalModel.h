//#  ARAPhysicalModel.h
//#
//#  Copyright (C) 2002-2004
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

#ifndef ARAPhysicalModel_H
#define ARAPhysicalModel_H

#include <vector>
#include <string>
#include <map>
#include <set>
#include <list>
#include <boost/utility.hpp>

namespace LOFAR
{

namespace ARA
{
  class ARAPhysicalModel : boost::noncopyable // prohibits access to copy construction and assignment
  {
    public:
      ARAPhysicalModel();
      ~ARAPhysicalModel();
      
      void inMaintenance(bool maintenance, std::string& resource);

    protected:  
    
    private:    
      typedef std::map<std::string, std::vector<std::string> >  PhysicalModelT;
      typedef PhysicalModelT::iterator                          PhysicalModelIterT;
      typedef std::set<std::string>                             MaintenanceFlagsT;
      typedef MaintenanceFlagsT::iterator                       MaintenanceFlagsIterT;
      
      PhysicalModelT      m_maintenanceModel;
      MaintenanceFlagsT   m_maintenanceFlags;
  };
};

} // namespace LOFAR
#endif
