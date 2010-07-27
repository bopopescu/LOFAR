#!/usr/bin/python
import sys
sys.path = sys.path + ["../src"]

from LOFAR.ParsetTester import testParset
from LOFAR.LogValidators import NoErrors
from LOFAR.Locations import Locations
from LOFAR import Logger

parsetFile = "RTCP-validate.parset"

if __name__ == "__main__":
  from optparse import OptionParser,OptionGroup
  import os
  import sys

  # parse the command line
  parser = OptionParser( usage = """usage: %prog -P partition [options]""" )

  opgroup = OptionGroup(parser, "Output" )
  opgroup.add_option( "-v", "--verbose",
                        dest = "verbose",
                        action = "store_true",
                        default = False,
                        help = "be verbose [%default]" )
  opgroup.add_option( "-q", "--quiet",
                        dest = "quiet",
                        action = "store_true",
                        default = False,
                        help = "be quiet [%default]" )
  opgroup.add_option( "-k", "--keeplogs",
                        dest = "keeplogs",
                        action = "store_true",
                        default = False,
                        help = "keep log files and data products of successful tests [%default]" )
  parser.add_option_group( opgroup )

  hwgroup = OptionGroup(parser, "Hardware" )
  hwgroup.add_option( "-P", "--partition",
                        dest = "partition",
                        type = "string",
                        help = "name of the BlueGene partition [%default]" )
  parser.add_option_group( hwgroup )

  dirgroup = OptionGroup(parser, "Directory and file locations")
  dirgroup.add_option( "--basedir",
  			dest = "basedir",
			default = Locations.files["basedir"],
			help = "base directory [%default]" )
  dirgroup.add_option( "--logdir",
  			dest = "logdir",
			default = "%s/VALIDATION-${TIMESTAMP}" % (os.getcwd(),),
			help = "log directory (syntax: [host:]path) [%default]" )
  dirgroup.add_option( "--rundir",
  			dest = "rundir",
			default = "${LOGDIR}",
			help = "run directory [%default]" )
  dirgroup.add_option( "--cnproc",
  			dest = "cnproc",
			default = Locations.files["cnproc"],
			help = "CNProc executable [%default]" )
  dirgroup.add_option( "--ionproc",
  			dest = "ionproc",
			default = Locations.files["ionproc"],
			help = "IONProc executable [%default]" )
  parser.add_option_group( dirgroup )

  testgroup = OptionGroup(parser, "Tests to run (or all tests if nothing is specified)")
  testgroup.add_option( "--clock",
  			dest = "clock",
                        action = "store_true",
                        default = False,
			help = "run clock tests" )
  testgroup.add_option( "--oneoutput",
  			dest = "oneoutput",
                        action = "store_true",
                        default = False,
			help = "run tests for individual outputs" )
  testgroup.add_option( "--subbandrun",
  			dest = "subbandrun",
                        action = "store_true",
                        default = False,
			help = "run tests for varying number of subbands" )
  testgroup.add_option( "--beamrun",
  			dest = "beamrun",
                        action = "store_true",
                        default = False,
			help = "run tests for varying number of beams" )
  parser.add_option_group( testgroup )

  # parse arguments
  (options, args) = parser.parse_args()

  if not options.partition:
    parser.print_help()
    sys.exit(1)

  Logger.initLogger()  

  for opt in dirgroup.option_list:
    Locations.setFilename( opt.dest, getattr( options, opt.dest ) )

  Locations.resolveAllPaths()

  run_all = not reduce( lambda x,y: x or options[y.dest], testgroup.option_list, True )

  # clocks
  if run_all or options.clock:
    for clock in [160,200]:
      if not testParset( "%d MHz clock" % (clock,), options.partition, "parset=%s" % (parsetFile,), { "Observation.sampleClock": clock }, [NoErrors()], cleanup = not options.keeplogs ):
        sys.exit(1)

  # individual outputs
  if run_all or options.oneoutput:
    for output in ["CorrelatedData","CoherentStokes","IncoherentStokes"]:
      if not testParset( "output %s only" % (output,), options.partition, "parset=%s" % (parsetFile,), { "Observation.output%s" % (output,): True }, [NoErrors()], cleanup = not options.keeplogs ):
        sys.exit(1)

  # test 2 outputs, various number of subbands (for 2nd transpose)
  if run_all or options.subbandrun:
    nrBeams = 1

    for nrSubbands in [1,2,3,4,8,10,11,13,16,32,62,63,64,128,248]:
      if nrSubbands < nrBeams:
        continue

      subbands =  [i     for i in xrange(0,nrSubbands)]
      beams =     [0     for i in xrange(0,nrSubbands)]
      rspboards = [i//62 for i in xrange(0,nrSubbands)]
      rspslots  = [i%62  for i in xrange(0,nrSubbands)]

      override_keys = {
             "Observation.subbandList":    subbands,
             "Observation.beamList":       beams,
             "Observation.rspBoardList":   rspboards,
             "Observation.rspSlotList":    rspslots,

             "Observation.outputCorrelatedData": True,
             "Observation.outputCoherentStokes": True,
            }

      if not testParset( "%d subbands" % (nrSubbands,), options.partition, "parset=%s" % (parsetFile,), override_keys, [NoErrors()], cleanup = not options.keeplogs ):
        sys.exit(1)

  # test 2 outputs, various number of subbands (for 2nd transpose), multiple beams
  if run_all or options.beamrun:
    for nrBeams in [2,4,7,9]:
      for nrSubbands in [1,2,3,4,8,10,11,13,16,32,62,63,64,128,248]:
        if nrSubbands < nrBeams:
          continue

        subbands =  [i     for i in xrange(0,nrSubbands)]
        beams =     [0     for i in xrange(0,nrSubbands)]
        rspboards = [i//62 for i in xrange(0,nrSubbands)]
        rspslots  = [i%62  for i in xrange(0,nrSubbands)]

        override_keys = {
               "Observation.subbandList":    subbands,
               "Observation.beamList":       beams,
               "Observation.rspBoardList":   rspboards,
               "Observation.rspSlotList":    rspslots,

               "Observation.outputCorrelatedData": True,
               "Observation.outputCoherentStokes": True,
               "Observation.nrPencils":            nrBeams - 1,
              }

        for n in xrange(0,nrBeams):
          override_keys["Observation.Pencil[%d].angle1" % (n,)] = 0
          override_keys["Observation.Pencil[%d].angle2" % (n,)] = 0

        if not testParset( "%d beams %d subbands" % (nrBeams,nrSubbands), options.partition, "parset=%s" % (parsetFile,), override_keys, [NoErrors()], cleanup = not options.keeplogs ):
          sys.exit(1)

