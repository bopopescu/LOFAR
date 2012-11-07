#                                                         LOFAR IMAGING PIPELINE
#
#                                  Master recipe to export calibration solutions
#                                                             Marcel Loose, 2011
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------
from __future__ import with_statement
import os
import sys

import lofarpipe.support.lofaringredient as ingredient

from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.group_data import load_data_map, store_data_map
from lofarpipe.support.group_data import validate_data_maps


class gainoutliercorrection(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    Recipe to correct outliers in the gain solutions of an parmdb,
    using the program `parmexportcal`   
    The main purpose of this program is to strip off the time axis information
    from a instrument model (a.k.a ParmDB)
    -or-
    a minimal implementation of the edit_parmdb program. Search all gains for
    outliers and swap these for the median

    1. Validate input
    2. load mapfiles, validate if a target output location is provided
    3. Call node side of the recipe
    4. validate performance, return corrected files

    **Command line arguments**

    1. A mapfile describing the data to be processed.
    2. A mapfile with target location <mapfiles are validated if present>
    
    """
    inputs = {
        'executable': ingredient.StringField(
            '--executable',
            default="",
            help="Full path to the `parmexportcal` executable, not settings this"
            " results in edit_parmdb behaviour"
        ),
        'suffix': ingredient.StringField(
            '--suffix',
            help="Suffix of the table name of the instrument model",
            default=".instrument"
        ),
        'working_directory': ingredient.StringField(
            '-w', '--working-directory',
            help="Working directory used on output nodes. "
                 "Results will be written here."
        ),
        'mapfile': ingredient.StringField(
            '--mapfile',
            help="Full path of mapfile to produce; it will contain "
                 "a list of the generated instrument-model files"
        ),
        'sigma': ingredient.FloatField(
            '--sigma',
            default=1.0,
            help="Clip at sigma * median: activates 'edit_parmdb' functionality"
        )
    }

    outputs = {
        'mapfile': ingredient.FileField(help="mapfile with corrected parmdbs")
    }


    def go(self):
        super(gainoutliercorrection, self).go()
        self.logger.info("Starting gainoutliercorrection run")
        # ********************************************************************
        # 1. Validate input
        # if sigma is none use default behaviour and use executable: test if
        # It excists
        executable = self.inputs['executable']
        if executable == "":
            pass
        elif not os.access(executable, os.X_OK):
            self.logger.warn(
                "No parmexportcal excecutable is not found on the suplied"
                "path: {0}".format(self.inputs['executable']))
            self.logger.warn("Defaulting to edit_parmdb behaviour")

        # ********************************************************************
        # 2. load mapfiles, validate if a target output location is provided
        args = self.inputs['args']
        self.logger.debug("Loading input-data mapfile: %s" % args[0])
        indata = load_data_map(args[0])
        if len(args) > 1:
            self.logger.debug("Loading output-data mapfile: %s" % args[1])
            outdata = load_data_map(args[1])
            if not validate_data_maps(indata, outdata):
                self.logger.error(
                    "Validation of input/output data mapfiles failed"
                )
                return 1
        else:
            outdata = [
                (host,
                 os.path.join(
                    self.inputs['working_directory'],
                    self.inputs['job_name'],
                    (os.path.splitext(os.path.basename(infile))[0] +
                     self.inputs['suffix']))
                 ) for host, infile in indata
            ]

        # ********************************************************************
        # 3. Call node side of the recipe
        command = "python %s" % (self.__file__.replace('master', 'nodes'))
        jobs = []
        for host, infile, outfile in (x + (y[1],)
            for x, y in zip(indata, outdata)):
            jobs.append(
                ComputeJob(
                    host,
                    command,
                    arguments=[
                        infile,
                        outfile,
                        self.inputs['executable'],
                        self.environment,
                        self.inputs['sigma']
                     ]
                )
            )
        self._schedule_jobs(jobs)

        # ********************************************************************
        # 4. validate performance, return corrected files
        if self.error.isSet():
            self.logger.warn("Detected failed gainoutliercorrection job")
            return 1
        else:
            self.logger.debug("Writing instrument map file: %s" %
                              self.inputs['mapfile'])
            store_data_map(self.inputs['mapfile'], outdata)
            self.outputs['mapfile'] = self.inputs['mapfile']
            return 0


if __name__ == '__main__':
    sys.exit(gainoutliercorrection().main())