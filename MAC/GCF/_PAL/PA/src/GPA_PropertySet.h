//#  GPA_PropertySet.h: manages the properties with its use count
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

#ifndef GPA_PROPERTYSET_H
#define GPA_PROPERTYSET_H

#include <GPA_Defines.h>
#include <GSA_Service.h>
#include <GCF/Protocols/PA_Protocol.ph>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace TM
  {
class GCFPortInterface;
  }
  namespace PAL 
  {

/**
   This class manages the properties with its use count, which are created 
   (resp. deleted) by means of the base class GSAService.
*/

class GPAController;

class GPAPropertySet : public GSAService
{
  public:
  	GPAPropertySet(GPAController& controller, TM::GCFPortInterface& serverPort);
  	virtual ~GPAPropertySet();

    typedef struct
    {
      TM::GCFPortInterface* pPSClientPort;
      unsigned short count;
    } TPSClient;

    bool enable(PARegisterScopeEvent& request);
    void disable(PAUnregisterScopeEvent& request);
    void load(PALoadPropSetEvent& request, TM::GCFPortInterface& p);
    void unload(PAUnloadPropSetEvent& request, const TM::GCFPortInterface& p);
    void configure(PAConfPropSetEvent& request);
    
    void linked(PAPropSetLinkedEvent& response);
    void unlinked(PAPropSetUnlinkedEvent& response);
    
    void clientGone(TM::GCFPortInterface& p);
    
    bool isOwner(const TM::GCFPortInterface& p) const { return (&p == &_serverPort); }
    bool mayDelete() const { return (_state == S_DISABLED); }
    bool knowsClient(const TM::GCFPortInterface& p) { return (findClient(p) != 0); }
    			
  protected:
    void dpCreated(const string& dpName);
    void dpDeleted(const string& dpName);
    void dpeValueGet(const string& /*dpeName*/, const Common::GCFPValue& /*value*/) {}; 
    void dpeValueChanged(const string& /*dpeName*/, const Common::GCFPValue& /*value*/) {};
    void dpeValueSet(const string& /*dpeName*/) {};
    void dpeSubscribed(const string& /*dpeName*/) {};
    void dpeSubscriptionLost (const string& dpeName);
    void dpeUnsubscribed(const string& /*dpeName*/) {};

  private: // helper methods
    void link();
    void unlink();
    void wrongState(const char* request);
    TPSClient* findClient(const TM::GCFPortInterface& p);

  private: // data members
    GPAController&	  _controller;
    unsigned short    _usecount;
    string            _name;
    string            _type;
    TM::GCFPortInterface& _serverPort;
    typedef list<TPSClient> TPSClients;
    TPSClients        _psClients;
    Common::TPSCategory       _category;
    
  private: // admin. data members
    typedef enum TSTATE {S_ENABLED, S_ENABLING, S_DISABLED, S_DISABLING, S_LINKING, S_LINKED, S_UNLINKING};
    TSTATE _state;
    unsigned int _counter;
    TPAResult _savedResult;
    unsigned short _savedSeqnr;
};
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
#endif
