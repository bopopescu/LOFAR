//#  CTStartDaemon.cc: Program that can start others on command.
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

// NOTE: Although the CTStartDaemon does NOT depend on PVSS it needs the PVSS libs
//		 during compiling because APLCommon depends on PVSS

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/LofarLocators.h>

#include <GCF/GCF_ServiceInfo.h>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/APLUtilities.h>
#include <APL/APLCommon/ControllerDefines.h>
#include <APL/APLCommon/StartDaemon_Protocol.ph>
#include "CTStartDaemon.h"

using namespace LOFAR::GCF::Common;
using namespace LOFAR::GCF::TM;

namespace LOFAR {
  namespace CUDaemons {

//
// CTStartDaemon(taskname)
//
CTStartDaemon::CTStartDaemon(const string& name) :
	GCFTask    			((State)&CTStartDaemon::initial_state, name),
	itsActionList		(),
	itsActiveCntlrs		(),
	itsListener			(0),
	itsListenRetryTimer	(0),
	itsClients			(),
	itsTimerPort		(0)
{
	LOG_TRACE_FLOW(formatString("CTStartDaemon(%s)", getName().c_str()));

	itsListener = new GCFTCPPort(*this, MAC_SVCMASK_STARTDAEMON, 
								 GCFPortInterface::MSPP, STARTDAEMON_PROTOCOL);
	ASSERTSTR(itsListener, "Unable to allocate listener port");

	itsTimerPort = new GCFTimerPort(*this, "TimerPort");
	ASSERTSTR(itsTimerPort, "Unable to allocate timer port");

	registerProtocol(STARTDAEMON_PROTOCOL, STARTDAEMON_PROTOCOL_signalnames);
}


//
// ~CTStartDaemon
//
CTStartDaemon::~CTStartDaemon()
{
	LOG_TRACE_FLOW(formatString("~CTStartDaemon(%s)", getName().c_str()));

	if (itsListener) {
		itsListener->close();
		delete itsListener;
	}

	if (itsTimerPort) {
		delete itsTimerPort;
	}

	itsActionList.clear();
	itsActiveCntlrs.clear();

}

// -------------------- Internal routines --------------------

//
// findAction(port)
//
CTStartDaemon::actionIter	CTStartDaemon::findAction(GCFPortInterface*		aPort)
{
	actionIter		iter = itsActionList.begin();
	actionIter		end  = itsActionList.end();
	while (iter != end && iter->parentPort != aPort) {
		iter++;
	}
	return (iter);
}


//
// findAction(timerID)
//
CTStartDaemon::actionIter	CTStartDaemon::findAction(uint32	aTimerID)
{
	actionIter		iter = itsActionList.begin();
	actionIter		end  = itsActionList.end();
	while (iter != end && iter->timerID != aTimerID) {
		iter++;
	}
	return (iter);
}


//
// findAction(cntlrName)
//
CTStartDaemon::actionIter	CTStartDaemon::findAction(const string&		cntlrName)
{
	actionIter		iter = itsActionList.begin();
	actionIter		end  = itsActionList.end();
	while (iter != end && iter->cntlrName != cntlrName) {
		iter++;
	}
	return (iter);
}


//
// findController(port)
//
CTStartDaemon::CTiter	CTStartDaemon::findController(GCFPortInterface*		aPort)
{
	CTiter		iter = itsActiveCntlrs.begin();
	CTiter		end  = itsActiveCntlrs.end();
	while (iter != end && iter->second != aPort) {
		iter++;
	}
	return (iter);
}


//
// sendCreatedMsg(action, result)
//
void CTStartDaemon::sendCreatedMsg(actionIter		action, int32	result)
{
	// send customer message that controller is on the air
	STARTDAEMONCreatedEvent createdEvent;
	createdEvent.cntlrType = action->cntlrType;
	createdEvent.cntlrName = action->cntlrName;
	createdEvent.result	   = result;
	action->parentPort->send(createdEvent);
}


//
// sendNewParentAndCreatedMsg()
//
void CTStartDaemon::sendNewParentAndCreatedMsg(actionIter		action)
{
	// connection with new controller is made, send 'newparent' message
	STARTDAEMONNewparentEvent	msg;
	msg.cntlrName	  = action->cntlrName;
	msg.parentHost	  = action->parentHost;
	msg.parentService = action->parentService;
	
	// map controllername to controllerport
	string		adminName = sharedControllerName(msg.cntlrName);
	CTiter	controller = itsActiveCntlrs.find(adminName);
	ASSERTSTR(isController(controller), adminName << 
										" not found in controller list");
	controller->second->send(msg);
	LOG_DEBUG_STR("Sending NewParent(" << msg.cntlrName << "," <<
						msg.parentHost << "," << msg.parentService << ")");

	// send customer message that controller is on the air
	sendCreatedMsg(action, SD_RESULT_NO_ERROR);
	action->parentPort->close();
}


//
// handleClientDisconnect(port)
//
// A disconnect event was receiveed on a client port. Close the port
// and remove the port from our pool.
//
void CTStartDaemon::handleClientDisconnect(GCFPortInterface&	port)
{
	// end TCP connection
	port.close();

	// remove actions on this port
	actionIter		action = findAction(&port);
	while (isAction(action)) {
		itsTimerPort->cancelTimer(action->timerID);
		LOG_DEBUG_STR ("Disconnect:removing " << action->cntlrName << " from actionlist");
		itsActionList.erase(action);
		action = findAction(&port);
	}

	// cleanup active controller list
	CTiter	controller = findController(&port);
	while (isController(controller)) {
		LOG_DEBUG_STR ("Removing " << controller->first << " from controllerlist");
		itsActiveCntlrs.erase(controller);
		controller = findController(&port);
	}

	// cleanup connection in our pool if it is one of them
	vector<GCFPortInterface*>::iterator	iter = itsClients.begin();
	vector<GCFPortInterface*>::iterator	theEnd = itsClients.end();

	while(iter != theEnd) {
		if (*iter == &port) {
			LOG_DEBUG_STR ("Erasing client port " << &port);
			delete *iter;	// ??
			itsClients.erase(iter);
			return;
		}
		iter++;
	}
}


//
// startController(taskname, paramfile)
//
int32 CTStartDaemon::startController(uint16			cntlrType,
								     const string&	cntlrName,
								     const string&	parentHost,
								     const string&	parentService)
{
	// not found? report problem
	if (cntlrType == CNTLRTYPE_NO_TYPE || cntlrType >= CNTLRTYPE_NR_TYPES) {
		LOG_DEBUG_STR("No support for starting controller of the type " << cntlrType);
		return (SD_RESULT_UNSUPPORTED_TYPE);
	}

	// locate program.
	ProgramLocator		PL;
	string	executable = PL.locate(getExecutable(cntlrType));
	if (executable.empty()) {
		LOG_DEBUG_STR("Executable '" << getExecutable(cntlrType) << "' not found.");
		return (SD_RESULT_PROGRAM_NOT_FOUND);
	}

	// construct system command
	string	startCmd = formatString("./startController.sh %s %s %s %s", 
									executable.c_str(),
									cntlrName.c_str(),
									parentHost.c_str(),
									parentService.c_str());
	LOG_DEBUG_STR("About to start: " << startCmd);

	int32	result = system (startCmd.c_str());
	LOG_DEBUG_STR ("Result of start = " << result);

	if (result == -1) {
		return (SD_RESULT_START_FAILED);
	}
	
	return (SD_RESULT_NO_ERROR);

}




// -------------------- STATE MACHINES --------------------

//
// initial_state(event, port)
//
// The only target in this state is to get the listener port on the air.
//
GCFEvent::TResult CTStartDaemon::initial_state(GCFEvent& event, 
											   GCFPortInterface& /*port*/)
{
	LOG_DEBUG(formatString("CTStartDaemon(%s)::initial_state (%s)",getName().c_str(),evtstr(event)));
  
	GCFEvent::TResult status = GCFEvent::HANDLED;
	switch (event.signal) {
	case F_INIT:
		break;

	case F_ENTRY: {
		itsListener->open();		// may result in CONN or DISCONN event
		break;
	}
  
	case F_CONNECTED:			// Must be from itsListener port
		if (itsListener->isConnected()) {
			if (itsListenRetryTimer) {
				itsListener->cancelTimer(itsListenRetryTimer);
				itsListenRetryTimer = 0;
			}
			LOG_DEBUG ("Listener port opened, going to operational mode");
			TRAN(CTStartDaemon::operational_state);
		}
		break;

	case F_DISCONNECTED:		// Must be from itsListener port
		LOG_DEBUG("Could not open listener, retry in 1 second");
		itsListenRetryTimer = itsListener->setTimer(1.0);	// retry in 1 second
		break;

	case F_TIMER:					//Must be from listener port
		if (!itsListener->isConnected()) {	// still not connected?
			itsListener->open();			// try again.
		}
		break;
	
	default:
		LOG_DEBUG("CTStartDaemon::initial_state, default");
		status = GCFEvent::NOT_HANDLED;
		break;
	}    

	return (status);
}

//
// operational_state(event, port)
//
// This is the normal operational mode. Wait for clients (e.g. MACScheduler) to
// connect, wait for commands from the clients and handle those.
//
GCFEvent::TResult CTStartDaemon::operational_state (GCFEvent& event, 
													GCFPortInterface& port)
{
	LOG_DEBUG(formatString("operational_state:%s", evtstr(event)));

	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (event.signal) {
	case F_INIT:
		break;
  
	case F_ENTRY: 
		break;

	case F_ACCEPT_REQ: {
		GCFTCPPort*		newClient = new GCFTCPPort;
		newClient->init(*this, "client", GCFPortInterface::SPP, STARTDAEMON_PROTOCOL);
		itsListener->accept(*newClient);
		itsClients.push_back(newClient);	// remember port for future delete.
		break;
	}

	case F_CONNECTED:
		break;

	case F_DISCONNECTED:
		ASSERTSTR (&port != itsListener, "Listener port was closed, bailing out!");
		handleClientDisconnect(port);
		break;

	case F_CLOSED:
		break;

	case F_TIMER: {
			GCFTimerEvent& timerEvent=static_cast<GCFTimerEvent&>(event);
			actionIter		action = findAction(timerEvent.id);
			if (isAction(action)) {
				// controller did not start with required time. report failure.
				STARTDAEMONCreatedEvent		createdEvent;
				createdEvent.cntlrType = action->cntlrType;
				createdEvent.cntlrName = action->cntlrName;
				createdEvent.result    = SD_RESULT_START_FAILED;
				action->parentPort->send(createdEvent);
				LOG_WARN_STR("Timeout in startup of " << action->cntlrName);

				LOG_DEBUG_STR ("Removing " << action->cntlrName << " from actionlist");
				itsActionList.erase(action);
			}
		}
		break;

	case STARTDAEMON_CREATE: {
		STARTDAEMONCreateEvent 	createEvent(event);
		// prepare an answer message
		STARTDAEMONCreatedEvent createdEvent;
		createdEvent.cntlrType = createEvent.cntlrType;
		createdEvent.cntlrName = createEvent.cntlrName;

		// convert fullname used in SD protocol to sharedname for internal admin
		string	adminName = sharedControllerName(createEvent.cntlrName);
		LOG_DEBUG_STR("request to start: " << createdEvent.cntlrType << "," << 
								createdEvent.cntlrName << "(" << adminName << ")");

		// is controller already known?
		CTiter	controller = itsActiveCntlrs.find(adminName);
		if (!isController(controller)) {		// no, controller is not active
			// Ask starter Object to start the controller.
			createdEvent.result = startController(createEvent.cntlrType,
												  adminName, 
												  createEvent.parentHost,
												  createEvent.parentService);
			// when creation failed, report it back.
			if (createdEvent.result != SD_RESULT_NO_ERROR) {
				LOG_WARN_STR("Startup of " << adminName << " failed");
				port.send(createdEvent);
				break;
			}
		}

		// controller already registered?
		if (isController(controller) && controller->second != 0) {
			// send newparent message right away.
			STARTDAEMONNewparentEvent	npEvent;
			npEvent.cntlrName     = createEvent.cntlrName;
			npEvent.parentHost    = createEvent.parentHost;
			npEvent.parentService = createEvent.parentService;
			controller->second->send(npEvent);
			LOG_DEBUG_STR("Sending NewParent(" << npEvent.cntlrName << "," <<
						npEvent.parentHost << "," << npEvent.parentService << ")");
			// report result back to caller.
			createdEvent.result = SD_RESULT_NO_ERROR;
			port.send(createdEvent);
			break;
		}

		// wait for controller to register, add action to actionList
		action_t			action;
		action.cntlrName	 = createEvent.cntlrName;
		action.cntlrType	 = createEvent.cntlrType;
		action.parentHost	 = createEvent.parentHost;
		action.parentService = createEvent.parentService;
		action.parentPort	 = &port;
		action.timerID		 = itsTimerPort->setTimer(20.0);	// failure over 20 sec
		LOG_DEBUG_STR ("Adding " << action.cntlrName << " to actionList");
		itsActionList.push_back(action);
		break;
	}

	case STARTDAEMON_ANNOUNCEMENT: {
		STARTDAEMONAnnouncementEvent	inMsg(event);
		// known controller? (announcement msg always contains sharedname).
		CTiter	controller = itsActiveCntlrs.find(inMsg.cntlrName);
		// controller already registered?
		if (isController(controller) && controller->second != 0) {
			if (controller->second == &port) {	// on same port, no problem
				LOG_DEBUG_STR ("Double announcement received of " << inMsg.cntlrName);
				break;
			}
			else {								// on other port, report error.
				LOG_ERROR_STR ("Controller " << inMsg.cntlrName << 
						  " registered at two ports!!! Ignoring second registration!");
				break;
			}
		}

		// register the controller and its port
		itsActiveCntlrs[inMsg.cntlrName] = &port; 
		LOG_DEBUG_STR("Received announcement of " << inMsg.cntlrName << 
						", adding it to the controllerList");

		// are there a newParent actions waiting?
		actionIter	action = findAction(inMsg.cntlrName);
		while (isAction(action)) {
			// first stop failure-timer
			itsTimerPort->cancelTimer(action->timerID);

			sendNewParentAndCreatedMsg(action);
			LOG_DEBUG_STR ("Removing " << action->cntlrName << " from actionlist");
			itsActionList.erase(action);
			action = findAction(inMsg.cntlrName);
		}
		break;
	}

	default:
		LOG_DEBUG(formatString("CTStartDaemon(%s)::operational_state, default",getName().c_str()));
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return (status);
}


  }; // namespace CUDaemons
}; // namespace LOFAR
