//#  VirtualRoute.h: handles all events for a task.
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

#ifndef VirtualRoute_H
#define VirtualRoute_H

//# Includes

//# GCF Includes

//# local includes
#include <APL/APLCommon/LogicalDevice.h>

//# Common Includes
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_map.h>
#include <boost/shared_ptr.hpp>

//# ACC Includes

// forward declaration

namespace LOFAR
{
  
namespace AVR
{

#define VR_LOGICALSEGMENT_PROPSET_BASENAME        "PAC_WAN_Segments_"
#define VR_LOGICALSEGMENT_PROPSET_TYPE            "TWanLogicalSegment"
#define VR_LOGICALSEGMENT_PROPNAME_CAPACITY       "Capacity"
#define VR_LOGICALSEGMENT_PROPNAME_ALLOCATED      "AllocatedBW"
#define VR_LOGICALSEGMENT_PROPNAME_CHANGEALLOCATED      "changeAllocatedBW"
#define VR_LOGICALSEGMENT_PROPNAME_ACTUALTRAFFIC  "ActualTraffic"

  class WanLSPropertyProxy;

  class VirtualRoute : public APLCommon::LogicalDevice
  {
    public:
      // Logical Device version
      static const string VR_VERSION;

      // property defines


      explicit VirtualRoute(const string& taskName, const string& parameterFile, GCF::TM::GCFTask* pStartDaemon);
      virtual ~VirtualRoute();

    protected:
      // protected default constructor
      VirtualRoute();
      // protected copy constructor
      VirtualRoute(const VirtualRoute&);
      // protected assignment operator
      VirtualRoute& operator=(const VirtualRoute&);

      virtual void concrete_handlePropertySetAnswer(GCF::TM::GCFEvent& answer);
      /**
      * Initial state additional behaviour must be implemented in the derived classes. 
      */
      virtual GCF::TM::GCFEvent::TResult concrete_initial_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p, TLogicalDeviceState& newState, APLCommon::TLDResult& errorCode);
      /**
      * Idle state additional behaviour must be implemented in the derived classes. 
      */
      virtual GCF::TM::GCFEvent::TResult concrete_idle_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p, TLogicalDeviceState& newState, APLCommon::TLDResult& errorCode);
      /**
      * Claiming state additional behaviour must be implemented in the derived classes. 
      */
      virtual GCF::TM::GCFEvent::TResult concrete_claiming_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p, TLogicalDeviceState& newState, APLCommon::TLDResult& errorCode);
      /**
      * Claimed state additional behaviour must be implemented in the derived classes. 
      */
      virtual GCF::TM::GCFEvent::TResult concrete_claimed_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p, TLogicalDeviceState& newState, APLCommon::TLDResult& errorCode);
      /**
      * Preparing state additional behaviour must be implemented in the derived classes. 
      */
      virtual GCF::TM::GCFEvent::TResult concrete_preparing_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p, TLogicalDeviceState& newState, APLCommon::TLDResult& errorCode);
      /**
      * active state additional behaviour must be implemented in the derived classes. 
      */
      virtual GCF::TM::GCFEvent::TResult concrete_active_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p, APLCommon::TLDResult& errorCode);
      /**
      * Releasing state additional behaviour must be implemented in the derived classes. 
      */
      virtual GCF::TM::GCFEvent::TResult concrete_releasing_state(GCF::TM::GCFEvent& e, GCF::TM::GCFPortInterface& p, TLogicalDeviceState& newState, APLCommon::TLDResult& errorCode);

      /**
      * Implementation of the Claim method is done in the derived classes. 
      */
      virtual void concreteClaim(GCF::TM::GCFPortInterface& port);
      /**
      * Implementation of the Prepare method is done in the derived classes. 
      */
      virtual void concretePrepare(GCF::TM::GCFPortInterface& port);
      /**
      * Implementation of the Resume method is done in the derived classes. 
      */
      virtual void concreteResume(GCF::TM::GCFPortInterface& port);
      /**
      * Implementation of the Suspend method is done in the derived classes. 
      */
      virtual void concreteSuspend(GCF::TM::GCFPortInterface& port);
      /**
      * Implementation of the Release method is done in the derived classes. 
      */
      virtual void concreteRelease(GCF::TM::GCFPortInterface& port);
      /**
      * Implementation of the Disconnected handler is done in the derived classes. 
      */
      virtual void concreteParentDisconnected(GCF::TM::GCFPortInterface& port);
      /**
      * Implementation of the Disconnected handler is done in the derived classes. 
      */
      virtual void concreteChildDisconnected(GCF::TM::GCFPortInterface& port);
      virtual void concreteHandleTimers(GCF::TM::GCFTimerEvent& timerEvent, GCF::TM::GCFPortInterface& port);
      virtual void concreteAddExtraKeys(ACC::APS::ParameterSet& /*psSubset*/);

    protected:    

    private:      
      typedef boost::shared_ptr<WanLSPropertyProxy> WanLSPropertyProxyPtr;

      bool _checkQualityRequirements();

      double                            m_requiredBandwidth;
      vector<string>                    m_logicalSegments;
      map<string,WanLSPropertyProxyPtr> m_lsProps;
      unsigned long                     m_qualityCheckTimerId;
      unsigned long                     m_lsCapacityCheckTimer;

      ALLOC_TRACER_CONTEXT  
  };
};//AVR
};//LOFAR
#endif
