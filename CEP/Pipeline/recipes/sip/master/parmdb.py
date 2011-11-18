#                                                         LOFAR IMAGING PIPELINE
#
#                                                                  parmdb recipe
#                                                            John Swinbank, 2010
#                                                      swinbank@transientskp.org
# ------------------------------------------------------------------------------

from __future__ import with_statement
import os
import sys
import subprocess
import shutil
import tempfile
import collections

from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.group_data import load_data_map
from lofarpipe.support.pipelinelogging import log_process_output
import lofarpipe.support.utilities as utilities
import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.parset import Parset

template = """
create tablename="%s"
adddef Gain:0:0:Ampl  values=1.0
adddef Gain:1:1:Ampl  values=1.0
adddef Gain:0:0:Real  values=1.0
adddef Gain:1:1:Real  values=1.0
adddef DirectionalGain:0:0:Ampl  values=1.0
adddef DirectionalGain:1:1:Ampl  values=1.0
adddef DirectionalGain:0:0:Real  values=1.0
adddef DirectionalGain:1:1:Real  values=1.0
adddef AntennaOrientation values=5.497787144
quit
"""

class parmdb(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    Add a parameter database to input MeasurementSets.

    This recipe is called by the :class:`bbs.bbs` recipe; it may also be used
    standalone.

    **Arguments**

    A mapfile describing the data to be processed.
    """
    inputs = {
        'executable': ingredient.ExecField(
            '--executable',
            help="Full path to parmdbm executable",
            default="/opt/LofIm/daily/lofar/bin/parmdbm"
        ),
        'nproc': ingredient.IntField(
            '--nproc',
            help="Maximum number of simultaneous processes per compute node",
            default=8
        ),
        'suffix': ingredient.StringField(
            '--suffix',
            help="Suffix of the table name of the empty parmameter database",
            default=".parmdb"
        ),
        'working_directory': ingredient.StringField(
            '-w', '--working-directory',
            help="Working directory used on output nodes. "
                 "Results will be written here."
        ),
        'mapfile': ingredient.StringField(
            '--mapfile',
            help="Full path of mapfile to produce; it will contain "
                 "a list of the generated empty parameter database files"
        )
    }

    outputs = {
        'mapfile': ingredient.FileField()
    }

    def go(self):
        self.logger.info("Starting parmdb run")
        super(parmdb, self).go()

        self.logger.info("Generating template parmdb")
        pdbdir = tempfile.mkdtemp(
            dir=self.config.get("layout", "job_directory")
        )
        pdbfile = os.path.join(pdbdir, self.inputs['suffix'])

        try:
            parmdbm_process = subprocess.Popen(
                [self.inputs['executable']],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE
            )
            sout, serr = parmdbm_process.communicate(template % pdbfile)
            log_process_output("parmdbm", sout, serr, self.logger)
        except OSError, e:
            self.logger.error("Failed to spawn parmdbm: %s" % str(e))
            return 1

        #                     try-finally block to always remove temporary files
        # ----------------------------------------------------------------------
        try:
            #                       Load file <-> compute node mapping from disk
            # ------------------------------------------------------------------
            self.logger.debug("Loading map from %s" % self.inputs['args'][0])
            data = load_data_map(self.inputs['args'][0])

            command = "python %s" % (self.__file__.replace('master', 'nodes'))
            outnames = collections.defaultdict(list)
            jobs = []
            for host, ms in data:
                outnames[host].append(
                    os.path.join(
                        self.inputs['working_directory'],
                        self.inputs['job_name'],
                        os.path.basename(ms) + self.inputs['suffix']
                    )
                )
                jobs.append(
                    ComputeJob(
                        host,
                        command,
                        arguments=[
                            pdbfile,
                            outnames[host][-1]
                        ]
                    )
                )
            self._schedule_jobs(jobs, max_per_node=self.inputs['nproc'])

        finally:
            self.logger.debug("Removing template parmdb")
            shutil.rmtree(pdbdir, ignore_errors=True)

        if self.error.isSet():
            self.logger.warn("Detected failed parmdb job")
            return 1
        else:
            self.logger.debug("Writing empty parameter database file: %s" %
                              self.inputs['mapfile'])
            Parset.fromDict(outnames).writeFile(self.inputs['mapfile'])
            self.outputs['mapfile'] = self.inputs['mapfile']
            return 0


if __name__ == '__main__':
    sys.exit(parmdb().main())
