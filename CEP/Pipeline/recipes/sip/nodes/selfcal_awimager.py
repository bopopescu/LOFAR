# LOFAR AUTOMATIC IMAGING PIPELINE
# awimager
# The awimager recipe creates based an image of the field of view. Based on
# nine concatenated and measurementsets each spanning 10 subbands
# The recipe contains two parts: The call to awimager
# and secondairy some functionality that calculates settings (for awimager)
# based on the information present in the measurement set
# The calculated parameters are:
#        1: The cellsize
#        2: The npixels in a each of the two dimension of the image
#        3. What columns use to determine the maximum baseline
#        4. The number of projection planes
# Wouter Klijn 2012
# klijn@astron.nl
# Nicolas Vilchez, 2014
# vilchez@astron.nl
# -----------------------------------------------------------------------------
from __future__ import with_statement
import sys
import shutil
import os.path
import math
import pyfits

from lofarpipe.support.lofarnode import LOFARnodeTCP
from lofarpipe.support.pipelinelogging import CatchLog4CPlus
from lofarpipe.support.pipelinelogging import log_time
from lofarpipe.support.utilities import patch_parset
from lofarpipe.support.utilities import get_parset
from lofarpipe.support.utilities import catch_segfaults
from lofarpipe.support.lofarexceptions import PipelineException
import pyrap.tables as pt  # @UnresolvedImport
from subprocess import CalledProcessError
from lofarpipe.support.utilities import create_directory
import pyrap.images as pim  # @UnresolvedImport
from lofarpipe.support.parset import Parset
import lofar.parmdb  # @UnresolvedImport
import numpy as np


