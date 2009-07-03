#!/usr/bin/env python

from LOFAR import Logger
from LOFAR import ObservationID
from LOFAR.Logger import debug,info,warning,error,fatal
from LOFAR import Sections
from LOFAR.Parset import Parset
from LOFAR.Stations import Stations
from util import Commands
from util.dateutil import format
from LOFAR.Locations import Locations,isDevelopment
from util.Hosts import ropen,rmkdir,rexists
import sys

DRYRUN = False

def print_exception( str ):
  import traceback

  print >>sys.stderr, str

  traceback.print_exc()

def extend_basedir( basedir, path ):
  """ Resolve a path relative to a certain base directory. """

  if path[:1] == "/":
    return path
  else:
    return "%s/%s" % (basedir,path)

def runObservation( parset, start_cnproc = True, start_ionproc = True, start_storage = True ):
  """ Run an observation using the provided parset. """

  # ----- Prepare the observation by writing the parset file and generating an observation ID

  # disable sections we won't start
  if not start_cnproc:
    parset.disableCNProc() 
  if not start_ionproc:
    parset.disableIONProc()
  if not start_storage:
    parset.disableStorage()

  # parse final settings (derive some extra keys)
  parset.finalise()

  # sanity check on parset
  parset.check()

  # write parset to disk (both rundir and logdir)
  parset.writeToFile( Locations.files["parset"] )
  parset.writeToFile( "%s/%s" % (Locations.files["logdir"],"RTCP.parset") )

  # TODO: remove this dependency hack, as everything is in one parset now
  os.system( "touch %s/OLAP.parset" % (Locations.files["rundir"]) )

  # ----- Select the sections to start
  sections = Sections.SectionSet()

  if start_cnproc:
    sections += [Sections.CNProcSection( parset )]
  if start_ionproc:
    sections += [Sections.IONProcSection( parset )]
  if start_storage:
    sections += [Sections.StorageSection( parset )]

  # sanity check on sections
  sections.check()

  # ----- Run all sections
  try:
    # start all sections
    sections.run()

    # wait for all sections to complete
    sections.wait()
  except KeyboardInterrupt:
    # abort all sections
    try:
      sections.abort(True) # soft kill
    except KeyboardInterrupt:
      sections.abort(False) # hard kill

  # let the sections clean up 
  sections.postProcess()

  info( "Done." )

