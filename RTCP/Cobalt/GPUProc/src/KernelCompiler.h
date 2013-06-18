//# KernelCompiler.h
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_GPUPROC_KERNEL_COMPILER_H
#define LOFAR_GPUPROC_KERNEL_COMPILER_H

#include <iosfwd>
#include <map>
#include <set>
#include <string>

namespace LOFAR
{
  namespace Cobalt
  {
    // Class representing the preprocessor definitions that will be passed to
    // the GPU Kernel compiler. The main purpose of this class is to provide a
    // clean way to print the preprocessor definitions into an output stream.
    class CompileDefinitions
    {
    public:
      // Return a reference to the value of the element \a key.
      std::string& operator[](const std::string& key);

      // Return true if the map of compile definitions is empty.
      bool empty() const;

    private:
      // Print compile definitions in a usable form to output stream \a os.
      friend std::ostream&
      operator<<(std::ostream& os, const CompileDefinitions& defs);

      // Store compile definitions as key/value pairs.
      std::map<std::string, std::string> defs;
    };

    // Class representing the compiler flags that will be passed to the GPU
    // Kernel compiler. The main purpose of this class is to provide a clean way
    // to print the compiler flags into an output stream.
    class CompileFlags
    {
    public:
      // Add a compile flag. Replaces an already existing flag.
      void add(const std::string& flag);

      // Return true if set of flags is empty.
      bool empty() const;

    private:
      // Print compile flags in a usable form to output stream \a os.
      friend std::ostream&
      operator<<(std::ostream& os, const CompileFlags& flags);

      // Store compile flags in a set of string.
      std::set<std::string> flags;
    };

    // Return a set of default compile flags. The implementation of this method
    // will be CUDA/OpenCL-specific.
    const CompileFlags& defaultCompileFlags();

  }
}

#endif
