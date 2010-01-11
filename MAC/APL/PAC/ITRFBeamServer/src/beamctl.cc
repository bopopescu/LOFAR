//#
//#  beamctl.cc: implementation of beamctl class
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

// this include needs to be first!
#include <APL/CAL_Protocol/CAL_Protocol.ph>
#include <APL/IBS_Protocol/IBS_Protocol.ph>
#include <APL/RSP_Protocol/RCUSettings.h>
#include <MACIO/MACServiceInfo.h>
#include <GCF/TM/GCF_Control.h>
#include <Common/LofarLocators.h>
#include <Common/ParameterSet.h>

#include "beamctl.h"

#include <iostream>
#include <sys/time.h>
#include <blitz/array.h>
#include <getopt.h>
#include <sys/types.h>
#include <unistd.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_bitset.h>
#include <Common/lofar_string.h>
#include <Common/lofar_list.h>

//#define BEAMCTL_ARRAY "beamctl_array"
#define BEAMCTL_BEAM  "beamctl_beam"

#define ARRAY_FLAG     0x01
#define RCUS_FLAG      0x02
#define RCUMODE_FLAG   0x04
#define DIRECTION_FLAG 0x08
#define SUBBANDS_FLAG  0x10
#define BEAMLETS_FLAG  0x20
#define ALL_FLAGS (  ARRAY_FLAG			\
			 | RCUS_FLAG			\
			 | RCUMODE_FLAG		\
			 | DIRECTION_FLAG		\
			 | SUBBANDS_FLAG		\
			 | BEAMLETS_FLAG)

#define SKYSCAN_STARTDELAY 		30.0
#define	BEAMLET_RING_OFFSET		1000

using namespace std;
using namespace blitz;
using namespace LOFAR;
using namespace BS;
using namespace RTC;
using namespace CAL_Protocol;
using namespace IBS_Protocol;
using namespace GCF::TM;

//
// beamctl(...)
//
beamctl::beamctl(const string& name,
				 const string& antennaSet,
				 const list<int>& rcus,
				 const list<int>& subbands,
				 const list<int>& beamlets,
				 RSP_Protocol::RCUSettings& rcumode,
				 double longitude,   double latitude,   const string& type,
				 double HBlongitude, double HBlatitude, const string& HBtype) :
	GCFTask((State)&beamctl::con2calserver, name), 
	itsCalServer	(0),
	itsBeamServer	(0),
    itsAntSet		(antennaSet), 
	m_rcus			(rcus), 
	m_subbands		(subbands), 
	m_beamlets		(beamlets),
    m_rcumode		(rcumode), 
	m_longitude		(longitude), 
	m_latitude		(latitude), 
	m_type			(type),
	m_HBlongitude	(HBlongitude), 
	m_HBlatitude	(HBlatitude), 
	m_HBtype		(HBtype),
    itsSkyScanTotalTime(3600), 
	itsSkyScanPointTime(2), 
	itsSkyScanWaitTime(10)
{
	registerProtocol(CAL_PROTOCOL, CAL_PROTOCOL_STRINGS);
	registerProtocol(IBS_PROTOCOL, IBS_PROTOCOL_STRINGS);

	itsCalServer  = new GCFTCPPort(*this, MAC_SVCMASK_CALSERVER,  GCFPortInterface::SAP, CAL_PROTOCOL);
	itsBeamServer = new GCFTCPPort(*this, MAC_SVCMASK_BEAMSERVER, GCFPortInterface::SAP, IBS_PROTOCOL);
	ASSERTSTR(itsCalServer,  "Cannot allocate port for CalServer");
	ASSERTSTR(itsBeamServer, "Cannot allocate port for BeamServer");

	try { itsSkyScanTotalTime = globalParameterSet()->getInt32("beamctl.SKYSCAN_TOTAL_TIME"); }
	catch (...) { LOG_INFO_STR(formatString("beamctl.SKYSCAN_TOTAL_TIME")); }

	try { itsSkyScanPointTime = globalParameterSet()->getInt32("beamctl.SKYSCAN_POINT_TIME"); }
	catch (...) { LOG_INFO_STR(formatString("beamctl.SKYSCAN_POINT_TIME")); }

	try { itsSkyScanWaitTime = globalParameterSet()->getInt32("beamctl.SKYSCAN_WAIT_TIME"); }
	catch (...) { LOG_INFO_STR(formatString("beamctl.SKYSCAN_WAIT_TIME")); }
}

