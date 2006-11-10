//#  GPM_Controller.cc: 
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

#include <lofar_config.h>
#include <Common/LofarLocators.h>

#include "GPM_Controller.h"
#include <GCF/PAL/GCF_ExtPropertySet.h>
#include <GCF/PAL/GCF_MyPropertySet.h>
#include <GCF/PAL/GCF_SysConnGuard.h>
#include <GCF/Utils.h>
#include <APS/ParameterSet.h>
#include <GCF/PAL/GCF_PVSSInfo.h>

#include <stdio.h>

namespace LOFAR {
  namespace GCF {
	using namespace Common;
	using namespace TM;  
	namespace PAL {

static string 	sPMLTaskName("GCF-PML");
GPMHandler* 	GPMHandler::_pInstance = 0;

void logResult(TPAResult result, GCFPropertySet& propSet);

//
// GPMController
//
GPMController::GPMController() : 
	GCFTask((State)&GPMController::initial, sPMLTaskName),
	_pSysConnGuard(GCFSysConnGuard::instance())
{
	// register the protocol for debugging purposes
	registerProtocol(PA_PROTOCOL, PA_PROTOCOL_signalnames);

	// initialize the port
	_propertyAgent.init(*this, "client", GCFPortInterface::SAP, PA_PROTOCOL);

	_distPropertyAgent.init(*this, "DPAclient", GCFPortInterface::SAP, PA_PROTOCOL);

	// read in the configuration files.
	ConfigLocator 	aCL;
	LOG_DEBUG ("Adopting config file: gcf-pml.conf");
	ACC::APS::globalParameterSet()->adoptFile(aCL.locate("gcf-pml.conf"));
	LOG_DEBUG ("Adopting config file: PropertyAgent.conf");
	ACC::APS::globalParameterSet()->adoptFile(aCL.locate("PropertyAgent.conf"));

	_pSysConnGuard->registerTask(*this);
}

//
// ~GPMController
//
GPMController::~GPMController()
{
	_pSysConnGuard->unregisterTask(*this);
}

GPMController* GPMController::instance(bool temporary)
{
  if (0 == GPMHandler::_pInstance)
  {    
    GPMHandler::_pInstance = new GPMHandler();
    ASSERT(!GPMHandler::_pInstance->mayDeleted());
    GPMHandler::_pInstance->_controller.start();
  }
  if (!temporary) GPMHandler::_pInstance->use();
  return &GPMHandler::_pInstance->_controller;
}

void GPMController::release()
{
  ASSERT(GPMHandler::_pInstance);
  ASSERT(!GPMHandler::_pInstance->mayDeleted());
  GPMHandler::_pInstance->leave(); 
  if (GPMHandler::_pInstance->mayDeleted())
  {
    delete GPMHandler::_pInstance;
    ASSERT(!GPMHandler::_pInstance);
  }
}

//
// loadPropSet(propSet)
//
TPMResult GPMController::loadPropSet(GCFExtPropertySet& propSet)
{
	string destPA = determineDest(propSet.getScope());
	LOG_DEBUG_STR("loadPropSet:scope=" << propSet.getScope() << ",destPA=" << destPA);

	if (!checkDestination(destPA)) {
		return (PM_PA_NOT_REACHABLE);
	}

	PALoadPropSetEvent request;

	TAction action;
	action.pPropSet = &propSet;
	action.signal = request.signal;

	request.seqnr = registerAction(action);

	if (_distPropertyAgent.isConnected() && _propertyAgent.isConnected()) {
		request.scope += propSet.getScope();
		string::size_type index = request.scope.find(':');
		if (index < request.scope.length()) {
			request.scope.erase(0, index + 1);
		}    

		_distPropertyAgent.setDestAddr(destPA);
		_distPropertyAgent.send(request);
	}

	return (PM_NO_ERROR);
}

//
// unloadPropSet(propSet)
//
TPMResult GPMController::unloadPropSet(GCFExtPropertySet& propSet)
{
	string destPA = determineDest(propSet.getScope());
	LOG_DEBUG_STR("unloadPropSet:scope=" << propSet.getScope() << ",destPA=" << destPA);

	if (!checkDestination(destPA)) {
		return (PM_PA_NOT_REACHABLE);
	}

	PAUnloadPropSetEvent request;

	TAction action;
	action.pPropSet = &propSet;
	action.signal = request.signal;

	request.seqnr = registerAction(action);


	if (_distPropertyAgent.isConnected() && _propertyAgent.isConnected()) {
		request.scope += propSet.getScope();
		string::size_type index = request.scope.find(':');
		if (index < request.scope.length()) {
			request.scope.erase(0, index + 1);
		}

		_distPropertyAgent.setDestAddr(destPA);
		_distPropertyAgent.send(request);

		_extPropertySets.remove(&propSet);
	}

	return (PM_NO_ERROR);
}

TPMResult GPMController::configurePropSet(GCFPropertySet& propSet, const string& apcName)
{
	string destPA = determineDest(propSet.getScope());
	LOG_DEBUG_STR("configurePropSet:scope=" <<propSet.getScope() << ",destPA=" << destPA);

	if (!checkDestination(destPA)) {
		return (PM_PA_NOT_REACHABLE);
	}

	PAConfPropSetEvent request;

	TAction action;
	action.pPropSet = &propSet;
	action.signal = request.signal;
	action.apcName = apcName;

	request.seqnr = registerAction(action);

	if (_distPropertyAgent.isConnected() && _propertyAgent.isConnected()) {
		request.scope += propSet.getScope();
		string::size_type index = request.scope.find(':');
		if (index < request.scope.length()) {
			request.scope.erase(0, index + 1);
		}
		request.apcName = apcName;

		_distPropertyAgent.setDestAddr(destPA);
		_distPropertyAgent.send(request);
	}

	return (PM_NO_ERROR);
}

void GPMController::deletePropSet(const GCFPropertySet& propSet)
{
  TAction* pAction;
  for (TActionSeqList::iterator iter = _actionSeqList.begin();
       iter != _actionSeqList.end(); ++iter)
  {
    pAction = &iter->second;
    if (pAction->pPropSet == &propSet)
    {
      pAction->pPropSet = 0;
    }
  }
}

//
// registerScope(propSet)
//
TPMResult GPMController::registerScope(GCFMyPropertySet& propSet)
{
	LOG_DEBUG_STR("registerScope:" << propSet.getScope());

 	// propery already registered??
	if (_myPropertySets.find(propSet.getScope()) != _myPropertySets.end()) {
		return(PM_SCOPE_ALREADY_EXISTS);
	}

	// prepare event and save in actionlist.
	PARegisterScopeEvent request;
	TAction 			 action;
	action.pPropSet = &propSet;
	action.signal   = request.signal;
	request.seqnr   = registerAction(action);

	// send event when connection is established.
	if (_distPropertyAgent.isConnected() && _propertyAgent.isConnected()) {
		_myPropertySets[propSet.getScope()] = &propSet;
		request.scope    = propSet.getScope();
		request.type     = propSet.getType();
		request.category = propSet.getCategory();
		_propertyAgent.send(request);
	}

	return (PM_NO_ERROR);
}

TPMResult GPMController::unregisterScope(GCFMyPropertySet& propSet)
{
  TPMResult result(PM_NO_ERROR);
 
  PAUnregisterScopeEvent request;

  TAction action;
  action.pPropSet = &propSet;
  action.signal = request.signal;
  
  request.seqnr = registerAction(action);
    
  if (_distPropertyAgent.isConnected() && _propertyAgent.isConnected())
  {
    request.scope = propSet.getScope();
    _propertyAgent.send(request);
  }
  _myPropertySets.erase(propSet.getScope());

  return result;
}

unsigned short GPMController::registerAction(TAction& action)
{
  unsigned short seqnr(1); // 0 is reserved for internal msg. in PA
  TActionSeqList::const_iterator iter;
  do   
  {
    seqnr++;
    iter = _actionSeqList.find(seqnr);
  } while (iter != _actionSeqList.end());

  _actionSeqList[seqnr] = action; 

  return seqnr;
}

void GPMController::propertiesLinked(const string& scope, TPAResult result)
{  
  if (_propertyAgent.isConnected())
  {
    PAPropSetLinkedEvent response;
    response.result = result;
    response.scope = scope;
    _propertyAgent.send(response);
  }
}

void GPMController::propertiesUnlinked(const string& scope, TPAResult result)
{
  if (_propertyAgent.isConnected())
  {
    PAPropSetUnlinkedEvent response;
    response.result = result;
    response.scope = scope;
    _propertyAgent.send(response);
  }
}

GCFEvent::TResult GPMController::initial(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  switch (e.signal)
  {
    case F_INIT:
      break;

    case F_ENTRY:
      if (!_propertyAgent.isConnected())
      {
        _propertyAgent.open();
      }
      if (!_distPropertyAgent.isConnected())
      {
        _distPropertyAgent.open();
      }
      break;
    
    case F_TIMER:
      p.open();
      break;

    case F_CONNECTED:
      if (_propertyAgent.isConnected() && _distPropertyAgent.isConnected())
      {
        TRAN(GPMController::connected);
      }
      break;

    case F_DISCONNECTED:
      p.setTimer(1.0); // try again after 1 second
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult GPMController::connected(GCFEvent& e, GCFPortInterface& p)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  TGCFResult result;

  switch (e.signal)
  {
    case F_DISCONNECTED:
      if (&_propertyAgent == &p)
      {
        LOG_WARN(formatString ( 
            "Connection lost to Property Agent"));
        p.close();
      }
      break;
      
    case F_CLOSED:
      TRAN(GPMController::initial);
      break;
      
    case F_ENTRY:
    {
      TAction* pAction(0);
      TActionSeqList tmpSeqList(_actionSeqList);
      _actionSeqList.clear();
      for (TActionSeqList::iterator iter = tmpSeqList.begin();
           iter != tmpSeqList.end(); ++iter)
      {
        pAction = &iter->second;
        if (!pAction->pPropSet) continue;
        switch (pAction->signal)
        {
          case PA_REGISTER_SCOPE: registerScope(* (GCFMyPropertySet*)pAction->pPropSet); break;
          case PA_UNREGISTER_SCOPE: unregisterScope(*(GCFMyPropertySet*)pAction->pPropSet); break;
          case PA_CONF_PROP_SET: configurePropSet(*pAction->pPropSet, pAction->apcName); break;
          case PA_LOAD_PROP_SET: loadPropSet(*(GCFExtPropertySet*)pAction->pPropSet); break;
          case PA_UNLOAD_PROP_SET: unloadPropSet(*(GCFExtPropertySet*)pAction->pPropSet); break;
          default: ASSERT(0);
        }
      }
      break;
    }  
    case PA_SCOPE_REGISTERED:
    {
      PAScopeRegisteredEvent response(e);
      result = (response.result == PA_NO_ERROR ? GCF_NO_ERROR : GCF_MYPS_ENABLE_ERROR);
      GCFMyPropertySet* pPropertySet = (GCFMyPropertySet*) findPropSetInActionList(response.seqnr);
      _actionSeqList.erase(response.seqnr);
      if (pPropertySet) {
        logResult(response.result, *pPropertySet);
        if (result != GCF_NO_ERROR) {
          _myPropertySets.erase(pPropertySet->getScope());
        }        
        pPropertySet->scopeRegistered(result);
      }
      break;
    }

    case PA_SCOPE_UNREGISTERED:
    {
      PAScopeUnregisteredEvent response(e);
      GCFMyPropertySet* pPropertySet = (GCFMyPropertySet*) findPropSetInActionList(response.seqnr);
      result = (response.result == PA_NO_ERROR ? GCF_NO_ERROR : GCF_MYPS_DISABLE_ERROR);
      _actionSeqList.erase(response.seqnr);
      if (pPropertySet)
      {
        pPropertySet->scopeUnregistered(result);
      }
      break;
    }

    case PA_LINK_PROP_SET:
    {
      PALinkPropSetEvent request(e);
      LOG_INFO(formatString ( 
        "PA-REQ: Link properties of prop. set '%s'",        
        request.scope.c_str()));
        
      GCFMyPropertySet* pPropertySet = findMyPropSet(request.scope);
      if (pPropertySet)
      {
        if (!pPropertySet->linkProperties())
        {
          // properties could not be set on this time, because the property set (DP) is not yet known 
          // at this time. The PVSS API is still busy with synchronising its DP name administration, after 
          // the PA has created the DP.
          
          TLinkTimers timers;
          
          timers.lastRetryTimerId = _propertyAgent.setTimer(0, 0, 0, 0, pPropertySet);
          timers.linkedTimeOutId = _propertyAgent.setTimer(2, 0, 0, 0, pPropertySet);
          LOG_DEBUG(formatString("Has started timers (%d, %d) for retrying the linking action for '%s'",
              timers.lastRetryTimerId,
              timers.linkedTimeOutId,
              pPropertySet->getScope().c_str()));
          _linkTimerList[pPropertySet] = timers;
        }
      }
      else
      {
        LOG_DEBUG(formatString ( 
            "Property set with scope %d was deleted in the meanwhile", 
            request.scope.c_str()));
        PAPropSetLinkedEvent response;
        response.result = PA_PS_GONE;
        _propertyAgent.send(response);
      }
      break;
    }
    case PA_UNLINK_PROP_SET:
    {
      PAUnlinkPropSetEvent request(e);
      LOG_INFO(formatString ( 
        "PA-REQ: Unlink properties of prop. set '%s'",
        request.scope.c_str()));
      GCFMyPropertySet* pPropertySet = findMyPropSet(request.scope);
      if (pPropertySet)
      {
        _linkTimerList.erase(pPropertySet);
        pPropertySet->unlinkProperties();
      }
      else
      {
        LOG_DEBUG(formatString ( 
            "Property set with scope %s was deleted in the meanwhile", 
            request.scope.c_str()));
        PAPropSetUnlinkedEvent response;
        response.result = PA_PS_GONE;
        _propertyAgent.send(response);
      }
      break;
    }
    case PA_PROP_SET_LOADED:
    {
      PAPropSetLoadedEvent response(e);
      result = (response.result == PA_NO_ERROR ? GCF_NO_ERROR : GCF_EXTPS_LOAD_ERROR);        
      GCFExtPropertySet* pPropertySet = (GCFExtPropertySet*) findPropSetInActionList(response.seqnr);
      _actionSeqList.erase(response.seqnr);
      if (pPropertySet)
      {
        if (result == GCF_NO_ERROR)
        {
          _extPropertySets.push_back(pPropertySet);
        }
        logResult(response.result, *pPropertySet);
        pPropertySet->loaded(result);
      }
      break;
    }
    case PA_PROP_SET_UNLOADED:
    {
      PAPropSetUnloadedEvent response(e);
      result = (response.result == PA_NO_ERROR ? GCF_NO_ERROR : GCF_EXTPS_UNLOAD_ERROR);        
      GCFExtPropertySet* pPropertySet = (GCFExtPropertySet*) findPropSetInActionList(response.seqnr);
      _actionSeqList.erase(response.seqnr);
      if (pPropertySet)
      {
        logResult(response.result, *pPropertySet);
        pPropertySet->unloaded(result);
      }
      break;
    }
    case PA_PROP_SET_CONF:
    {
      PAPropSetConfEvent response(e);
      result = (response.result == PA_NO_ERROR ? GCF_NO_ERROR : GCF_PS_CONFIGURE_ERROR);        
      GCFPropertySet* pPropertySet = findPropSetInActionList(response.seqnr);
      _actionSeqList.erase(response.seqnr);
      if (pPropertySet)
      {
        logResult(response.result, *pPropertySet);
        pPropertySet->configured(result, response.apcName);
      }
      break;
    }
    case F_SYSGONE:   
    case PA_PROP_SET_GONE:
    {
      string scope;
      if (e.signal == F_SYSGONE)
      {
        GCFSysConnGuardEvent* pEvent = (GCFSysConnGuardEvent*) &e;
        scope = pEvent->pSysName;
        scope += ':';
      }
      else
      {
        PAPropSetGoneEvent indication(e);
        scope = indication.scope;
      }
      GCFExtPropertySet* pPropertySet;
      string fullScope;
      for (TExtPropertySets::iterator iter = _extPropertySets.begin();
           iter != _extPropertySets.end(); ++iter)
      {
        pPropertySet = *iter;
        ASSERT(pPropertySet);
        fullScope = pPropertySet->getScope();        

        if (fullScope.find(':') >= fullScope.length())
        {
          fullScope = GCFPVSSInfo::getLocalSystemName() + ":" + fullScope;
        }
        if (fullScope.find(scope) == 0)
        {
          pPropertySet->serverIsGone();
          iter = _extPropertySets.erase(iter);
          --iter;
        }
      }
      break;
    }
    case F_TIMER:
    {
      GCFTimerEvent* pTimer = (GCFTimerEvent*)(&e);      
      GCFMyPropertySet* pPropertySet = (GCFMyPropertySet*)(pTimer->arg);
      TLinkTimers timers;
      TLinkTimerList::iterator iter = _linkTimerList.find(pPropertySet);
      if (iter == _linkTimerList.end())
      {
        LOG_DEBUG(formatString("Property set with retry timer %d has been deleted from list. Skipped!",
          pTimer->id));
        break;
      }
      timers = iter->second;
      if (timers.linkedTimeOutId == pTimer->id)
      {
        _propertyAgent.cancelTimer(timers.lastRetryTimerId);
        LOG_ERROR(formatString(
            "link request could not be handled in time by the property set provider of '%s'",
            pPropertySet->getScope().c_str()));
        propertiesLinked(pPropertySet->getScope(), PA_LINK_TIME_OUT);
        _linkTimerList.erase(iter);
      }
      else
      {
        if (!pPropertySet->tryLinking())
        {
          timers.lastRetryTimerId = _propertyAgent.setTimer(0, 0, 0, 0, pPropertySet);
          _linkTimerList[pPropertySet] = timers;
          LOG_DEBUG(formatString("Has restarted retry timers (%d) for retrying the linking action for '%s'",
              timers.lastRetryTimerId,
              pPropertySet->getScope().c_str()));        
        }
        else
        {
          LOG_DEBUG(formatString(
            "link request could be handled in time by the property set provider of '%s'",
            pPropertySet->getScope().c_str()));
          _propertyAgent.cancelTimer(timers.linkedTimeOutId);
          _linkTimerList.erase(iter);
        }
      }
      break; 
    }     
    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

void logResult(TPAResult result, GCFPropertySet& propSet)
{
  switch (result)
  {
    case PA_NO_ERROR:
      break;
    case PA_UNKNOWN_ERROR:
      LOG_FATAL(formatString ("Unknown error"));      
      break;
    case PA_PS_GONE:
      LOG_ERROR(formatString ( 
          "The property set is gone while perfoming an action on it. (s=%s,t=%s)",
          propSet.getScope().c_str(), propSet.getType().c_str()));
      break;
    case PA_MISSING_PROPS:
      LOG_ERROR(formatString ( 
          "One or more loaded properties are not owned by any application. (s=%s,t=%s)",
          propSet.getScope().c_str(), propSet.getType().c_str()));
      break;
    case PA_WRONG_STATE:
      LOG_FATAL(formatString ( 
          "The my property set is in a wrong state. (s=%s,t=%s)",
          propSet.getScope().c_str(), propSet.getType().c_str()));
      break;
    case PA_PROP_SET_NOT_EXISTS:
      LOG_INFO(formatString ( 
          "Prop. set does not exists. (s=%s,t=%s)",
          propSet.getScope().c_str(), propSet.getType().c_str()));
      break;
    case PA_PROP_SET_ALREADY_EXISTS:
      LOG_INFO(formatString ( 
          "Prop. set allready exists. (s=%s,t=%s)",
          propSet.getScope().c_str(), propSet.getType().c_str()));
      break;
    case PA_DPTYPE_UNKNOWN:
      LOG_INFO(formatString ( 
          "Specified type not known. (s=%s,t=%s)",
          propSet.getScope().c_str(), propSet.getType().c_str()));
      break;
    case PA_INTERNAL_ERROR:
      LOG_FATAL(formatString ( 
          "Internal error in PA. (s=%s,t=%s)",
          propSet.getScope().c_str(), propSet.getType().c_str()));
      break;
    case PA_PI_INTERNAL_ERROR:
      LOG_FATAL(formatString ( 
          "Internal error in PI. (s=%s,t=%s)",
          propSet.getScope().c_str(), propSet.getType().c_str()));
      break;
    case PA_APC_NOT_EXISTS:
      LOG_ERROR(formatString ( 
          "APC not exists. (s=%s,t=%s)",
          propSet.getScope().c_str(), propSet.getType().c_str()));
      break;
    case PA_LINK_TIME_OUT:
      LOG_ERROR(formatString ( 
          "Linking of the prop. set could not be completed in time (s=%s,t=%s)",
          propSet.getScope().c_str(), propSet.getType().c_str()));
      break;
    case PA_SERVER_GONE:
      LOG_INFO(formatString ( 
          "Server of prop. set is gone (s=%s,t=%s)",
          propSet.getScope().c_str(), propSet.getType().c_str()));
      break;      
    default:
      break;
  }
}

string GPMController::determineDest(const string& scope) const
{
  string::size_type index = scope.find(':');
  string destDP("__gcfportAPI_DPAserver");
  if (index < scope.length())
  {
    destDP.insert(0, scope.c_str(), index + 1);
  }
  return destDP;
}

bool GPMController::checkDestination(const string& destAddr) const
{
  if (!GCFPVSSInfo::propExists(destAddr))
  {
    string destPA = destAddr;
    string::size_type pos = destPA.find(':');
    if (pos < destPA.length())
    {
      destPA.erase(pos, destPA.length());
    }
    else
    {
      destPA = GCFPVSSInfo::getLocalSystemName();
    }
    LOG_ERROR(formatString(
        "PA on system %s not reachable!",
        destPA.c_str()));
    return false;
  }
  else
  {
    return true;
  }
}

GCFPropertySet* GPMController::findPropSetInActionList(unsigned short seqnr) const
{
  TActionSeqList::const_iterator iter = _actionSeqList.find(seqnr);
  const TAction* pAction(0);
  GCFPropertySet* pPropSet(0);
  if (iter != _actionSeqList.end())
  {
    pAction = &(iter->second);
  }
  if (pAction)
  {
    pPropSet = pAction->pPropSet;
  }
  
  return pPropSet;
}

GCFMyPropertySet* GPMController::findMyPropSet(string& scope) const
{
  TMyPropertySets::const_iterator iter = _myPropertySets.find(scope);
  GCFMyPropertySet* pMyPropSet(0);
  if (iter != _myPropertySets.end())
  {
    pMyPropSet = iter->second;
  }
  return pMyPropSet;
}
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
