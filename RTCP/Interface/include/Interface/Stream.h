//#  Stream.h: functions to construct streams between ION/CN/Storage
//#
//#  Copyright (C) 2006
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id: Stream.h 16488 2010-10-07 10:06:14Z mol $

#ifndef LOFAR_INTERFACE_STREAM_H
#define LOFAR_INTERFACE_STREAM_H

#include <Stream/Stream.h>
#include <string>

#ifndef HAVE_BGP_CN
#include <Interface/Parset.h>
#endif

namespace LOFAR {
namespace RTCP {

// Create a stream from a descriptor
Stream *createStream(const std::string &descriptor, bool asReader);

// Return a string descriptor, for all supported streamTypes except FCNP
std::string getStreamDescriptorBetweenIONandCN(const char *streamType, unsigned pset, unsigned core, unsigned numpsets, unsigned numcores, unsigned channel);

#ifndef HAVE_BGP_CN
std::string getStreamDescriptorBetweenIONandStorage(const Parset &parset, unsigned subband, unsigned output, bool perSubband = true);
#endif

} // namespace RTCP
} // namespace LOFAR

#endif