class selfcal_awimager(LOFARnodeTCP):
    def run(self, executable, environment, parset, working_directory,
            output_image, concatenated_measurement_set, sourcedb_path,
             mask_patch_size, autogenerate_parameters, specify_fov, fov, 
             major_cycle, nr_cycles, perform_self_cal):
        """
        :param executable: Path to awimager executable
        :param environment: environment for catch_segfaults (executable runner)
        :param parset: parameters for the awimager,
        :param working_directory: directory the place temporary files
        :param output_image: location and filesname to story the output images
          the multiple images are appended with type extentions
        :param concatenated_measurement_set: Input measurement set
        :param sourcedb_path: Path the the sourcedb used to create the image 
          mask
        :param mask_patch_size: Scaling of the patch around the source in the 
          mask
        :param autogenerate_parameters: Turns on the autogeneration of: 
           cellsize, npix, wprojplanes, wmax, fov
        :param fov: if  autogenerate_parameters is false calculate 
           imageparameter (cellsize, npix, wprojplanes, wmax) relative to this 
           fov
        :param major_cycle: number of the self calibration cycle to determine 
            the imaging parameters: cellsize, npix, wprojplanes, wmax, fov            
        :param nr_cycles: The requested number of self cal cycles           
        :param perform_self_cal: Bool used to control the selfcal functionality
            or the old semi-automatic functionality       
        :rtype: self.outputs["image"] The path to the output image
        """
        self.logger.info("Start selfcal_awimager node run:")
        log4_cplus_name = "selfcal_awimager"
        self.environment.update(environment)

        with log_time(self.logger):
            # Read the parameters as specified in the parset
            parset_object = get_parset(parset)

            # *************************************************************
            # 1. Calculate awimager parameters that depend on measurement set
            # and the parset
            if perform_self_cal:
                # Calculate awimager parameters that depend on measurement set
                # and the parset              
                self.logger.info(
                   "Calculating selfcalibration parameters  ")
                cell_size, npix, w_max, w_proj_planes, \
                   UVmin, UVmax, robust, threshold =\
                        self._get_selfcal_parameters(
                            concatenated_measurement_set,
                            parset, major_cycle, nr_cycles) 

                self._save_selfcal_info(concatenated_measurement_set, 
                                        major_cycle, npix, UVmin, UVmax)

            else:
                self.logger.info(
                   "Calculating parameters.. ( NOT selfcalibration)")
                cell_size, npix, w_max, w_proj_planes = \
                    self._get_imaging_parameters(
                            concatenated_measurement_set,
                            parset,
                            autogenerate_parameters,
                            specify_fov,
                            fov)

            self.logger.info("Using autogenerated parameters; ")
            self.logger.info(
                 "Calculated parameters: cell_size: {0}, npix: {1}".format(
                     cell_size, npix))

            self.logger.info("w_max: {0}, w_proj_planes: {1} ".format(
                        w_max, w_proj_planes))

            # ****************************************************************
            # 2. Get the target image location from the mapfile for the parset.
            # Create target dir if it not exists
            image_path_head = os.path.dirname(output_image)
            create_directory(image_path_head)
            self.logger.debug("Created directory to place awimager output"
                              " files: {0}".format(image_path_head))

            # ****************************************************************
            # 3. Create the mask
            #mask_file_path = self._create_mask(npix, cell_size, output_image,
            #             concatenated_measurement_set, executable,
            #             working_directory, log4_cplus_name, sourcedb_path,
            #              mask_patch_size, image_path_head)
            # *****************************************************************
            # 4. Update the parset with calculated parameters, and output image
            patch_dictionary = {'uselogger': 'True',  # enables log4cpluscd log
                               'ms': str(concatenated_measurement_set),
                               'cellsize': str(cell_size),
                               'npix': str(npix),
                               'wmax': str(w_max),
                               'wprojplanes': str(w_proj_planes),
                               'image': str(output_image),
                               'maxsupport': str(npix)
                               # 'mask':str(mask_file_path),  #TODO REINTRODUCE
                               # MASK, excluded to speed up in this debug stage                               
                               }

            # Add some aditional keys from the self calibration method
            if perform_self_cal:
                self_cal_patch_dict = {
                               'weight': 'briggs', 
                               'padding': str(1.18),
                               'niter' : str(1000000), 
                               'operation' : 'mfclark',
                               'timewindow' : '300',
                               'fits' : '',
                               'threshold' : str(threshold),
                               'robust' : str(robust),
                               'UVmin' : str(UVmin), 
                               'UVmax' : str(UVmax),
                               'maxbaseline' : str(10000000),
                               'select' : str("sumsqr(UVW[:2])<1e12"), 
                               }
                patch_dictionary.update(self_cal_patch_dict)

            # save the parset at the target dir for the image
            calculated_parset_path = os.path.join(image_path_head,
                                                       "parset.par")

            try:
                temp_parset_filename = patch_parset(parset, patch_dictionary)
                # Copy tmp file to the final location
                shutil.copyfile(temp_parset_filename, calculated_parset_path)
                self.logger.debug("Wrote parset for awimager run: {0}".format(
                                                    calculated_parset_path))
            finally:
                # remove temp file
                os.remove(temp_parset_filename)

            # *****************************************************************
            # 5. Run the awimager with the parameterset

            # TODO: FIXME: manually Limit number of threads used.
            omp_num_threads = 8
            self.environment['OMP_NUM_THREADS'] = str(omp_num_threads)
            self.logger.debug("Using %s threads for swimager" % omp_num_threads)

            cmd = [executable, calculated_parset_path]
            self.logger.debug("Parset used for awimager run:")
            self.logger.debug(cmd)
            try:
                with CatchLog4CPlus(working_directory,
                        self.logger.name + "." +
                        os.path.basename(log4_cplus_name),
                        os.path.basename(executable)
                ) as logger:
                    catch_segfaults(cmd, working_directory, self.environment,
                                            logger, usageStats=self.resourceMonitor)

            # Thrown by catch_segfault
            except CalledProcessError, exception:
                self.logger.error(str(exception))
                return 1

            except Exception, exception:
                self.logger.error(str(exception))
                return 1

        # *********************************************************************
        # 6. Return output
        # Append static .restored: This might change but prob. not
        # The actual output image has this extention always, default of
        # awimager
        self.outputs["image"] = output_image + ".restored"
        return 0

    def _get_imaging_parameters(self, measurement_set, parset,
                autogenerate_parameters, specify_fov, fov):
        """
        (1) calculate and format some parameters that are determined runtime.
        Based  on values in the measurementset and input parameter (set):
        
        a. <string> The cellsize
        b. <int> The npixels in a each of the two dimension of the image
        c. <string> The largest baseline in the ms smaller then the maxbaseline
        d. <string> The number of projection planes
        
        The calculation of these parameters is done in three steps:
        
        1. Calculate intermediate results based on the ms. 
        2. The calculation of the actual target values using intermediate
           result       
        """
        # *********************************************************************
        # 1. Get partial solutions from the parameter set
        # Get the parset and a number of raw parameters from this parset
        parset_object = get_parset(parset)
        baseline_limit = parset_object.getInt('maxbaseline')

        # Get the longest baseline
        max_baseline = pt.taql(
                        'CALC sqrt(max([select sumsqr(UVW[:2]) from ' + \
            '{0} where sumsqr(UVW[:2]) <{1} giving as memory]))'.format(\
            measurement_set, baseline_limit *
            baseline_limit))[0]  # ask ger van diepen for details if ness.
        # Calculate the wave_length
        table_ms = pt.table(measurement_set)
        table_spectral_window = pt.table(
                                        table_ms.getkeyword("SPECTRAL_WINDOW"))
        freq = table_spectral_window.getcell("REF_FREQUENCY", 0)

        table_spectral_window.close()
        wave_length = pt.taql('CALC C()') / freq
        wave_length = wave_length[0]

        # Calculate the cell_size from the ms
        arc_sec_in_degree = 3600
        arc_sec_in_rad = (180.0 / math.pi) * arc_sec_in_degree
        cell_size = (1.0 / 3) * (wave_length / float(max_baseline))\
             * arc_sec_in_rad

        # Calculate the number of pixels in x and y dim
        #    fov and diameter depending on the antenna name
        fov_from_ms, station_diameter = self._get_fov_and_station_diameter(
                                                            measurement_set)

        # use fov for to calculate a semi 'user' specified npix and cellsize
        # The npix thus depends on the ms cellsize and fov
        # Do not use use supplied Fov if autogenerating
        if not autogenerate_parameters and specify_fov:
            if fov == 0.0:
                raise PipelineException("fov set to 0.0: invalid value.")

        # else use full resolution (calculate the fov)
        else:
            self.logger.info("Using fov calculated on measurement data: " +
                             str(fov_from_ms))
            fov = fov_from_ms

        # ********************************************************************
        # 2. Calculate the ms based output variables
        # 'optimal' npix based on measurement set calculations or user specified
        npix = (arc_sec_in_degree * fov) / cell_size

        # Get the closest power of two larger then the calculated pixel size
        npix = self._nearest_ceiled_power2(npix)

        # Get the max w with baseline < 10000
        w_max = pt.taql('CALC max([select UVW[2] from ' + \
            '{0} where sumsqr(UVW[:2]) <{1} giving as memory])'.format(
            measurement_set, baseline_limit * baseline_limit))[0]

        # Calculate number of projection planes
        w_proj_planes = min(257, math.floor((max_baseline * wave_length) /
                                             (station_diameter ** 2)))
        w_proj_planes = int(round(w_proj_planes))

        # MAximum number of proj planes set to 1024: George Heald, Ger van
        # Diepen if this exception occurs
        maxsupport = max(1024, npix)
        if w_proj_planes > maxsupport:
            raise Exception("The number of projections planes for the current"
                            + "measurement set is to large.")

        # *********************************************************************
        # 3. if the npix from the parset is different to the ms calculations,
        # calculate a sizeconverter value  (to be applied to the cellsize)
        if npix < 256:
            self.logger.warn("Using a image size smaller then 256x256:"
            " This leads to problematic imaging in some instances!!")

        # If we are not autocalculating based on ms or fov, use the npix
        # and cell_size specified in the parset
        # keep the wmax and w_proj_planes
        if (not autogenerate_parameters and not specify_fov):
            npix = parset_object.getString('npix')
            cell_size_formatted = parset_object.getString('cellsize')
        else:
            cell_size_formatted = str(
                        int(round(cell_size))) + 'arcsec'

        self.logger.info("Using the following awimager parameters:"
            " cell_size: {0}, npix: {1},".format(
                        cell_size_formatted, npix) +
             " w_max: {0}, w_proj_planes: {1}".format(w_max, w_proj_planes))

        return cell_size_formatted, str(npix), str(w_max), str(w_proj_planes)


    # Awimager parameters  for selfcal process (depends with major cycle)
    # nicolas: THis function needs a lot more documentation:
    # THis is the function that does the magic.
    # For each step everything must be cristal clear what is happening.
    # I will need to support this function 
    # this function is full with magic numbers
    # Instead of:
    # variable_a = 3.14 * 3600 * 5
    # use:
    # pi = math.pi
    # second_in_hour = 3600
    # factorx = 5    # Factor controlling x, value based on manual optimalization    
    def _get_selfcal_parameters(self, measurement_set, parset, major_cycle,
                                nr_cycles): 
      """
      0. modify the nof cycle to have a final step at the same resolution 
      as the previous last cycle
      1. Determine target coordinates especially declinaison, because 
      for low dec (<35 deg) UVmin = 0.1 to excluse very short baseline
      2. Determine the frequency and the wavelenght
      3. Determine the longuest baseline and the best resolution avaible
      4. Estimate all imaging parameters
      5. Calculate number of projection planes
      6. Pixelsize must be a string number : number +arcsec

      # Nicolas Vilchez, 2014
      # vilchez@astron.nl
      """		
      
      
      # ********************************************************************
      #0. modify the nof cycle to have a final step at the same resolution 
      #as the previous last cycle

      if major_cycle < nr_cycles-1:          
           nr_cycles = nr_cycles-1

      scaling_factor = float(major_cycle) / float(nr_cycles - 1)
    
      # ********************************************************************
      #1. Determine Target coordinates for UVmin
      tabtarget	= pt.table(measurement_set)
      tabfield	= pt.table(tabtarget.getkeyword('FIELD'))
      coords		= tabfield.getcell('REFERENCE_DIR',0)
      target		= coords[0] * 180.0 / math.pi  # Why

      UVmin=0
      if target[1] <= 35:  # WHy?
          UVmin = 0.1		    

      ra_target	= target[0] + 360.0  # Why
      dec_target	= target[1]    

      # ********************************************************************        
      # 2. Determine the frequency and the wavelenght
      tabfreq					= pt.table(measurement_set)
      table_spectral_window 	= pt.table(tabfreq.getkeyword("SPECTRAL_WINDOW"))
      frequency				= table_spectral_window.getcell('REF_FREQUENCY', 0)   

      wavelenght  = 3.0E8 / frequency  # Why

      # ********************************************************************        
      # 3. Determine the longuest baseline and the best resolution avaible	

      tabbaseline 	= pt.table(measurement_set, readonly=False, ack=True)
      posbaseline 	= tabbaseline.getcol('UVW')
      maxBaseline 	= max(posbaseline[:, 0] ** 2 + 
                          posbaseline[:, 1] ** 2) ** 0.5 

      bestBeamresol	= round((wavelenght / maxBaseline) * 
                            (180.0 / math.pi) * 3600.0, 0)

      # Beam resolution limitation to 10arcsec to avoid too large images
      if bestBeamresol < 10.0:
          bestBeamresol = 10.0	

      # ********************************************************************        
      # 4. Estimate all imaging parameters
 
      # estimate fov
      # fov = 5 degree, except for High HBA Observation => 1.5 degree
      if frequency > 1.9E8:
          fov	= 1.5				
      else:
          fov	= 5.0	    

      # we need 4 pixel/beam to have enough sampling
      pixPerBeam	= 4.0 

      # best resolution pixel size (i.e final pixel size for selfcal)
      bestPixelResol  = round(bestBeamresol / pixPerBeam, 2) 

      # factor to estimate the starting resolution (9 times in this case)
      badResolFactor	= 9
      
      pixsize	= round((badResolFactor * bestPixelResol) - 
          (badResolFactor * bestPixelResol - bestPixelResol) *
           scaling_factor , 3)
                   

      # number of pixel must be a multiple of 2 !!
      nbpixel	= int(fov * 3600.0 / pixsize)
      if nbpixel % 2 ==1:
          nbpixel = nbpixel + 1		
      
      robust	= 0 #round(1.0 - (3.0 * scaling_factor), 2)

      UVmax	= round((wavelenght) / 
                       (pixPerBeam * pixsize / 3600.0 * math.pi / 180.0 ) / 
                       (1E3 * wavelenght), 3)

      wmax	= round(UVmax * (wavelenght) * 1E3, 3)		

      # ********************************************************************        
      # 5. Calculate number of projection planes
      # Need to compute station diameter (the fov is fixed to 5 degree)
      # using wouter's function, to compute the w_proj_planes
      #    fov and diameter depending on the antenna name       
      fov_from_ms, station_diameter = self._get_fov_and_station_diameter(
                                                              measurement_set)        

      w_proj_planes = min(257, math.floor((maxBaseline * wavelenght) / 
                                          (station_diameter ** 2)))
      w_proj_planes = int(round(w_proj_planes))

      # MAximum number of proj planes set to 1024: George Heald, Ger van
      # Diepen if this exception occurs
      maxsupport = max(1024, nbpixel)
      if w_proj_planes > maxsupport:
          raise Exception("The number of projections planes for the current" +
                          "measurement set is to large.")

      # Warnings on pixel size
      if nbpixel < 256:
          self.logger.warn("Using a image size smaller then 256x256: This " + 
                           "leads to problematic imaging in some instances!!") 
 
 
      # ********************************************************************        
      # 6. Pixelsize must be a string number : number +arcsec
      #    conversion at this step
      pixsize = str(pixsize)+'arcsec'

      # ********************************************************************        
      # 7. Threshold determination from the previous cycle 
      if major_cycle == 0:
          threshold = '0.075Jy'	
      else:
        fits_image_path_list	= measurement_set.split('concat.ms')
        fits_image_path			= fits_image_path_list[0] +\
                'awimage_cycle_%s/image.fits'%(major_cycle-1)


        # open a FITS file 
        fitsImage	= pyfits.open(fits_image_path) 
        scidata 	= fitsImage[0].data 

        dataRange	= range(fitsImage[0].shape[2])
        sortedData	=  range(fitsImage[0].shape[2] ** 2)

        # FIXME We have the sneaking suspicion that this takes very long
        # due to bad coding style... (double for loop with compute in inner loop)
        for i in dataRange:
            for j in dataRange:
                sortedData[i * fitsImage[0].shape[2] + j]	=  scidata[0,0,i,j]

        sortedData 		= sorted(sortedData)

        # Percent of faintest data to use to determine 5sigma value : use 5%			
        dataPercent		= int(fitsImage[0].shape[2] * 0.05)

        fiveSigmaData	= sum(sortedData[0:dataPercent]) / dataPercent	
        threshold		= (abs(fiveSigmaData) / 5.0) * (2.335 / 2.0) * 15

      return pixsize, str(nbpixel), str(wmax), str(w_proj_planes), \
             str(UVmin), str(UVmax), str(robust), str(threshold)
     
      
          
    def _get_fov_and_station_diameter(self, measurement_set):
        """
        _field_of_view calculates the fov, which is dependend on the
        station type, location and mode:
        For details see:
        (1) http://www.astron.nl/radio-observatory/astronomers/lofar-imaging-capabilities-sensitivity/lofar-imaging-capabilities/lofar
        
        """
        # Open the ms
        table_ms = pt.table(measurement_set)

        # Get antenna name and observation mode
        antenna = pt.table(table_ms.getkeyword("ANTENNA"))
        antenna_name = antenna.getcell('NAME', 0)
        antenna.close()

        observation = pt.table(table_ms.getkeyword("OBSERVATION"))
        antenna_set = observation.getcell('LOFAR_ANTENNA_SET', 0)
        observation.close()

        # static parameters for the station diameters ref (1)
        hba_core_diameter = 30.8
        hba_remote_diameter = 41.1
        lba_inner = 32.3
        lba_outer = 81.3

        # use measurement set information to assertain antenna diameter
        station_diameter = None
        if antenna_name.count('HBA'):
            if antenna_name.count('CS'):
                station_diameter = hba_core_diameter
            elif antenna_name.count('RS'):
                station_diameter = hba_remote_diameter
        elif antenna_name.count('LBA'):
            if antenna_set.count('INNER'):
                station_diameter = lba_inner
            elif antenna_set.count('OUTER'):
                station_diameter = lba_outer

        # raise exception if the antenna is not of a supported type
        if station_diameter == None:
            self.logger.error(
                    'Unknown antenna type for antenna: {0} , {1}'.format(\
                              antenna_name, antenna_set))
            raise PipelineException(
                    "Unknown antenna type encountered in Measurement set")

        # Get the wavelength
        spectral_window_table = pt.table(table_ms.getkeyword(
                                                            "SPECTRAL_WINDOW"))
        freq = float(spectral_window_table.getcell("REF_FREQUENCY", 0))
        wave_length = pt.taql('CALC C()') / freq
        spectral_window_table.close()

        # Now calculate the FOV see ref (1)
        # alpha_one is a magic parameter: The value 1.3 is representative for a
        # WSRT dish, where it depends on the dish illumination
        alpha_one = 1.3

        # alpha_one is in radians so transform to degrees for output
        fwhm = alpha_one * (wave_length / station_diameter) * (180 / math.pi)
        fov = fwhm / 2.0
        table_ms.close()

        return fov, station_diameter

    def _create_mask(self, npix, cell_size, output_image,
                     concatenated_measurement_set, executable,
                     working_directory, log4_cplus_name, sourcedb_path,
                     mask_patch_size, image_path_directory):
        """
        (3) create a casa image containing an mask blocking out the
        sources in the provided sourcedb.
        
        It expects:
        
        a. the ms for which the mask will be created, it is used to de
           termine some image details: (eg. pointing)
        b. parameters for running within the catchsegfault framework
        c. and the size of the mask_pach.
           To create a mask, first a empty measurement set is created using
           awimager: ready to be filled with mask data 
           
        This function is a wrapper around some functionality written by:
        fdg@mpa-garching.mpg.de
        
        steps: 
        1. Create a parset with image paramters used by:
        2. awimager run. Creating an empty casa image.
        3. Fill the casa image with mask data
           
        """
        # ********************************************************************
        # 1. Create the parset used to make a mask
        mask_file_path = output_image + ".mask"

        mask_patch_dictionary = {"npix": str(npix),
                                 "cellsize": str(cell_size),
                                 "image": str(mask_file_path),
                                 "ms": str(concatenated_measurement_set),
                                 "operation": "empty",
                                 "stokes": "'I'"
                                 }
        mask_parset = Parset.fromDict(mask_patch_dictionary)
        mask_parset_path = os.path.join(image_path_directory, "mask.par")
        mask_parset.writeFile(mask_parset_path)
        self.logger.debug(
                "Write parset for awimager mask creation: {0}".format(
                                                      mask_parset_path))

        # *********************************************************************
        # 2. Create an empty mask using awimager
        cmd = [executable, mask_parset_path]
        self.logger.info(" ".join(cmd))
        try:
            with CatchLog4CPlus(working_directory,
                    self.logger.name + "." + os.path.basename(log4_cplus_name),
                    os.path.basename(executable)
            ) as logger:
                catch_segfaults(cmd, working_directory, self.environment,
                                        logger)
        # Thrown by catch_segfault
        except CalledProcessError, exception:
            self.logger.error(str(exception))
            return 1
        except Exception, exception:
            self.logger.error(str(exception))
            return 1

        # ********************************************************************
        # 3. create the actual mask
        self.logger.debug("Started mask creation using mask_patch_size:"
                          " {0}".format(mask_patch_size))

        self._msss_mask(mask_file_path, sourcedb_path, mask_patch_size)
        self.logger.debug("Fished mask creation")
        return mask_file_path

    def _msss_mask(self, mask_file_path, sourcedb_path, mask_patch_size = 1.0):
        """
        Fill casa image with a mask based on skymodel(sourcedb)
        Bugs: fdg@mpa-garching.mpg.de
        
        pipeline implementation klijn@astron.nl
        version 0.32
        
        Edited by JDS, 2012-03-16:
         - Properly convert maj/minor axes to half length
         - Handle empty fields in sky model by setting them to 0
         - Fix off-by-one error at mask boundary
        
        FIXED BUG
         - if a source is outside the mask, the script ignores it
         - if a source is on the border, the script draws only the inner part
         - can handle skymodels with different headers
        
        KNOWN BUG
         - not works with single line skymodels, workaround: add a fake
           source outside the field
         - mask patched display large amounts of aliasing. A possible 
           sollution would
           be normalizing to pixel centre. ( int(normalize_x * npix) /
           npix + (0.5 /npix)) 
           ideally the patch would increment in pixel radiuses
             
        Version 0.3  (Wouter Klijn, klijn@astron.nl)
         - Usage of sourcedb instead of txt document as 'source' of sources
           This allows input from different source sources
        Version 0.31  (Wouter Klijn, klijn@astron.nl)
         - Adaptable patch size (patch size needs specification)
         - Patch size and geometry is broken: needs some astronomer magic to
           fix it, problem with afine transformation prol.
        Version 0.32 (Wouter Klijn, klijn@astron.nl)
         - Renaming of variable names to python convention
        """
        # increment in maj/minor axes [arcsec]
        pad = 500.

        # open mask
        mask = pim.image(mask_file_path, overwrite = True)
        mask_data = mask.getdata()
        xlen, ylen = mask.shape()[2:]
        freq, stokes, null, null = mask.toworld([0, 0, 0, 0])

        # Open the sourcedb:
        table = pt.table(sourcedb_path + "::SOURCES")
        pdb = lofar.parmdb.parmdb(sourcedb_path)

        # Get the data of interest
        source_list = table.getcol("SOURCENAME")
        source_type_list = table.getcol("SOURCETYPE")
        # All date in the format valuetype:sourcename
        all_values_dict = pdb.getDefValues()

        # Loop the sources
        for source, source_type in zip(source_list, source_type_list):
            if source_type == 1:
                type_string = "Gaussian"
            else:
                type_string = "Point"
            self.logger.info("processing: {0} ({1})".format(source,
                                                             type_string))

            # Get de right_ascension and declination (already in radians)
            right_ascension = all_values_dict["Ra:" + source][0, 0]
            declination = all_values_dict["Dec:" + source][0, 0]
            if source_type == 1:
                # Get the raw values from the db
                maj_raw = all_values_dict["MajorAxis:" + source][0, 0]
                min_raw = all_values_dict["MinorAxis:" + source][0, 0]
                pa_raw = all_values_dict["Orientation:" + source][0, 0]
                # convert to radians (conversion is copy paste JDS)
                # major radius (+pad) in rad
                maj = (((maj_raw + pad)) / 3600.) * np.pi / 180.
                # minor radius (+pad) in rad
                minor = (((min_raw + pad)) / 3600.) * np.pi / 180.
                pix_asc = pa_raw * np.pi / 180.
                # wenss writes always 'GAUSSIAN' even for point sources
                # -> set to wenss beam+pad
                if maj == 0 or minor == 0:
                    maj = ((54. + pad) / 3600.) * np.pi / 180.
                    minor = ((54. + pad) / 3600.) * np.pi / 180.
            # set to wenss beam+pad
            elif source_type == 0:
                maj = (((54. + pad) / 2.) / 3600.) * np.pi / 180.
                minor = (((54. + pad) / 2.) / 3600.) * np.pi / 180.
                pix_asc = 0.
            else:
                self.logger.info(
                    "WARNING: unknown source source_type ({0}),"
                    "ignoring: ".format(source_type))
                continue

            # define a small square around the source to look for it
            null, null, border_y1, border_x1 = mask.topixel(
                    [freq, stokes, declination - maj,
                      right_ascension - maj / np.cos(declination - maj)])
            null, null, border_y2, border_x2 = mask.topixel(
                    [freq, stokes, declination + maj,
                     right_ascension + maj / np.cos(declination + maj)])
            xmin = np.int(np.floor(np.min([border_x1, border_x2])))
            xmax = np.int(np.ceil(np.max([border_x1, border_x2])))
            ymin = np.int(np.floor(np.min([border_y1, border_y2])))
            ymax = np.int(np.ceil(np.max([border_y1, border_y2])))

            if xmin > xlen or ymin > ylen or xmax < 0 or ymax < 0:
                self.logger.info(
                    "WARNING: source {0} falls outside the mask,"
                    " ignoring: ".format(source))
                continue

            if xmax > xlen or ymax > ylen or xmin < 0 or ymin < 0:
                self.logger.info(
                    "WARNING: source {0} falls across map edge".format(source))

            for pixel_x in xrange(xmin, xmax):
                for pixel_y in xrange(ymin, ymax):
                    # skip pixels outside the mask field
                    if pixel_x >= xlen or pixel_y >= ylen or\
                       pixel_x < 0 or pixel_y < 0:
                        continue
                    # get pixel right_ascension and declination in rad
                    null, null, pix_dec, pix_ra = mask.toworld(
                                                    [0, 0, pixel_y, pixel_x])
                    # Translate and rotate coords.
                    translated_pixel_x = (pix_ra - right_ascension) * np.sin(
                        pix_asc) + (pix_dec - declination) * np.cos(pix_asc)
                    # to align with ellipse
                    translate_pixel_y = -(pix_ra - right_ascension) * np.cos(
                        pix_asc) + (pix_dec - declination) * np.sin(pix_asc)
                    if (((translated_pixel_x ** 2) / (maj ** 2)) +
                        ((translate_pixel_y ** 2) / (minor ** 2))) < \
                                                         mask_patch_size:
                        mask_data[0, 0, pixel_y, pixel_x] = 1
        null = null
        mask.putdata(mask_data)
        table.close()
    
    # some helper functions
    def _nearest_ceiled_power2(self, value):
        """
        Return int value of  the nearest Ceiled power of 2 for the
        suplied argument
        
        """
        return int(pow(2, math.ceil(math.log(value, 2))))


    def _save_selfcal_info(self, concatenated_measurement_set,
                           major_cycle, npix, UVmin, UVmax):
        """ 
        The selfcal team requested meta information to be added to 
        measurement set that allows the reproduction of intermediate
        steps.
        """ 
        self.logger.info("Save-ing selfcal parameters to file:")
        meta_file = os.path.join(
              concatenated_measurement_set + "_selfcal_information",
                                  "uvcut_and_npix.txt")
        self.logger.info(meta_file)

        # check if we have the output file? Add the header 
        if not os.path.exists(meta_file):
            meta_file_pt = open(meta_file, 'w')
            meta_file_pt.write("#cycle_nr npix uvmin(klambda) uvmax(klambda)\n")
            meta_file_pt.close()
                 
        meta_file_pt = open(meta_file, 'a')

        # Create the actual string with info
        meta_info_str = " ".join([str(major_cycle),
                                 str(npix),
                                 str(UVmin),
                                str(UVmax)])
        meta_file_pt.write(meta_info_str + "\n")
        meta_file_pt.close()


if __name__ == "__main__":
    _JOBID, _JOBHOST, _JOBPORT = sys.argv[1:4]
    sys.exit(selfcal_awimager(
                    _JOBID, _JOBHOST, _JOBPORT).run_with_stored_arguments())
