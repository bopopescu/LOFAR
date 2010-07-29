import Logger
from logging import debug,info,warning,error,critical

from Core import runCorrelator,buildParset
from Parset import Parset
from CommandClient import sendCommand
from LogValidators import ValidationError
from Locations import Locations
from Partitions import PartitionPsets
from util.Commands import SyncCommand
from threading import Thread
from time import sleep
import os

class ParsetTester:
  """ Tests a parset by starting the correlator in a separate process and inserting the parset. 

      Usage example:

         pt = ParsetTester( "RTCP.parset", partition, "name of test" )

         # adjust the parset at will -- it has been read but not post-processed
         pt.setNrSubbands( 248 )
         pt.parset["Observation.outputCorrelatedData"] = True

         # run it and validate the logs
         pt.runParset()
         pt.validate( [NoErrors()] )

         # clean up logs and data products (if desired)
         pt.cleanup()
  """       

  def __init__( self, parsetFilename, partition, testname ):
    self.partition = partition
    self.testname = testname

    # configure the correlator before the parsets are added such that they will use the right paths
    testfilename = self.testname

    for c in " :":
      testfilename = testfilename.replace(c,"_")

    self.logdir = "%s/%s" % (Locations.files["logdir"],testfilename)

    self.results = {
      "started":    False,
      "terminated": False,
      "logdir":     self.logdir,
    }

    self.parset = Parset()
    self.olapparset_filename = "%s/../OLAP.parset" % (os.path.dirname(__file__),)
    self.rtcpparset_filename = parsetFilename

    for f in [self.olapparset_filename, self.rtcpparset_filename]:
      self.parset.readFile( Locations.resolvePath( f ) )

  def setNrSubbands( self, nrSubbands ):
    """ Use subbands 0 .. nrSubbands. """

    subbands =  [i     for i in xrange(0,nrSubbands)]
    beams =     [0     for i in xrange(0,nrSubbands)]
    rspboards = [i//62 for i in xrange(0,nrSubbands)]
    rspslots  = [i%62  for i in xrange(0,nrSubbands)]

    override_keys = {
           "Observation.subbandList":    subbands,
           "Observation.beamList":       beams,
           "Observation.rspBoardList":   rspboards,
           "Observation.rspSlotList":    rspslots,
            }

    for k,v in override_keys.iteritems():
      self.parset[k] = v

  def setNrStations( self, nrStations ):
    """ Use fake stations 0 .. nrStations which map to this partition. Uses at most |partition| stations. """

    psets = PartitionPsets[self.partition]
    ipsuffixes = [ip.split(".")[3] for ip in psets]
    stations = ["S%s" % (s,) for s in ipsuffixes]

    self.parset.forceStations( stations[:nrStations] )


  def setNrPencilBeams( self, nrBeams ):
    """ Use nrBeams fake pencil beams. """

    self.parset["Observation.nrPencils"] = nrBeams - 1
    for n in xrange(0,nrBeams):
      self.parset["Observation.Pencil[%d].angle1" % (n,)] = 0
      self.parset["Observation.Pencil[%d].angle2" % (n,)] = 0

  def runParset( self, starttimeout = 30, runtime = 60, stoptimeout = 120 ):
    # finalise and check parset BEFORE we start doing anything fancy
    self.parset = buildParset( self.parset, "", "start=+10,run=%d" % (runtime,), self.partition )
    self.parset.preWrite()
    self.parset.check()

    self.results["started"] = True

    class CorrelatorThread(Thread):
      def __init__(self,partition):
        Thread.__init__(self)
        self.partition = partition

      def run(self):
        runCorrelator( self.partition )

    info( "********** Starting test '%s' **********" % (self.testname,) )

    try:
      # start the correlator (ni a separate process to allow full control in runCorrelator
      info( "Starting correlator." )
      pid = os.fork()
      if pid == 0:
        # child process
        try:
          Locations.files["logdir"] = self.logdir
          info("Logdir = %s" % (self.logdir,))
          os.makedirs( self.logdir )

          runCorrelator( self.partition )
        except:
          error( "Correlator aborted." )
          os._exit(1)
        else:  
          info( "Correlator stopped." )
          os._exit(0)
      else:
        # parent process
        for i in xrange( starttimeout ):
          sleep( 1 )
          try:
            sendCommand( self.partition, "" )
          except:
            continue
          else:  
            break
        else:
          raise Exception("Correlator did not start.")

      # inject the parset
      self.parset.setFilename( Locations.resolvePath( "%s/RTCP-${MSNUMBER}.parset" % (self.logdir,), self.parset ) );
      info( "Sending parset '%s' to correlator." % (self.parset.filename,) )
      self.parset.save()
      sendCommand( self.partition, "parset %s" % (self.parset.filename,) )

      # quit immediately after processing the parset
      sendCommand( self.partition, "quit" )

      # wait for correlator to finish
      def isStopped():
        ret = os.waitpid( pid, os.WNOHANG )

        if ret[1] > 0:
          raise Exception("Correlator did not start.")

        return ret != (0,0)

      for i in xrange( runtime + stoptimeout ):
        sleep( 1 )

        if isStopped():
          stopped = True
          break
      else:  
        stopped = False

      # process outcome
      if stopped:
        info( "Correlator terminated succesfully." );
        self.results["terminated"] = True
      else:  
        error( "Correlator did not terminate." );
    except Exception,e:
      error( "Exception: %s" % (e,) )

  def validate(self,validators,continue_on_error = False):
     """ Run a set of validators on the logfiles produced by an earlier runParset run. """

     valid = True
     logfiles = ["run.CNProc.log","run.IONProc.log"]

     for v in validators:
       v.begin()

     for f in logfiles:
       fname = "%s/%s" % (self.results["logdir"],f)

       try:
         fd = file(fname)
       except IOError,e:
         error( "Could not open %s: %s" % (fname,e) )
         valid = False
       else:
         for linenr,l in enumerate(fd):
           try:
             v.parse(l)
           except ValidationError,e:
             error( "Validation error in %s:%s: %s" % (fname,linenr,e) )
             error( "Offending line: %s" % (l,) )
             valid = False

             if not continue_on_error:
               return

     for v in validators:
       try:
         v.end()
       except ValidationError,e:
         error( "Validation error in %s: %s" % (fname,e) )
         valid = False

         if not continue_on_error:
           return

     if valid and self.results["terminated"]:
       info( "********** Test '%s': OK **********" % (self.testname,) )
       return True
     else:  
       error( "********** Test '%s': Not OK **********" % (self.testname,) )
       return False

  def cleanup(self):
    # clean up logs and parsets
    info( "Removing log files in %s." % (self.logdir,) )
    SyncCommand("rm -f %s/run.CNProc.log %s/run.IONProc.log" % (self.logdir,self.logdir))

    info( "Removing parset %s." % (self.parset.filename,) )
    SyncCommand("rm -f %s" % (self.parset.filename,))

    info( "Removing log directory %s." % (self.logdir,) )
    SyncCommand("rmdir %s" % (self.logdir,))

    # clean up data products
    dataMask = self.parset.parseMask()
    dataMaskParts = dataMask.split("/")
    dataDir = "/".join(dataMaskParts[0:-1])

    if len(dataMaskParts) >= 4: # safety
      for storageNode in self.parset["OLAP.OLAP_Conn.IONProc_Storage_ServerHosts"]:
        info( "Removing data in %s:%s" % (storageNode,dataDir) )
        SyncCommand("ssh %s rm -rf %s" % (storageNode,dataDir))
    else:    
      warning( "Not removing data in %s:%s" % (storageNode,dataDir) )