//
// ~beamctl()
//
beamctl::~beamctl()
{
	if (itsBeamServer) { itsBeamServer->close();	}
	if (itsCalServer)  { itsCalServer->close();		}
}

//
// con2calserver(event, port)
//
GCFEvent::TResult beamctl::con2calserver(GCFEvent& e, GCFPortInterface& port)
{
	switch(e.signal) {
	case F_INIT:
	break;

	case F_ENTRY: {
		cout << "Connecting to CalServer... " << endl;
		itsCalServer->autoOpen(5, 0, 2);
	}
	break;

	case F_CONNECTED: {
		TRAN(beamctl::con2beamserver);
	}
	break;

	case F_DISCONNECTED: {
		// retry once every second
		cout << "Can not connect to the CalServer, is it running?" << endl;
		port.close();
		port.setTimer(10.0);
	}
	break;

	case F_TIMER: {
		// try again
		cout << "Stil waiting for CalServer... " << endl;
		itsCalServer->autoOpen(5, 0, 2);
	}
	break;

	default:
		return (GCFEvent::NOT_HANDLED);
	break;
	}

	return (GCFEvent::HANDLED);
}

//
// con2beamserver(event, port)
//
GCFEvent::TResult beamctl::con2beamserver(GCFEvent& e, GCFPortInterface& port)
{
	switch(e.signal) {
	case F_ENTRY: {
		cout << "Connecting to BeamServer... " << endl;
		itsBeamServer->autoOpen(5, 0, 2);
	}
	break;

	case F_CONNECTED: {
		TRAN(beamctl::create_subarray);
	}
	break;

	case F_DISCONNECTED: {
		// retry once every second
		if (&port == itsCalServer) {
			cout << "Lost connection with CalServer, trying to reconnect." << endl;
			itsCalServer->close();
			TRAN(beamctl::con2calserver);
		}
		else {
			cout << "Can not connect to the BeamServer, is it running?" << endl;
			port.close();
			port.setTimer(10.0);
		}
	}
	break;

	case F_TIMER: {
		// try again
		cout << "Still waiting for BeamServer... " << endl;
		itsBeamServer->autoOpen(5, 0, 2);
	}
	break;

	default:
		return (GCFEvent::NOT_HANDLED);
	break;
	}

	return (GCFEvent::HANDLED);
}

//
// create_subarray(event, port)
//
GCFEvent::TResult beamctl::create_subarray(GCFEvent& e, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	//
	// Create a new subarray
	//
	switch (e.signal) {
	case F_ENTRY: {
		CALStartEvent start;
		start.name   = BEAMCTL_BEAM + formatString("_%d", getpid());
		start.parent = itsAntSet;
		start.subset = getRCUMask();
		start.rcumode().resize(1);
		start.rcumode()(0) = m_rcumode()(0);

		LOG_INFO(formatString("Rcumode(dec)=%06d", start.rcumode()(0).getRaw()));
		LOG_INFO(formatString("Rcumode(hex)=%06X", start.rcumode()(0).getRaw()));
		LOG_INFO_STR("Creating subarray: " << start.name);

		itsCalServer->send(start);
	}
	break;

	case CAL_STARTACK: {
		CALStartackEvent ack(e);
		if (ack.status != CAL_Protocol::CAL_SUCCESS) {
			cerr << "Error: failed to start calibration" << endl;
			TRAN(beamctl::final);
		} else {
			cout << "Calserver accepted settings." << endl;
			TRAN(beamctl::create_beam);
		}
	}
	break;

	case F_DISCONNECTED: {
		port.close();
		cerr << "Error: unexpected disconnect" << endl;
		TRAN(beamctl::final);
	}
	break;

	case F_EXIT:
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
	break;
	}

	return status;
}

