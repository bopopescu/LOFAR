# $Id$
#
# Copyright (C) 2008-2009
# ASTRON (Netherlands Foundation for Research in Astronomy)
# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

# Try to find Log4Cplus.
#
# Variables used by this module:
#  LOG4CPLUS_ROOT_DIR     - Log4Cplus root directory
#
# Variables defined by this module:
#  LOG4CPLUS_FOUND        - system has Log4Cplus
#  LOG4CPLUS_INCLUDE_DIR  - the Log4Cplus include directory (cached)
#  LOG4CPLUS_INCLUDE_DIRS - the Log4Cplus include directories
#                           (identical to LOG4CPLUS_INCLUDE_DIR)
#  LOG4CPLUS_LIBRARY      - the Log4Cplus library (cached)
#  LOG4CPLUS_LIBRARIES    - the Log4Cplus libraries
#                           (identical to LOG4CPLUS_LIBRARY)

if(NOT LOG4CPLUS_FOUND)

  find_path(LOG4CPLUS_INCLUDE_DIR log4cplus/logger.h
    PATHS ${LOG4CPLUS_ROOT_DIR} PATH_SUFFIXES include)
  find_library(LOG4CPLUS_LIBRARY log4cplus
    PATHS ${LOG4CPLUS_ROOT_DIR} PATH_SUFFIXES lib)
  mark_as_advanced(LOG4CPLUS_INCLUDE_DIR LOG4CPLUS_LIBRARY)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(Log4Cplus DEFAULT_MSG
    LOG4CPLUS_LIBRARY LOG4CPLUS_INCLUDE_DIR)

  set(LOG4CPLUS_INCLUDE_DIRS ${LOG4CPLUS_INCLUDE_DIR})
  set(LOG4CPLUS_LIBRARIES ${LOG4CPLUS_LIBRARY})

endif(NOT LOG4CPLUS_FOUND)
