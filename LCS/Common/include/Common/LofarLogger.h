//#  LofarLogger.h: Macro interface to the lofar logging package
//#
//#  Copyright (C) 2004
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
//#  $Id$

#ifndef LOFAR_COMMON_LOFARLOGGER_H
#define LOFAR_COMMON_LOFARLOGGER_H

// \file
// Macro interface to the lofar logging package.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
#include <Common/StringUtil.h>

//# Dependent of the availability of the log4Cplus package one set of macro's
//# or the other set of macro's is included.
#ifdef HAVE_LOG4CPLUS
# include <Common/LofarLog4Cplus.h>
#else
# include <Common/LofarLogCout.h>
#endif //HAVE_LOG4CPLUS

namespace LOFAR 
{

// \ingroup Common
// \addtogroup LofarLogger
  // @{

  // Define the LOFAR::Exception class.
EXCEPTION_CLASS(AssertError,Exception);


//# --------------------- Common definitions ---------------
//
// \name Common definitions 
// @{

// The package name of the current package. This variable should be defined in
// the file lofar_config.h which is generated by the lofarconf script. Here,
// we will make sure that LOFARLOGGER_PACKAGE is always defined.
#ifndef LOFARLOGGER_PACKAGE
# define LOFARLOGGER_PACKAGE "Unknown_package"
#endif

// The full package name is defined as the concatenation of the package name
// and an optional sub-package name that may be defined by the user.
#ifdef LOFARLOGGER_SUBPACKAGE
# define LOFARLOGGER_FULLPACKAGE LOFARLOGGER_PACKAGE "." LOFARLOGGER_SUBPACKAGE
#else
# define LOFARLOGGER_FULLPACKAGE LOFARLOGGER_PACKAGE
#endif

//#
//# AUTO_FUNCTION_NAME
//#
// This macro will be resolved by the (pre)compiler to hold the name of the
// function the macro was used in.
#if defined(HAVE_PRETTY_FUNCTION)
#	define AUTO_FUNCTION_NAME		__PRETTY_FUNCTION__
#elif defined(HAVE_FUNCTION)
#	define AUTO_FUNCTION_NAME		__FUNCTION__
#else
#	define AUTO_FUNCTION_NAME		"??"
#endif

// @}

//#------------------------ Assert and FailWhen -------------------------------
//#
//# NB: THROW is defined in package-dependant file.
//
// \name Assert and FailWhen
// \note The \c DBG version of \c ASSERT and \c FAILWHEN will only be in your
// compiled code when the (pre)compiler flag \c ENABLE_DBGASSERT is defined.
// @{

// If the condition of assert is NOT met a logrequest is sent to the logger
// <tt>\<module\>.EXCEPTION</tt> and an AssertError exception is thrown.
#define ASSERT(cond) \
	if (!(cond))  THROW(::LOFAR::AssertError, "Assertion: " #cond)

// If the condition of assert is NOT met a logrequest is sent to the logger
// <tt>\<module\>.EXCEPTION</tt> and an AssertError exception is thrown.
#define ASSERTSTR(cond,stream) \
	if (!(cond))  THROW(::LOFAR::AssertError, "Assertion: " #cond "; " << stream)

// If the condition of failwhen is met a logrequest is sent to the logger
// <tt>\<module\>.EXCEPTION</tt> and an AssertError exception is thrown.
#define FAILWHEN(cond) \
	if (cond)  THROW(::LOFAR::AssertError, "Failtest: " #cond)

#ifdef ENABLE_DBGASSERT
#define DBGASSERT(cond)				ASSERT(cond)
#define DBGASSERTSTR(cond,stream)	ASSERTSTR(cond,stream)
#define DBGFAILWHEN(cond)			FAILWHEN(cond)
#else
#define DBGASSERT(cond)
#define DBGASSERTSTR(cond,stream)
#define DBGFAILWHEN(cond)
#endif

// @}
    
} // namespace LOFAR

#endif // file read before