//
// create_beam(event, port)
//
GCFEvent::TResult beamctl::create_beam(GCFEvent& e, GCFPortInterface& port)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	//
	// Create a new subarray
	//
	switch (e.signal) {
	case F_ENTRY: {
		IBSBeamallocEvent 	alloc;
		alloc.beamName	   = BEAMCTL_BEAM + formatString("_%d", getpid());
		alloc.antennaSet   = itsAntSet;
		alloc.rcumask	   = getRCUMask();
		// assume beamletnumbers are right so the ring can be extracted from those numbers.
		// when the user did this wrong the BeamServer will complain.
		alloc.ringNr	   = m_beamlets.front() >= BEAMLET_RING_OFFSET;

		list<int>::iterator its = m_subbands.begin();
		list<int>::iterator itb = m_beamlets.begin();
		for (; its != m_subbands.end() && itb != m_beamlets.end(); ++its, ++itb) {
			if (((*itb) >= BEAMLET_RING_OFFSET ? 1 : 0) != alloc.ringNr) {	// all beamlets in the same ring?
				cerr << "Beamlet " << *itb << " does not lay in ring " << alloc.ringNr << endl;
				TRAN(beamctl::final);
				break;
			}
			alloc.allocation()[(*itb) % BEAMLET_RING_OFFSET] = (*its);
		}
		itsBeamServer->send(alloc);
		LOG_INFO_STR("name        : " << alloc.beamName);
		LOG_INFO_STR("antennaSet  : " << alloc.antennaSet);
		LOG_INFO_STR("rcumask     : " << alloc.rcumask);
		LOG_INFO_STR("ringNr      : " << alloc.ringNr);
//		LOG_INFO_STR(alloc.allocation);
	}
	break;

	case IBS_BEAMALLOCACK: {
		IBSBeamallocackEvent ack(e);
		if (ack.status != IBS_Protocol::IBS_NO_ERR) {
			cerr << "Error: " << errorName(ack.status) << endl;
			TRAN(beamctl::final);
		} else {
			cout << "BeamServer accepted settings" << endl;
			itsBeamHandle = ack.beamName;
			LOG_DEBUG(formatString("got beam_handle=%s for %s", itsBeamHandle.c_str(), ack.antennaGroup.c_str()));
		}
		
		// send HB analogue pointing?
		if (m_HBtype[0]) {
			send_direction(m_HBlongitude, m_HBlatitude, m_HBtype, true);
		}

		if (m_type[0]) {
			send_direction(m_longitude, m_latitude, m_type, false);
		}
		
	}
	break;

	case F_DISCONNECTED: {
		port.close();
		cerr << "Error: unexpected disconnect" << endl;
		TRAN(beamctl::final);
	}
	break;

	case F_EXIT:
	break;

	default:
		status = GCFEvent::NOT_HANDLED;
	break;
	}

	return status;
}

GCFEvent::TResult beamctl::final(GCFEvent& e, GCFPortInterface& /*port*/)
{
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch(e.signal) {
			case F_ENTRY:
		GCFScheduler::instance()->stop();
		break;
			
			case F_EXIT:
		break;

			default:
		status = GCFEvent::NOT_HANDLED;
		break;
	}

	return status;
}

