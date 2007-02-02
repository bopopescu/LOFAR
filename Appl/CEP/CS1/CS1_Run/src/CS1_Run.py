#!/usr/bin/env python

import math
import time
import os
import sys
from optparse import OptionParser

from CS1_Hosts import *
from CS1_Parset import CS1_Parset
from CS1_RSPCtl import RSPCtl
from CS1_Sections import *
from CS1_Stations import *

def doObservation(obsID, parset):
    if False:
        rsp = RSPCtl(CS10LCU)
        rsp.initBoards(parset.getClockString())
        rsp.selectSubbands(parset.getFirstSubband())
        rsp.selectRCUs([0, 1, 2, 8, 9])
        rsp.setWG([2], 60e6)
    

    sections = [\
        DelayCompensationSection(parset, list012),
        InputSection(parset, liifen),
        BGLProcSection(parset, bglfen3, 'R000_128_0', '/bglhome2/lofarsystem/'),
        StorageSection(parset, listfen)
        #Flagger(parset, listfen)
        ]


    stepTime = float(parset['Observation.NSubbandSamples']) / parset['Observation.SampleRate']
    sz = int(math.ceil((parset['Observation.StopTime'] - parset['Observation.StartTime']) / stepTime))
    #print int(parset['Observation.StopTime']), int(parset['Observation.StartTime']), noRuns

    logdir = '/log/'
    if not os.access(logdir, os.W_OK):
        logdir = './'
    #parset.writeToFile(logdir + parset.getMSName().split('/')[-1] + '.parset')
    parset.writeToFile(logdir + obsID + '.parset')

    try:
        for section in sections:
            print ('Starting ' + section.package)
            runlog = logdir + obsID + '.' + section.getName() + '.runlog'

            # todo 27-10-2006 this is a temporary hack because storage doesn't close neatly.
            # This way all sections run longer than needed and storage stops before the rest does
            if not isinstance(section, StorageSection):
		noRuns = ((sz+15)&~15) + 16;
		section.run(runlog, noRuns)
            else:
	        noRuns = (sz+15)&~15
                section.run(runlog, noRuns)
            print 

        for section in sections:
            print ('Waiting for ' + section.package + ' to finish ...')
            ret = section.isRunSuccess()
            print (ret)
            print
    except KeyboardInterrupt:
        for s in section:
            print ('Aborting ' + section.package)
            s.abortRun()

