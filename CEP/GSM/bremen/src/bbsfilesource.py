#!/usr/bin/python
"""
BBS-format file source object for GSM.
Author: Alexey Mints (2012).
"""
import os.path
from src.errors import SourceException
from src.gsmlogger import get_gsm_logger
from copy import copy


class GSMBBSFileSource(object):
    """
    Reads source list from BBS file.
    """
    #Number of lines to be read in each iteration;
    BLOCK_SIZE = 100

    FIELD_NAMES = {
      'ra': 'ra',
      'e_ra': 'ra_err',
      'dec': 'decl',
      'e_dec': 'decl_err',
      'total_flux': 'int_flux',
      'e_total_flux': 'int_flux_err',
      'peak_flux': 'peak_flux',
      'e_peak_flux': 'peak_flux_err',
    }

    HEADER_INDEX = {
      'ra': 0,
      'dec': 1,
      'e_ra': 2,
      'e_dec': 3,
      'peak_flux': 4,
      'e_peak_flux': 5,
      'total_flux': 6,
      'e_total_flux': 7,
      'dc_min': 8,
      'e_dc_min': 9,
      'dc_maj': 10,
      'e_dc_maj': 11,
      'dc_pa': 10,
      'e_dc_pa': 11,
    }

    DEFAULTS = map(str, [0, 0, 0.1, 0.1,  # ra/decl
                         0, 0.001, 0, 0.001,  # Flux
                         0.0, 0.0, 0.0, 0.0, 0.0, 0.0,  # Gaussian
                         3.0,
                         ])

    def __init__(self, file_id, filename, fileformat="default"):
        self.filename = filename
        self.file_id = file_id
        self.fileformat = fileformat
        self.defaults = copy(self.DEFAULTS)
        self.sources = 0
        self.log = get_gsm_logger('bbsfiles', 'import.log')
        self.log.info('BBS file source created for name: %s' % filename)
        self.order = []

    def process_header(self, header):
        """
        Get reordering array for the file
        TODO: write a better comment.
        """
        self.order = []
        for ind, item in enumerate(header):
            if type(item) == str and item in self.HEADER_INDEX:
                self.order.append((ind, self.HEADER_INDEX[item]))
            elif type(item) == tuple and item[0] in self.HEADER_INDEX:
                self.defaults[self.HEADER_INDEX[item[0]]] = item[1]

    def process_line(self, line):
        """
        Turn a line into an array using header information.
        """
        answer = copy(self.defaults)
        len_line = len(line)
        for from_index, to_index in self.order:
            if from_index < len_line:
                answer[to_index] = line[from_index]
        return answer

    def read_and_store_data(self, conn):
        """
        Read all from the BBS file.
        """
        line = None
        if not os.path.isfile(self.filename):
            raise SourceException('no file %s' % self.filename)
        datafile = open(self.filename, 'r')
        if self.fileformat == 'test':
            header = datafile.readline().split('=',
                                        1)[1].strip(' ').lower().split(',')
            for ind, head_parts in enumerate(header):
                head_part = head_parts.split('=')
                if len(head_part) != 1:  # Default value is given
                    header[ind] = (head_part[0],
                                   head_part[1].strip("'").strip())
        elif self.fileformat == 'default':
            line = datafile.readline()
            while not (line.startswith('# Gaus') or line.startswith("# RA")):
                line = datafile.readline()
            header = line[2:].strip().lower().split(' ')
        self.process_header(header)

        sql_data = []
        commit = conn.autocommit
        conn.set_autocommit(False)
        sql_insert = 'insert into detections (image_id, lra, ldecl, lra_err, '\
                     'ldecl_err, lf_peak, lf_peak_err, ' \
                     'lf_int, lf_int_err, ' \
                     'g_minor, g_minor_err, g_major, g_major_err,' \
                     'g_pa, g_pa_err, ldet_sigma) values'
        while True:
            data_lines = datafile.readlines(self.BLOCK_SIZE)
            if not data_lines:
                break
            for data_line in data_lines:
                if data_line.strip() == '' or data_line.startswith('#'):
                    #skip comments and empty lines
                    continue
                self.sources = self.sources + 1
                dhash = self.process_line(data_line.split())
                sql_data.append("('%s', %s)" %
                                (self.file_id, ','.join(dhash)))
            sql = "%s %s;" % (sql_insert, ',\n'.join(sql_data))
            conn.execute(sql)
            self.log.info('%s sources loaded from %s' % (self.sources,
                                                         self.filename))
            conn.commit()
            sql_data = []
        #Restore autocommit.
        conn.set_autocommit(commit)
        return True