void beamctl::send_direction(double	longitude, double	latitude, const string&	dirType, bool	isAnalogue)
{
	IBSBeampointtoEvent pointto;
	pointto.beamName = itsBeamHandle;
	pointto.pointing.setDirection(longitude, latitude);
	pointto.analogue = isAnalogue;
	if (dirType == "J2000") {
		pointto.pointing.setType(Pointing::J2000);
	} else if (dirType == "AZEL") {
		pointto.pointing.setType(Pointing::AZEL);
	} else if (dirType == "LOFAR_LMN") {
		pointto.pointing.setType(Pointing::LOFAR_LMN);
	} else if (dirType != "SKYSCAN") {
		LOG_FATAL_STR("Error: invalid coordinate type '" << dirType << "'");
		exit(EXIT_FAILURE);
	}

	if (dirType != "SKYSCAN") {
		itsBeamServer->send(pointto);
		return;
	}

	// SKYSCAN
	Timestamp time, end_time;
	time.setNow(SKYSCAN_STARTDELAY); // start after appropriate delay
	pointto.pointing.setType(Pointing::LOFAR_LMN);

	// step through l and m
	int l_steps = static_cast<int>(longitude);
	int m_steps = static_cast<int>(latitude);
	end_time = time + static_cast<long>(itsSkyScanTotalTime);

	double m_increment = 2.0 / (m_steps - 1);
	double l_increment = 2.0 / (l_steps - 1);
	double eps = 5.6e-16;

	do { 
		for (double m = -1.0; m <= 1.0 + eps; m += m_increment) {
			for (double l = -1.0; l <= 1.0 + eps; l+= l_increment) {
				if (l*l+m*m <= 1.0 + eps) {
					pointto.pointing.setTime(time);
					pointto.pointing.setDirection(l, m);
					itsBeamServer->send(pointto);
					time = time + static_cast<long>(itsSkyScanPointTime); // advance seconds
				}
			}
		}
		pointto.pointing.setTime(time);
		pointto.pointing.setDirection(0.0, 0.0);
		itsBeamServer->send(pointto);
		time = time + static_cast<long>(itsSkyScanWaitTime); // advance seconds
	} while (time < end_time);
}


void usage()
{
	cout <<
"Usage: beamctl\n"
"  --antennaset=name # name of the antenna (sub)field the RCU's are part of\n"
"  --rcus=<set>      # optional subselection of RCU's\n"
"  --rcumode=0..7    # RCU mode to use (may not conflict with antennaset\n"
"  --subbands=<set>  # set of subbands to use for this beam\n"
"  --beamlets=<list> # list of beamlets on which to allocate the subbands\n" 
"                    # beamlet range = 0..247 when Serdes splitter is OFF\n"
"                    # beamlet range = 0..247 + 1000..1247 when Serdes splitter is ON\n"
"  --direction=longitude,latitude[,type]\n"
"                    # lon,lat are floating point values specified in radians\n"
"                    # type is one of J2000 (default), AZEL, LOFAR_LMN, SKYSCAN\n"
"                    # SKYSCAN will scan the sky with a L x M grid in the (l,m) plane\n"
"  --hbdirection=longitude,latitude[,type]\n"
"                    # direction of the analogue HBA beam\n"
"  --help            # print this usage\n"
"\n"
"This utility connects to the CalServer to create a subarray of --array\n"
"containing the selected RCU's. The CalServer sets those RCU's in the mode\n"
"specified by --rcumode. Another connection is made to the BeamServer to create a\n"
"beam on the created subarray pointing in the direction specified with --direction.\n"
	<< endl;
}

bitset<LOFAR::MAX_RCUS> beamctl::getRCUMask() const
{
	bitset<LOFAR::MAX_RCUS> mask;
	
	mask.reset();
	list<int>::const_iterator it;
	int count = 0; // limit to ndevices
	for (it = m_rcus.begin(); it != m_rcus.end(); ++it, ++count) {
		if (count >= LOFAR::MAX_RCUS) break;
		
		if (*it < LOFAR::MAX_RCUS)
			mask.set(*it);
	}
	return mask;
}

static list<int> strtolist(const char* str, int max)
{
	string inputstring(str);
	char* start = (char*)inputstring.c_str();
	char* end   = 0;
	bool  range = false;
	long prevval = 0;
	list<int> resultset;

	resultset.clear();

	while (start)
	{
		long val = strtol(start, &end, 10); // read decimal numbers
		start = (end ? (*end ? end + 1 : 0) : 0); // advance
		if (val >= max || val < 0) {
			cerr << formatString("Error: value %ld out of range",val) << endl;
			resultset.clear();
			return resultset;
		}

		if (end) {
			switch (*end) {
				case ',':
				case 0: {
					if (range) {
						if (0 == prevval && 0 == val)
							val = max - 1;
						if (val < prevval) {
				cerr << "Error: invalid range specification" << endl;
							resultset.clear();
							return resultset;
						}
						for (long i = prevval; i <= val; i++)
							resultset.push_back(i);
					} 
					else {
						resultset.push_back(val);
					}
					range=false;
				}
				break;

				case ':':
					range=true;
					break;

				default:
		cerr << formatString("Error: invalid character %c",*end) << endl;
					resultset.clear();
					return resultset;
					break;
			}
		}
		prevval = val;
	}

	return resultset;
}

