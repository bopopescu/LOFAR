//#  ApplControlComm.h: Implements the communication of Application Control.
//#
//#  Copyright (C) 2004
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
//#  Note: This source is best read with tabstop 4.
//#
//#  $Id$

#ifndef LOFAR_ACC_APPLCONTROLCOMM_H
#define LOFAR_ACC_APPLCONTROLCOMM_H

// \file ApplControlComm.h
// Implements the communication of Application Control commands, used by the
// ApplControlClient and ApplControlServer.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <ACC/DH_ApplControl.h>

namespace LOFAR {
  namespace ACC {
// \addtogroup ACC
// @{

// The result field that is passed from the Application Controller to the 
// ACuser is a bitmask that represents the result of the command. See
// \c resultInfo method for more information.
typedef enum { AcCmdMaskOk 	 	  = 0x0001,
			   AcCmdMaskScheduled = 0x0002,
			   AcCmdMaskOverruled = 0x0004,
			   AcCmdMaskCommError = 0x8000 } AcCmdResultMask;

//# Description of class.
// The ApplControlComm class implements the communication between the 
// Application Controller and the ACuser.
//
class ApplControlComm 
{
public:
	// The only constructor.
	explicit ApplControlComm(bool	syncComm);

	// Destructor;
	virtual ~ApplControlComm();

	// CommandInfo returns extra information about the conditions that were met
	// during the execution/scheduling of the last command.
	// The returned value is a bitMask with the following bits:
	// AcCmdMaskOk		 : reflects the bool value returned by the commandcall
	// AcCmdMskScheduled : command was scheduled iso executed immediately
	// AcCmdMskOverruled : the command overruled another scheduled command
	// AcCmdMskCommError : a communication error with the AC server occured
	uint16	resultInfo	(void) const;

	// Constructs a command and sends it to the other side.
	void	sendCmd(const ACCmd			theCmd,
				 	const time_t		theScheduleTime,
				 	const time_t		theWaitTime = 0,
					const string&		theOptions = "",
					const string&		theProcList = "",
					const string&		theNodeList = "") const;

	// Is called after a message is sent to the server. Returns \c true in async
	// comm, does a read on the socket in sync comm. and returns the analysed
	// result.
	bool	waitForResponse() const;

	// Executes the given command: fills a dataholder, send it to the sender,
	// and does a 'waitForResponse'.
	bool	doRemoteCmd(const ACCmd			theCmd,
					    const time_t		theScheduleTime,
					    const time_t		theWaitTime,
						const string&		theOptions = "",
						const string&		theProcList = "",
						const string&		theNodeList = "") const;

	// Returns a pointer to the dataholder.
	DH_ApplControl*		getDataHolder() const;

	// Install a pointer to the DataHolder that can be used to hold the data
	// that must be exchanged.
	void				setDataHolder(DH_ApplControl*	aDHPtr);

private:
	// Not default constructable
	ApplControlComm() {}

	// Copying is also not allowed.
	ApplControlComm(const ApplControlComm& that);

	// Copying is also not allowed.
	ApplControlComm& 	operator=(const ApplControlComm& that);

	//# --- Datamembers ---
	// Pointer to a dataholder (=buffer) that is used for packing and unpacking
	// the information when it is send or read.
	DH_ApplControl*		itsDataHolder;

	// Synchrone or Asynchrone communication.
	bool				itsSyncComm;
};

inline DH_ApplControl*	ApplControlComm::getDataHolder() const
{
	return itsDataHolder;
}

inline void ApplControlComm::setDataHolder(DH_ApplControl*	aDHPtr)
{
	itsDataHolder = aDHPtr;
}

// @} addgroup
} // namespace ACC
} // namespace LOFAR

#endif