if __name__ == '__main__':

    parser = OptionParser()

    # do not use the callback actions of the OptionParser, because we must make sure we read the parset before adding any variables
    parser.add_option('--parset'         , dest='parset'         , default='CS1.parset', type='string', help='name of the parameterset [%default]')
    parser.add_option('--clock'          , dest='clock'          , default='160MHz'    , type='string', help='clock frequency (either 160MHz or 200MHz) [%default]')
    parser.add_option('--subbands'       , dest='subbands'       , default='60MHz,8'   , type='string', help='freq of first subband and number of subbands to use [%default]')
    parser.add_option('--runtime'        , dest='runtime'        , default='600'       , type='int'   , help='length of measurement in seconds [%default]')
    parser.add_option('--starttime'     , dest='starttime', default=int(time.time() + 150), type='int', help='start of measurement in UTC seconds [now + 150s]')
    parser.add_option('--integrationtime', dest='integrationtime', default='60'        , type='int'   , help='length of integration interval in seconds [%default]')
    parser.add_option('--msname'         , dest='msname'                               , type='string', help='name of the measurement set')
    parser.add_option('--stationlist'    , dest='stationlist' , default='CS10_4dipoles', type='string', help='name of the station or stationconfiguration (see CS1_Stations.py) [%default]')
    parser.add_option('--fakeinput'      , dest='fakeinput'      , action='count'                     , help='do not really read from the inputs, but from memory')

    # parse the options
    (options, args) = parser.parse_args()

    # create the parset
    parset = CS1_Parset()    

    # todo: set following variables
    parset['General.SubbandsPerPset'] = 8
    parset['BGLProc.NodesPerPset'] = 8
    parset['BGLProc.PsetsPerCell'] = 1

    parset.readFromFile(options.parset)
    parset.setClock(options.clock)
    parset.setIntegrationTime(options.integrationtime)
    if options.msname:
        parset.setMSName(options.msname)

    # read the subbands
    parts = options.subbands.split(',')
    nsb = int(parts[-1])
    first = ','.join(parts[:-1])
    parset.setSubbands(first, nsb)

    # read the runtime (optional start in utc and the length of the measurement)
    parset.setInterval(options.starttime, options.runtime)
    
    # convert beamdirections from RA and Dec to Radians
    #parset.setBeamdir()

    # read the stations from CS1_Stations.py
    # todo: WARNING this is very dangerous, because there could be any code in the station string
    # the exec should probably be replaced by something safer, but this is only a temporary script
    # This way is convenient because a user can say CS10_4dipoles + CS01_4dipoles
    try:
        exec('stationList =' + options.stationlist)
    except:
        print 'Cannot parse station configuration: ' + str(options.stationlist)
        sys.exit(1)
        
    parset.setStations(stationList)

    # see if we are using fake input
    if options.fakeinput > 0:
        parset.setInterval(1, options.runtime+10)
        parset.setInputToMem()

    # if the msname wasn't given, read the next number from the file
    runningNumberFile = '/log/nextMSNumber'
    MSdatabaseFile = '/log/MSList'

    if not 'Storage.MSNames' in parset:
        try:
            inf = open(runningNumberFile, 'r')
            measurementnumber = int(inf.readline())
            inf.close()

            # MS name is L<yyyy>_<nnnnn>_<mmm>.MS
            # the <mmm> is filled in by the subbandwriter
            year = str(time.gmtime()[0])
            MSNumber = '/data/L' + year + '_' + '%05d' % measurementnumber
	    MSName = MSNumber + '.MS'
	    if (parset.getInt32('General.SubbandsPerPset') == 1):
	        MSName = '\'' + MSNumber + '_SB%01d' % 0 + '.MS' + '\''
	        for i in range(1, parset.getInt32('Observation.NSubbands')):
                    MSName = MSName + ', ' + '\'' + MSNumber + '_SB%01d' % i + '.MS' + '\''
	    else:
		MSName = '\'' + MSNumber + '_SB%01d' % 0 + '-%01d' % (parset.getInt32('General.SubbandsPerPset') - 1) +'.MS' + '\''
		for i in range(1, (parset.getInt32('Observation.NSubbands')/parset.getInt32('General.SubbandsPerPset'))):
		    first = i * parset.getInt32('General.SubbandsPerPset')
		    last =  first + (parset.getInt32('General.SubbandsPerPset') -1)
		    MSName = MSName + ', ' + '\'' + MSNumber + '_SB%01d' % first + '-%01d' % last +'.MS' + '\''
            outf = open(runningNumberFile, 'w')
            outf.write(str(measurementnumber + 1) + '\n')
            outf.close()

            dbfile = open(MSdatabaseFile, 'a')
            nodesStr = str([1] * parset.getNCells() + [0] * (12 - parset.getNCells()))[1:-1]
            dateStr = time.strftime('%Y %0m %0d', time.gmtime())
            dbfile.write(MSNumber + '\t' + dateStr + '\t' + nodesStr + '\n')
            dbfile.close()
        except:
            MSName = '/data/Test.MS'
	    print 'Error: please start CS1_Run.py from host: LISTFEN'
	    sys.exit(1)
        parset['Storage.MSNames'] = '[' + MSName + ']'
    
    obsID = parset['Storage.MSNames'].strip('.MS').split('/')[-1]
    obsID = obsID.split('_')
    obsID = obsID[0] + '_' + obsID[1] 

    # start the observation
    doObservation(obsID, parset)