void printList(list<int>&		theList)
{
	list<int>::reverse_iterator		riter = theList.rbegin();
	list<int>::reverse_iterator		rend  = theList.rend();
	while (riter != rend) {
		cout << *riter;
		++riter;
	}
	cout << endl;
}

void beamctl::mainloop()
{
	start(); // make initial transition
	GCFScheduler::instance()->run();
}

int main(int argc, char** argv)
{
	char *array = "unset";              // --array argument goes here
	list<int> rcus;                     // --rcus argument goes here
	list<int> subbands;                 // --subbands argument goed here
	list<int> beamlets;                 // --beamlets argument goes here
	RSP_Protocol::RCUSettings rcumode;  // --rcumode argument goes here
	Pointing direction;                 // --direction argument goed here
	unsigned char presence = 0;         // keep track of which arguments have been specified

	double latitude  = 0.0;
	double longitude = 0.0;
	char   type[11]  = "J2000";
	double HBlatitude  = 0.0;
	double HBlongitude = 0.0;
	char   HBtype[11]  = "J2000";

	// initialize rcus
	rcus.clear();
	subbands.clear();
	beamlets.clear();

	// initialize rcumode
	rcumode().resize(1);
	
	cout << "Reading configuration files" << endl;
	try {
		LOFAR::ConfigLocator cl;
		globalParameterSet()->adoptFile(cl.locate("beamctl.conf"));
	}
	catch (LOFAR::Exception& e) {
		cerr << "Failed to load configuration files: " << e.text() << endl;
		exit(EXIT_FAILURE);
	}
		
	// parse options
	optind = 0; // reset option parsing
	while (1) {

		static struct option long_options[] = {
			{ "antennaset",	 required_argument, 0, 'a' },
			{ "rcus",      	 required_argument, 0, 'r' },
			{ "rcumode",   	 required_argument, 0, 'm' },
			{ "direction", 	 required_argument, 0, 'd' },
			{ "hbdirection", required_argument, 0, 'D' },
			{ "subbands",  	 required_argument, 0, 's' },
			{ "beamlets",  	 required_argument, 0, 'b' },
			{ "help",      	 no_argument,       0, 'h' },
			{ 0, 0, 0, 0 },
		};

		int option_index = 0;
		int c = getopt_long(argc, argv, "a:r:m:d:D:s:b:h", long_options, &option_index);

		if (c == -1) 
			break;

		switch (c) {
		case 'a': {
			if (!optarg) {
				cerr << "Error: missing --array value" << endl;
			} else {
				array=strdup(optarg);
				cout << "array=" << array << endl;
				presence |= ARRAY_FLAG;
			}
		}
		break;
		 
		case 'r': {
			if (!optarg) {
				cerr << "Error: missing --rcus value" << endl;
			} else {
				rcus = strtolist(optarg, LOFAR::MAX_RCUS);
				presence |= RCUS_FLAG;
				cout << "rcus=" << optarg << endl;
			}
		}
		break;

		case 'm': {
			if (!optarg) {
				cerr << "Error: missing --rcumode value" << endl;
			} else {
				int mode = 0;
				if ((mode = atoi(optarg)) < 0 || mode > 7) {
					cerr << formatString("Error: --rcumode=%d, out of range [0..7]", mode) << endl;
				} else {
					rcumode()(0).setMode((RSP_Protocol::RCUSettings::Control::RCUMode)mode);
					presence |= RCUMODE_FLAG;
					cout << "rcumode=" << optarg << endl;
				}
			}
		}
		break;

		case 'D': 
		case 'd': {
			if (!optarg) {
				cerr << "Error: missing --direction value" << endl;
			} else {
				double	lat, lon;
				char	dirType[20];
				int	nargs = sscanf(optarg, "%lf,%lf,%10s", &lon, &lat, dirType);
				if (nargs < 2) {
					cerr << "Error: invalid number of parameters for --direction " << endl;
				} else {
					cout << formatString("longitude=%lf, latitude=%lf, type=%s", lon, lat, dirType);

					if (string("J2000") == string(type)) {
						presence |= DIRECTION_FLAG;
					} else if (string("AZEL") == string(type)) {
						presence |= DIRECTION_FLAG;
					} else if (string("LOFAR_LMN") == string(type)) {
						presence |= DIRECTION_FLAG;
					} else if (string("SKYSCAN") == string(type)) {
						presence |= DIRECTION_FLAG;
					} else {
						cerr << "Error: invalid coordinate type '" << type << "'" << endl;
					}
					// move it to the right variables.
					if (c == 'd') {
						longitude = lon;
						latitude  = lat;
						strcpy(type, dirType);
					}
					else {
						HBlongitude = lon;
						HBlatitude  = lat;
						strcpy(HBtype, dirType);
					}
				}
			}
		}
		break;

		case 's': {
			if (!optarg) {
				cerr << "Error: missing --subbands value" << endl;
			} else {
				subbands = strtolist(optarg, LOFAR::MAX_SUBBANDS);
				presence |= SUBBANDS_FLAG;
				cout << "subbands = " << optarg << "(" << subbands.size() << ")" << endl;
			}
		}
		break;

		case 'b': {
			if (!optarg) {
				cerr << "Error: missing --beamlets value" << endl;
			} else {
				beamlets = strtolist(optarg, BEAMLET_RING_OFFSET + LOFAR::MAX_BEAMLETS);
				presence |= BEAMLETS_FLAG;
				cout << "beamlets = " << optarg << "(" << beamlets.size() << ")" << endl;
			}
		}
		break;

		case 'h': {
			usage();
			exit(EXIT_SUCCESS);
		}
		break;

		case '?':
		default: {
			//cerr << formatString("Error: unknown argument '%s'", optarg) << endl;
			exit(EXIT_FAILURE);
		}
		break;
		}
	}

	if (! (presence & ARRAY_FLAG)) {
		cerr << "Error: --array argument missing." << endl;
	}
	if (! (presence & RCUS_FLAG)) {
		cerr << "Error: --rcus argument missing." << endl;
	}
	if (! (presence & RCUMODE_FLAG)) {
		cerr << "Error: --rcumode argument missing." << endl;
	}
	if (! (presence & DIRECTION_FLAG)) {
		cerr << "Error: --direction argument missing." << endl;
	}
	if (! (presence & SUBBANDS_FLAG)) {
		cerr << "Error: --subbands argument missing." << endl;
	}
	if (! (presence & BEAMLETS_FLAG)) {
		cerr << "Error: --beamlets argument missing." << endl;
	}

	if (presence != ALL_FLAGS) {
		cerr << "Error: Not all required arguments have been specified correctly." << endl;
		usage();
		exit(EXIT_FAILURE);
	}

	if (subbands.size() != beamlets.size()) {
		cerr << "Error: length of --subbands value must be equal to length of --beamlets value." << endl;
		exit(EXIT_FAILURE);
	}

	cout << "Argument are ok, creating a task" << endl;
	GCFScheduler::instance()->init(argc, argv, "beamctl");
	LOG_INFO(formatString("Program %s has started", argv[0]));

	beamctl ctl("beamctl", array, rcus, subbands, beamlets, rcumode, 
								longitude, latitude, type, HBlongitude, HBlatitude, HBtype);

	try {
		ctl.mainloop();
	} catch (Exception& e) {
		cerr << "Exception: " << e.text() << endl;
		exit(EXIT_FAILURE);
	}

	LOG_INFO("Normal termination of program");

	return (0);
}