if __name__ == "__main__":
  from optparse import OptionParser,OptionGroup
  import os
  import time

  # parse the command line
  parser = OptionParser( usage = "usage: %prog [options] parset parset ..." )
  parser.add_option( "-d", "--dry-run",
  			dest = "dryrun",
			action = "store_true",
			default = False,
  			help = "do not actually execute anything [%default]" )

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
  opgroup.add_option( "-l", "--log-server",
  			dest = "logserver",
			type = "string",
			default = Locations.nodes["logserver"],
  			help = "TCP address (IP:port) to send logging to [%default]" )
  parser.add_option_group( opgroup )


  psgroup = OptionGroup(parser, "Observation" )
  psgroup.add_option( "-o", "--option",
  			dest = "option",
			action = "append",
			type = "string",
  			help = "override a parset parameter (syntax: key=value) [%default]" )

  psgroup.add_option( "-s", "--starttime",
  			dest = "starttime",
			type = "string",
                        default = "+00:00:15",
  			help = "set the start time (syntax: timestamp, [YYYY-MM-DD] HH:MM[:SS], +seconds or +HH:MM[:SS]) [%default]" )

  psgroup.add_option( "-r", "--runtime",
  			dest = "runtime",
			type = "string",
                        default = "00:01:00",
  			help = "set the run time (syntax: seconds or HH:MM[:SS]) [%default]" )

  psgroup.add_option( "-t", "--tcp-input",
  			dest = "tcpinput",
			action = "store_true",
			default = False,
  			help = "station data will be transmitted over TCP [%default]" )

  psgroup.add_option( "-n", "--null-input",
  			dest = "nullinput",
			action = "store_true",
			default = False,
  			help = "station data will be generated by Blue Gene [%default]" )
  parser.add_option_group( psgroup )

  hwgroup = OptionGroup(parser, "Hardware" )
  hwgroup.add_option( "-S", "--stations",
			action = "append",
  			dest = "stations",
			type = "string",
  			help = "the station(s) to use [%default]" )
  hwgroup.add_option( "-P", "--partition",
  			dest = "partition",
			type = "string",
  			help = "name of the BlueGene partition [%default]" )
  hwgroup.add_option( "-D", "--storagenode",
  			dest = "storagenode",
			action = "append",
			type = "string",
  			help = "storage nodes to use [%s]" % (",".join(Locations.nodes["storage"]),) )
  parser.add_option_group( hwgroup )

  secgroup = OptionGroup(parser, "Sections" )
  secgroup.add_option( "--nocnproc",
  			dest = "nocnproc",
			action = "store_true",
			default = False,
			help = "disable the CNProc section [%default]" )
  secgroup.add_option( "--noionproc",
  			dest = "noionproc",
			action = "store_true",
			default = False,
			help = "disable the IONProc section [%default]" )
  secgroup.add_option( "--nostorage",
  			dest = "nostorage",
			action = "store_true",
			default = False,
			help = "disable the storage section [%default]" )
  parser.add_option_group( secgroup )

  dirgroup = OptionGroup(parser, "Directory and file locations")
  dirgroup.add_option( "--basedir",
  			dest = "basedir",
			default = Locations.files["basedir"],
			help = "base directory [%default]" )
  dirgroup.add_option( "--logdir",
  			dest = "logdir",
			default = Locations.files["logdir"],
			help = "log directory (syntax: [host:]path) [%default]" )
  dirgroup.add_option( "--rundir",
  			dest = "rundir",
			default = Locations.files["rundir"],
			help = "run directory [%default]" )
  dirgroup.add_option( "--cnproc",
  			dest = "cnproc",
			default = Locations.files["cnproc"],
			help = "CNProc executable [%default]" )
  dirgroup.add_option( "--ionproc",
  			dest = "ionproc",
			default = Locations.files["ionproc"],
			help = "IONProc executable [%default]" )
  dirgroup.add_option( "--storage",
  			dest = "storage",
			default = Locations.files["storage"],
			help = "Storage executable [%default]" )

  parser.add_option_group( dirgroup )

  # parse arguments
  (options, args) = parser.parse_args()

  if not args:
    parser.print_help()
    sys.exit(1)

  if options.verbose:
    Commands.debug = debug
    Logger.DEBUG = True

  if not options.quiet:
    DEBUG = True
    Sections.DEBUG = True
    Logger.VERBOSE = True

  if options.dryrun:
    DRYRUN = True
    Commands.DRYRUN = True
    ObservationID.DRYRUN = True

  # parse the parset file(s)
  parset = Parset()
  for f in args:
    try:
      info( "Reading parset %s..." % (f,) )
      parset.readFromFile( Locations.resolvePath( f ) )
    except IOError,e:
      error("ERROR: Cannot read parset file: %s" % (e,))

  # override parset with command-line values
  if options.partition is None:
    options.partition = parset.distillPartition()

    if options.partition:
      info( "Distilled partition %s from parset." % (options.partition,) )
    else:
      warning( "No BlueGene partition selected on command line or in parset." )

  parset.setPartition( options.partition )

  # set storage nodes
  if options.storagenode is not None:
    # treat values like list001,list002 as two nodes
    options.storagenode = ",".join(options.storagenode).split(",")
    Locations.nodes["storage"] = options.storagenode

  parset.setStorageNodes( Locations.nodes["storage"] )

  # set log server
  Locations.nodes["logserver"] = options.logserver

  # set stations
  if options.stations is None:
    options.stations = parset.distillStations()

    if options.stations:
      info( "Distilled stations %s from parset." % (",".join(options.stations),) )
    else:
      warning( "No stations or inputs selected on command line or in parset. " )

  # join multiple station options with + to let them be parsed together
  stationList = Stations.parse( "+".join(options.stations) )

  if options.tcpinput:
    # turn all inputs into tcp:*
    def tcpify( input ):
      if input.startswith("tcp:") or input.startswith("file:"):
        return input
      elif input.startswith("udp:"):
        return "tcp:"+input[4:]
      else:
        return "tcp:"+input

    for s in stationList:
      s.inputs = map( tcpify, s.inputs )

  if options.nullinput:
    # turn all inputs into null:
    for s in stationList:
      s.inputs = ["null:"] * len(s.inputs)

  parset.setStations( stationList )

  # set runtime
  parset.setStartRunTime( options.starttime, options.runtime )
  info( "Running from %s to %s." % (parset["Observation.startTime"], parset["Observation.stopTime"] ) )

  # parse specific parset values from the command line
  if options.option is not None:  
    for opt in options.option: 
      try:
        k,v = opt.split("=")

        parset[k] = v
      except ValueError,e:
        error("Cannot parse option %s: %s" % (opt,e,))

  # reserve an observation id
  try:
    obsid = ObservationID.ObservationID.generateID( parset )
  except IOError,e:
    print_exception("Could not generate observation ID: %s" % (e,))
    sys.exit(1)  

  info( "Observation ID %s" % (obsid,) )

  # resolve all paths now that parset is set up and the observation ID is known
  for opt in dirgroup.option_list:
    Locations.setFilename( opt.dest, getattr( options, opt.dest ) )
  Locations.resolveAllPaths( parset ) 

  # create log and directory if it does not exist
  for d in ["logdir","rundir"]:
    if not rexists(Locations.files[d]):
      warning( "Creating %s directory %s" % ( d, Locations.files[d], ) )

      if not DRYRUN:
        rmkdir( Locations.files[d] )

  # run the observation
  runObservation( parset, not options.nocnproc, not options.noionproc, not options.nostorage )

