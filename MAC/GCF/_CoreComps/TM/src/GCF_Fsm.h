//#  GCF_Fsm.h: header file for the finite state machine implementation.
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

#ifndef GCF_FSM_H
#define GCF_FSM_H

#include <GCF_Event.h>
#include <GCF_TMProtocols.h>
#include <PortInterface/GCF_PortInterface.h>
#include <iostream>
#include <cstdlib>

/**
 * Simple Optimal FSM implementation from 
 * "Practical Statecharts in C/C++", Samek, M., CMP Books, 2002.
 *
 * @todo * Implement the <i>hierarchical</i> state machines.
 */
class GCFDummyPort : public GCFPortInterface
{
    public:
        GCFDummyPort(GCFTask* pTask, string name, int protocol) : 
            GCFPortInterface(pTask, name, SPP, protocol) {};

        int close() {return 0;}
        int open() {return 0;}

		ssize_t send(const GCFEvent& event, void* buf = 0, size_t count = 0) 
        {
            buf = buf;
            count = count;
            return 0;
        }
		ssize_t sendv(const GCFEvent& event, const iovec buffers[], int n) 
        {
            buffers = buffers;
            n = n;            
            return 0;
        }
		
		ssize_t recv(void* buf, size_t count) 
        {
            buf = buf;
            count = count;
            return 0;
        }
		ssize_t recvv(iovec buffers[], int n) 
        {
            buffers = buffers;
            n = n;
            return 0;
        }
        long setTimer(long  delay_sec,
                                            long  delay_usec    = 0,
                                            long  interval_sec  = 0,
                                            long  interval_usec = 0,
                                            const void* arg     = 0) 
        {
            delay_sec = delay_sec;
            delay_usec = delay_usec;
            interval_sec = interval_sec;
            interval_usec = interval_usec;
            arg = arg;
            return 0;
        }

        long setTimer(double delay_seconds, 
                                           double interval_seconds = 0.0,
                                             const void*  arg        = 0)
        {
            delay_seconds = delay_seconds;
            interval_seconds = interval_seconds;
            arg = arg;
            return 0;
        }

        int  cancelTimer(long   timerid, const void** arg = 0) 
        {
            timerid = timerid;
            arg = arg;
            return 0;
        }

        int  cancelAllTimers() {return 0;}

        int  resetTimerInterval(long timerid,
                                            long sec,
                                          long usec = 0) 
        {
            timerid = timerid;
            sec = sec;
            usec = usec;
            return 0;
        }
};

class GCFFsm
{
    public:

        typedef int (GCFFsm::*State)(GCFEvent& event, GCFPortInterface& port); // ptr to state handler type

		GCFFsm(State initial) : _state(initial) {} // ctor
		virtual ~GCFFsm() {}                       // virtual xtor
		
		void initFsm()
		{
			GCFEvent e;
			e.signal = F_ENTRY_SIG;
			(void)(this->*_state)(e, _gcfPort); // entry signal
			e.signal = F_INIT_SIG;
			if (0 != (this->*_state)(e, _gcfPort)) // initial transition
			{
				cerr << "Fsm::init: initial transition F_SIGNAL(F_FSM_PROTOCOL, F_INIT_SIG) not handled.\n";
				exit(1); // EXIT
			}
		}

		inline int dispatch(GCFEvent& event, GCFPortInterface& port)
		{
            return (this->*_state)(event, port);
		}

    protected:

		inline void tran(State target)
		{
			GCFEvent e;
			e.signal = F_EXIT_SIG;
			(void)(this->*_state)(e, _gcfPort); // exit signal
			
			_state = target; // state transition
			
			e.signal = F_ENTRY_SIG;
			(void)(this->*_state)(e, _gcfPort); // entry signal
		}
		#define TRAN(_target_) tran(static_cast<State>(_target_))

    protected:
        volatile State _state;

    private:
        static GCFDummyPort _gcfPort;
};

#endif
