//# Observation.cc: class for easy access to observation definitions
//#
//# Copyright (C) 2006
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/lofar_datetime.h>
#include <Common/StreamUtil.h>
#include <ApplCommon/Observation.h>
#include <Common/lofar_set.h>
#include <boost/format.hpp>

using boost::format;

namespace LOFAR {

//
// Observation()
//
Observation::Observation() :
	name(),
	obsID(0),
	startTime(0),
	stopTime(0),
	nyquistZone(0),
	sampleClock(0),
	splitterOn(false)
{
}

//
// Observation(ParameterSet*)
//
Observation::Observation(ParameterSet*		aParSet,
						 bool				hasDualHBA) :
	name(),
	obsID(0),
	startTime(0),
	stopTime(0),
	nyquistZone(0),
	sampleClock(0),
	splitterOn(false)
{
	// analyse ParameterSet.
	string prefix = aParSet->locateModule("Observation") + "Observation.";
	LOG_TRACE_VAR_STR("'Observation' located at: " << prefix);

	name  = aParSet->getString(prefix+"name", "");
	obsID = aParSet->getInt32("_treeID", 0);
	realPVSSdatapoint = aParSet->getString("_DPname","NOT_THE_REAL_DPNAME");
#if !defined HAVE_BGL
	try {
		if (aParSet->isDefined(prefix+"startTime")) {
			startTime = to_time_t(time_from_string(aParSet->getString(prefix+"startTime")));
		}
	} catch( boost::bad_lexical_cast ) {
		THROW( Exception, prefix << "startTime cannot be parsed as a valid time string. Please use YYYY-MM-DD HH:MM:SS[.hhh]." );
	}
	try {
		if (aParSet->isDefined(prefix+"stopTime")) {
			stopTime = to_time_t(time_from_string(aParSet->getString(prefix+"stopTime")));
		}
	} catch( boost::bad_lexical_cast ) {
		THROW( Exception, prefix << "stopTime cannot be parsed as a valid time string. Please use YYYY-MM-DD HH:MM:SS[.hhh]." );
	}
#endif
	if (aParSet->isDefined(prefix+"VirtualInstrument.stationList")) {
		stationList = aParSet->getString(prefix+"VirtualInstrument.stationList");
		stations    = aParSet->getStringVector(prefix+"VirtualInstrument.stationList", true);	// true:expandable
		std::sort(stations.begin(), stations.end());
	}

	sampleClock = aParSet->getUint32(prefix+"sampleClock",  0);
	filter 		= aParSet->getString(prefix+"bandFilter",   "");
	antennaArray= aParSet->getString(prefix+"antennaArray", "");
	MSNameMask  = aParSet->getString(prefix+"MSNameMask",   "");
	nyquistZone = nyquistzoneFromFilter(filter);

	// new way of specifying the receivers and choosing the antenna array.
	antennaSet  	 = aParSet->getString(prefix+"antennaSet", "");
	useLongBaselines = aParSet->getBool  (prefix+"longBaselines", false);

	// auto select the right antennaArray when antennaSet variable is used.
	if (!antennaSet.empty()) {
		antennaArray = antennaSet.substr(0,3);
	}
	splitterOn = ((antennaSet == "HBA_ZERO") || (antennaSet == "HBA_ONE") || (antennaSet == "HBA_DUAL"));
	dualMode   = (antennaSet == "HBA_DUAL");

	RCUset.reset();							// clear RCUset by default.
	if (aParSet->isDefined(prefix+"receiverList")) {
		receiverList = aParSet->getString(prefix+"receiverList");
		vector<uint32> RCUnumbers (aParSet->getUint32Vector(prefix+"receiverList", true));	// true:expandable
		for (uint i = 0; i < RCUnumbers.size();i++) {
			RCUset.set(RCUnumbers[i]);	// set mentioned receivers
		}
	}

	BGLNodeList     = compactedArrayString(aParSet->getString(prefix+"VirtualInstrument.BGLNodeList","[]"));
	storageNodeList = compactedArrayString(aParSet->getString(prefix+"VirtualInstrument.storageNodeList","[]"));

	// allocate beamlet 2 beam mapping and reset to -1
	beamlet2beams.resize   (MAX_BEAMLETS, -1);
	beamlet2subbands.resize(MAX_BEAMLETS, -1);
	// when nrSlotsInFrame differs from MAX_BEAMLETS_PER_RSP mark the gaps at the end of each RSPboard.
	nrSlotsInFrame = aParSet->getInt(prefix+"nrSlotsInFrame");
	if (nrSlotsInFrame != MAX_BEAMLETS_PER_RSP) {
		for (int rsp = 0; rsp < 4; rsp++) {
			for (int bl = nrSlotsInFrame; bl < MAX_BEAMLETS_PER_RSP; bl++) {
				beamlet2beams   [rsp*MAX_BEAMLETS_PER_RSP + bl] = 999;
				beamlet2subbands[rsp*MAX_BEAMLETS_PER_RSP + bl] = 999;
			}
		}
	}
	
	set<uint32> subbands;		
		
	//
	// NOTE: THE DATAMODEL USED IN SAS IS NOT RIGHT. IT SUPPORTS ONLY 1 POINTING PER BEAM.
	//		 THIS IS NO PROBLEM BECAUSE THAT IS WHAT WE AGREED.
	//		 WITHIN OBSERVATION HOWEVER WE USE THE RIGHT DATAMODEL THAT IS ALMOST THE SAME AS
	//		 THE BEAMSERVER USES.
	//

	// NOTE: One of the nasty problems we solve in this class is the duplication of a beam when it
	//		 is allocated on the HBA_DUAL antennaSet. For the user its one setting but for all software
	//		 it are two beams, each on a different antennaField. When HBA_DUAL is used in this observation
	//		 the corresponding beam(s) are added twice to the beam vectors, once on the HBA_ZERO antennaSet
	//		 once in the HBA_ONE antennaSet.

	// loop over all digital beams
	int32	nrBeams = aParSet->getInt32(prefix+"nrBeams", 0);		// theoretical number
	while (nrBeams > 0 && !aParSet->isDefined(prefix+formatString("Beam[%d].angle1", nrBeams-1))) {	// check reality
		nrBeams--;
	}
	for (int32 beamIdx(0) ; beamIdx < nrBeams; beamIdx++) {
		Beam	newBeam;
		string	beamPrefix(prefix+formatString("Beam[%d].", beamIdx));
		newBeam.momID	 = aParSet->getInt(beamPrefix+"momID", 0);
		newBeam.subbands = aParSet->getInt32Vector(beamPrefix+"subbandList", vector<int32>(), true);	// true:expandable
		newBeam.beamlets = aParSet->getInt32Vector(beamPrefix+"beamletList", vector<int32>(), true);	// true:expandable
		if (newBeam.subbands.size() != newBeam.beamlets.size()) {
			THROW (Exception, "Number of subbands(" << newBeam.subbands.size() << 
							  ") != number of beamlets(" << newBeam.beamlets.size() << 
							  ") in beam " << beamIdx);
		}
		newBeam.name = getBeamName(beamIdx);
		newBeam.antennaSet = antennaSet;
		if (dualMode) {
			newBeam.antennaSet = "HBA_ZERO";
			if (hasDualHBA) {
				newBeam.name += "_0";
			}
		}

		// ONLY one pointing per beam.
		Pointing		newPt;
		newPt.angle1 		= aParSet->getDouble(beamPrefix+"angle1", 0.0);
		newPt.angle2 		= aParSet->getDouble(beamPrefix+"angle2", 0.0);
		newPt.directionType = aParSet->getString(beamPrefix+"directionType", "");
		newPt.duration	    = aParSet->getInt(beamPrefix+"duration", 0);
		newPt.startTime 	= startTime;	// assume time of observation itself
#if !defined HAVE_BGL
		try {
			string	timeStr = aParSet->getString(beamPrefix+"startTime","");
			if (!timeStr.empty() && timeStr != "0") {
				newPt.startTime = to_time_t(time_from_string(timeStr));
			}
		} catch (boost::bad_lexical_cast) {
			LOG_ERROR_STR("Starttime of pointing of beam " << beamIdx << " not valid, using starttime of observation");
		}
#endif
		newBeam.pointings.push_back(newPt);

		// Finally add the beam to the vector
		beams.push_back(newBeam);
		
		// Duplicate beam on second HBA subfield when in HBA_DUAL mode.
		if (dualMode && hasDualHBA) {
			newBeam.name	   = getBeamName(beamIdx) + "_1";
			newBeam.antennaSet = "HBA_ONE";
			beams.push_back(newBeam);
		}

		// finally update beamlet 2 beam mapping.
		for (int32  i = newBeam.beamlets.size()-1 ; i > -1; i--) {
			if (beamlet2beams[newBeam.beamlets[i]] != -1) {
				stringstream	os;
				os << "beamlet2beams   : "; writeVector(os, beamlet2beams,    ",", "[", "]"); os << endl;
				os << "beamlet2subbands: "; writeVector(os, beamlet2subbands, ",", "[", "]"); os << endl << endl;
				LOG_ERROR_STR(os.str());
				THROW (Exception, "beamlet " << i << "(" << newBeam.beamlets[i] << ") of beam " << beamIdx << " clashes with beamlet of other beam"); 
			}
			beamlet2beams   [newBeam.beamlets[i]] = beamIdx;
			beamlet2subbands[newBeam.beamlets[i]] = newBeam.subbands[i];
			subbands.insert(newBeam.subbands[i]);
		}
	} // for

	// loop over al analogue beams
	int32	nrAnaBeams = aParSet->getInt32(prefix+"nrAnaBeams", 0);		// theoretical number
	while (nrAnaBeams > 0 && !aParSet->isDefined(prefix+formatString("AnaBeam[%d].angle1", nrAnaBeams-1))) {	// check reality
		nrAnaBeams--;
	}
	for (int32 beamIdx(0) ; beamIdx < nrAnaBeams; beamIdx++) {
		AnaBeam	newBeam;
		string	beamPrefix(prefix+formatString("AnaBeam[%d].", beamIdx));
		newBeam.rank	   = aParSet->getInt   (beamPrefix+"rank", 5);
		newBeam.name 	   = getAnaBeamName();
		newBeam.antennaSet = antennaSet;
		if (dualMode) {
			newBeam.antennaSet = "HBA_ZERO";
			if (hasDualHBA) {
				newBeam.name += "_0";
			}
		}

		// ONLY one pointing per beam.
		Pointing		newPt;
		newPt.angle1 	    = aParSet->getDouble(beamPrefix+"angle1", 0.0);
		newPt.angle2 		= aParSet->getDouble(beamPrefix+"angle2", 0.0);
		newPt.directionType = aParSet->getString(beamPrefix+"directionType", "");
		newPt.duration	    = aParSet->getInt   (beamPrefix+"duration", 0);
		newPt.startTime 	= startTime;	// assume time of observation itself
#if !defined HAVE_BGL
		try {
			string	timeStr = aParSet->getString(beamPrefix+"startTime","");
			if (!timeStr.empty() && timeStr != "0") {
				newPt.startTime = to_time_t(time_from_string(timeStr));
			}
		} catch (boost::bad_lexical_cast) {
			LOG_ERROR_STR("Starttime of pointing of analogue beam " << beamIdx << " not valid, using starttime of observation");
		}
#endif
		newBeam.pointings.push_back(newPt);
		
		// add beam to the analogue beam vector
		anaBeams.push_back(newBeam);

		// Duplicate beam on second HBA subfield when in HBA_DUAL mode.
		if (dualMode && hasDualHBA) {
			newBeam.name	   = getBeamName(beamIdx) + "_1";
			newBeam.antennaSet = "HBA_ONE";
			anaBeams.push_back(newBeam);
		}
	}

#if 0
        // loop over all data products and generate all data flows
	string olapprefix = aParSet->locateModule("OLAP") + "OLAP.";
        const char *dataProductNames[] = { "CoherentStokes", "IncoherentStokes", "Beamformed", "Correlated", "Filtered" };
        unsigned dataProductPhases[]   = { 3,                2,                  3,            2,            2          };
        size_t nrDataProducts = sizeof dataProductNames / sizeof dataProductNames[0];

        // by default, use all psets
        std::vector<unsigned> phaseTwoPsets;
        if (aParSet->isDefined(olapprefix+"CNProc.phaseTwoPsets"))
          phaseTwoPsets = aParSet->getUint32Vector(olapprefix+"CNProc.phaseTwoPsets", true);
        if (phaseTwoPsets.empty()) 
          for (unsigned p = 0; p < 64; p++)
            phaseTwoPsets.push_back(p);

        // by default, use all psets
        std::vector<unsigned> phaseThreePsets;
        if (aParSet->isDefined(olapprefix+"CNProc.phaseThreePsets"))
          phaseTwoPsets = aParSet->getUint32Vector(olapprefix+"CNProc.phaseThreePsets", true);
        if (phaseThreePsets.empty()) 
          for (unsigned p = 0; p < 64; p++)
            phaseThreePsets.push_back(p);

        for (size_t d = 0; d < nrDataProducts; d ++) {
          bool enabled = aParSet->getBool(prefix+str(format("DataProducts.Output_%s.enabled") % dataProductNames[d]), false);

          if (!enabled)
            continue;

          // phase 2: files are ordered by beam, subband  
          // phase 3: files are ordered by beam, pencil, stokes, part

          // The .locations parset value contains the storage nodes which
          // will store each file.

          // The I/O node will allocate the files in order depth-wise.
          // That is, we determine the maximum number of files to output per
          // pset, and then proceed to fill up the I/O nodes starting from
          // the first pset. Each data product is treated individually.

          std::vector<std::string> filenames = aParSet->getStringVector(prefix+str(format("DataProducts.Output_%s.filenames") % dataProductNames[d]));
          std::vector<std::string> locations = aParSet->getStringVector(prefix+str(format("DataProducts.Output_%s.locations") % dataProductNames[d]));
          std::vector<unsigned> &psets = dataProductPhases[d] == 2 ? phaseTwoPsets : phaseThreePsets;

          unsigned numFiles = filenames.size();
          unsigned filesPerPset = (numFiles + psets.size() - 1) / psets.size();

          for (size_t i = 0; i < filenames.size(); i++) {
            StreamToStorage a;

            a.dataProduct = dataProductNames[d];
            a.streamNr = i;
            a.filename = filenames[i];
            a.sourcePset = psets[i / filesPerPset];

            std::vector<std::string> locparts = StringUtil::split(locations[i],':');
            a.destStorageNode = locparts[0];
            a.destDirectory = locparts[1];

            streamsToStorage.push_back(a);
          }
        }
#endif
}


//
// ~Observation()
//
Observation::~Observation()
{
}

//
// conflicts (Observation& other)
//
// check if the given Observation conflicts with this one
bool	Observation::conflicts(const	Observation&	other) const
{
#if defined HAVE_BGL
	LOG_WARN("BG/P code cannot check for conflicts between observations!!!");
	return (false);
#endif

	// if observations don't overlap they don't conflict per definition.
	if ((other.stopTime <= startTime) || (other.startTime >= stopTime)) {
		return (false);
	}

	// Observation overlap, check clock
	if (other.sampleClock != sampleClock) {
		LOG_INFO_STR("Clock of observation " << obsID << " and " << other.obsID << " conflict");
		return (true);
	}

	// Observation overlap, check splitters
	if (other.splitterOn != splitterOn) {
		LOG_INFO_STR("Splitters of observation " << obsID << " and " << other.obsID << " conflict");
		return (true);
	}

	// if rcumode differs there may be no overlap in receivers.
	if (other.filter != filter) {
		RCUset_t	resultSet(RCUset & other.RCUset);
		if (resultSet.any()) {
			LOG_INFO_STR("Conflicting use of receivers between observation " << obsID << 
						 " and " << other.obsID);
			LOG_DEBUG_STR("receiverConflict: " << resultSet);
			return (true);
		}
	}

	// check beamlets overlap
	int		maxBeamlets = beamlet2beams.size();
	for (int bl = 0; bl < maxBeamlets; bl++) {
		if (beamlet2beams[bl] != -1 && other.beamlet2beams[bl] != -1) {
			LOG_INFO_STR("Conflict in beamlets between observation " << obsID <<
						 " and " << other.obsID);
			LOG_DEBUG_STR("First conflicting beamlet: " << bl);
			return (true);
		}
	}

	// for now also check nr of slots in frame. In the future we might allow
	// different slotsinFrame for each RSPboard but for now we treat it as a conflict.
	if (nrSlotsInFrame != other.nrSlotsInFrame) {
		LOG_INFO_STR("Conflict in nrSlotsInFrame: " << nrSlotsInFrame << "<->"  <<
					other.nrSlotsInFrame << " for resp. observation " << obsID << 
					" and " << other.obsID);
		return (true);
	}

	return (false);	// no conflicts
}

//
// getRCUbitset(nrLBAs, nrHBAs, antennaSet): bitset
//
// Returns a bitset containing the RCU's requested by the observation.
// The can be te list specified in the receiverList or (when empty) just some
// standin dummmy purely based on the number of antennas.
//
bitset<MAX_RCUS> Observation::getRCUbitset(int nrLBAs, int nrHBAs, const string& anAntennaSet)
{
	// user defined set always overrules.
	if (RCUset.any()) {
		return (RCUset);
	}

	// HBA's in Core stations sometimes use half of the rcus.
	int	nrRCUS = 2 * ((anAntennaSet.find("LBA") == 0) ? nrLBAs : nrHBAs);
	
	// Set up the RCUbits for this antennaSet. Remember that we don't care here which of the three inputs is used.
	RCUset_t	tmpRCUset(RCUset);
	for (int rcu = 0; rcu < nrRCUS; rcu++) {	// check all bits
		tmpRCUset.set(rcu);
	}
	return (tmpRCUset);
}

//
// TEMP HACK TO GET THE ANTENNAARRAYNAME
//
string Observation::getAntennaArrayName(bool hasSplitters) const
{
	string	result;
	if (antennaSet.empty()) {
		result = antennaArray;
	}
	else {
		result = antennaSet;
	}

	if (result.find("LBA") == 0)  {
		return ("LBA");
	}
	
	if (!hasSplitters) {		// no splitter, always use all HBA
		return ("HBA");
	}

	// station has splitters
	if (result == "HBA_ZERO") 	return ("HBA0");
	if (result == "HBA_ONE") 	return ("HBA1");
	if (result == "HBA_JOINED")	return ("HBA");
	return ("HBA");
}	

//
// getBeamName(beamidx): string
//
string Observation::getBeamName(uint32	beamIdx) const
{
	return (formatString("observation[%d]beam[%d]", obsID, beamIdx));
}
// there can only be one analogue beam
string Observation::getAnaBeamName() const
{
	return (formatString("observation[%d]anabeam", obsID));
}

//
// nyquistzoneFromFilter(filtername)
//
uint32 Observation::nyquistzoneFromFilter(const string&	filterName)
{
	// support of old names
	if (filterName == "LBL_10_80")		{ return(1); }
	if (filterName == "LBL_30_80")		{ return(1); }
	if (filterName == "LBH_10_80")		{ return(1); }
	if (filterName == "LBH_30_80")		{ return(1); }
	if (filterName == "HB_100_190") 	{ return(2); }
	if (filterName == "HB_170_230") 	{ return(3); }
	if (filterName == "HB_210_240") 	{ return(3); }
	if (filterName == "LBA_30_80")		{ return(1); }

	// support of new names
	if (filterName == "LBA_10_70")		{ return(1); }
	if (filterName == "LBA_30_70")		{ return(1); }
	if (filterName == "LBA_10_90")		{ return(1); }
	if (filterName == "LBA_30_90")		{ return(1); }
	if (filterName == "HBA_110_190") 	{ return(2); }
	if (filterName == "HBA_170_230") 	{ return(3); }
	if (filterName == "HBA_210_250") 	{ return(3); }

	LOG_ERROR_STR ("filterselection value '" << filterName << 
											"' not recognized, using LBA_10_90");
	return (1);
}

//
// print (os)
//
ostream& Observation::print (ostream&	os) const
{
	os << endl;
	os << "Observation  : " << name << endl;
    os << "ObsID        : " << obsID << endl;
#if !defined HAVE_BGL
    os << "starttime    : " << to_simple_string(from_time_t(startTime)) << endl;
    os << "stoptime     : " << to_simple_string(from_time_t(stopTime)) << endl;
#endif
    os << "stations     : " << stations << endl;
//    os << "stations     : "; writeVector(os, stations, ",", "[", "]"); os << endl;
    os << "antennaArray : " << antennaArray << endl;
    os << "antenna set  : " << antennaSet << endl;
    os << "receiver set : " << RCUset << endl;
    os << "sampleClock  : " << sampleClock << endl;
    os << "filter       : " << filter << endl;
    os << "splitter     : " << (splitterOn ? "ON" : "OFF") << endl;
    os << "nyquistZone  : " << nyquistZone << endl << endl;
    os << "Meas.set     : " << MSNameMask << endl << endl;

	os << "(Receivers)  : " << receiverList << endl;
	os << "Stations     : " << stationList << endl;
	os << "BLG nodes    : " << BGLNodeList << endl;
	os << "Storage nodes: " << storageNodeList << endl << endl;

    os << "nrBeams      : " << beams.size() << endl;
	for (size_t	b(0) ; b < beams.size(); b++) {
		os << "Beam[" << b << "].name       : " << beams[b].name << endl;
		os << "Beam[" << b << "].antennaSet : " << beams[b].antennaSet << endl;
		os << "Beam[" << b << "].momID      : " << beams[b].momID << endl;
		os << "Beam[" << b << "].subbandList: "; writeVector(os, beams[b].subbands, ",", "[", "]"); os << endl;
		os << "Beam[" << b << "].beamletList: "; writeVector(os, beams[b].beamlets, ",", "[", "]"); os << endl;
		os << "nrPointings : " << beams[b].pointings.size() << endl;
		for (size_t p = 0; p < beams[b].pointings.size(); ++p) {
			const Pointing*		pt = &(beams[b].pointings[p]);
#if defined HAVE_BGL
			os << formatString("Beam[%d].pointing[%d]: %f, %f, %s\n", b, p, pt->angle1, pt->angle2, pt->directionType.c_str());
#else
			os << formatString("Beam[%d].pointing[%d]: %f, %f, %s, %s\n", b, p, pt->angle1, pt->angle2, 
				pt->directionType.c_str(), to_simple_string(from_time_t(pt->startTime)).c_str());
#endif
		}
	}
	os << "beamlet2beams   : "; writeVector(os, beamlet2beams,    ",", "[", "]"); os << endl;
	os << "beamlet2subbands: "; writeVector(os, beamlet2subbands, ",", "[", "]"); os << endl << endl;

    os << "nrAnaBeams   : " << anaBeams.size() << endl;
	for (size_t	b(0) ; b < anaBeams.size(); b++) {
		os << "AnaBeam[" << b << "].name      : " << anaBeams[b].name << endl;
		os << "AnaBeam[" << b << "].antennaSet: " << anaBeams[b].antennaSet << endl;
		os << "AnaBeam[" << b << "].rank      : " << anaBeams[b].rank << endl;
		os << "nrPointings : " << anaBeams[b].pointings.size() << endl;
		for (uint p = 0; p < beams[b].pointings.size(); ++p) {
			const Pointing*		pt = &(anaBeams[b].pointings[p]);
			os << formatString("anaBeam[%d].pointing[%d]: %f, %f, %s\n", b, p, pt->angle1, pt->angle2, pt->directionType.c_str());
		}
	}

	return (os);
}


} // namespace LOFAR
