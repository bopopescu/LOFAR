//# Cells.h: The cells in a given domain.
//#
//# Copyright (C) 2003
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef MEQ_CELLS_H
#define MEQ_CELLS_H


//# Includes
#include <MEQ/Domain.h>
#include <MEQ/AID-Meq.h>
#include <DMI/DataRecord.h>
#include <Common/Lorrays.h>
#include <ostream>

#pragma aidgroup Meq
#pragma types #Meq::Cells

namespace Meq {

//##ModelId=3F86886E017A
class Cells : public DataRecord
{
public:
  typedef enum {
    NONE        = 0,
    SET_NCELLS  = 1,
    INTEGRATE   = 2,
    UPSAMPLE    = 3
  } CellsOperations;
    
    //##ModelId=400E53030256
  typedef CountedRef<Cells> Ref;
    
    //##ModelId=3F86886E02C1
  Cells ();
  // Construct from DataRecord. 
    //##ModelId=3F86886E02C8
  Cells (const DataRecord &other,int flags=DMI::PRESERVE_RW,int depth=0);
  
  // creates an empty cells with a domain
  Cells (const Domain& domain);
  
    //##ModelId=3F95060B01D3
    //##Documentation
    //## constructs uniformly-spaced cells with the given number of x and y
    //## points. Domain  must define axes 0 and 1 (and no others)
  Cells (const Domain& domain,int nx,int ny);
  
  // creates a resampling combining cells a and b. If resample>0, upsamples
  // the lower resolution. If resample<0, integrates the higher resolution.
  Cells (const Cells &a,const Cells &b,int resample);
  
  // creates a resampling of a Cells. op[] is an array of operations on each
  // axis (see the CellsOperations enums above), arg[] is an array of arguments
  Cells (const Cells &other,const int op[Axis::MaxAxis],const int arg[Axis::MaxAxis]);
  
    //##ModelId=3F86886E02D1
  ~Cells();
  
      //##ModelId=400E530403C1
  virtual TypeId objectType () const
  { return TpMeqCells; }
  
  // implement standard clone method via copy constructor
    //##ModelId=400E530403C5
  virtual CountedRefTarget* clone (int flags, int depth) const
  { return new Cells(*this,flags|(depth>0?DMI::DEEP:0)); }
  
//   // implement standard clone method via copy constructor
//   virtual CountedRefTarget* clone (int flags, int depth) const
//   { return new Cells(*this,flags,depth); }
  
  // validate record contents and setup shortcuts to them. This is called 
  // automatically whenever a Cells object is made from a DataRecord
  // (or when the underlying DataRecord is privatized, etc.)
    //##ModelId=400E530403DB
  virtual void validateContent ();
  virtual void revalidateContent ();

  // returns true if some cells are defined over the given axis
  bool isDefined (int iaxis) const
  { 
    DbgAssert(iaxis>=0 && iaxis<Axis::MaxAxis);
    return iaxis < int(defined_.size()) ? defined_[iaxis] : false; 
  }
  
  // returns number of cells along axis
  int ncells (int iaxis) const
  { return isDefined(iaxis) ? shape_[iaxis] : 0; }
  
  // Get domain.
    //##ModelId=3F86886E02D2
  const Domain& domain() const
    { FailWhen(!domain_.valid(),"no domain"); return *domain_; }

  // returns vector of cell centers along specified axis 
  const LoVec_double & center (int iaxis) const
    { DbgAssert(iaxis>=0 && iaxis<Axis::MaxAxis);
      return grid_[iaxis]; }
      
  // returns vector of cell sizes along specified axis 
  const LoVec_double & cellSize (int iaxis) const
    { DbgAssert(iaxis>=0 && iaxis<Axis::MaxAxis);
      return cell_size_[iaxis]; }

  const LoShape & shape () const
    { return shape_; }    
  
  int rank () const
    { return shape_.size(); }
        
  // returns vector of cell lower boundaries (this is computed)
  LoVec_double cellStart (int iaxis) const
  {
    LoVec_double res(ncells(iaxis)); 
    return res = center(iaxis) - cellSize(iaxis)/2;
  }
  
  // returns vector of cell upper boundaries (this is computed)
  LoVec_double cellEnd (int iaxis) const
  {
    LoVec_double res(ncells(iaxis)); 
    return res = center(iaxis) + cellSize(iaxis)/2;
  }
  
  // computes returns the cell boundaries together
  void getCellStartEnd (LoVec_double &start,LoVec_double &end,int iaxis) const;
  
  // returns the number of uniformly-gridded segments along axis
  int numSegments (int iaxis) const
  {
    DbgAssert(iaxis>=0 && iaxis<Axis::MaxAxis);
    return seg_start_[iaxis].size();
  }
  
  // returns the starting indices of each segment
  const LoVec_int & segmentStart (int iaxis) const
  {
    DbgAssert(iaxis>=0 && iaxis<Axis::MaxAxis);
    return seg_start_[iaxis];
  }
      
  // returns the ending indices of each segment
  const LoVec_int & segmentEnd   (int iaxis) const
  {
    DbgAssert(iaxis>=0 && iaxis<Axis::MaxAxis);
    return seg_end_[iaxis];
  }
  
  // sets the cells for an axis. This will implictly call
  // recomputeSegments()
  // explicitly specified centers and sizes
  void setCells (int iaxis,const LoVec_double &cen,const LoVec_double &size);
  // explicitly specified centers, uniform size
  void setCells (int iaxis,const LoVec_double &cen,double size);
  // regular grid of num points covering the interval [x0,x1]. If size>0 is
  // specified, use that, else use default (x1-x0)/num
  void setCells (int iaxis,double x0,double x1,int num,double size=-1);
  // "enumeration" grid (centers at 0,1,2,...; sizes of 0)
  void setEnumCells (int iaxis,int num);

  // refreshes segment info for an axis
  void recomputeSegments (int iaxis);
  
  // refreshes envelope domain. Should be always be called after a series
  // of setCells(), otherwise the Cells object is left in an inconsistent state.
  void recomputeDomain ();

  // method used to compare cells & resolutions
  // returns:  0 if cells are the same
  //          >0 if domains match but resolutions are different
  //          <0 if domains do not match
  int compare (const Cells &that) const;
  
    //##ModelId=400E530403DE
  bool operator== (const Cells& that) const;

    //##ModelId=400E53050002
  bool operator!= (const Cells& that) const
    { return !(*this == that); }

  // print to stream
    //##ModelId=400E5305000E
  void show (std::ostream&) const;

  // override privatize() to detach/reattach shortcuts
  virtual void privatize (int flags = 0, int depth = 0);
  
private:
  DataRecord::merge;
  DataRecord::add;
  DataRecord::removeField;
  DataRecord::replace;
  DataRecord::field;
  DataRecord::fieldWr;
  DataRecord::get;
  DataRecord::insert;
  DataRecord::remove;
  DataRecord::initFieldIter;
  DataRecord::getFieldIter;
    
  // helper function: sets domain of cells
  void setDomain (const Domain &domain);

  // helper function: inits basic record structure
  void init (const Domain& domain);
  
  // helper function: inits regularly-spaced axis
  // which must already be present in the domain
  void setRegularAxis (int iaxis,int np);
    
  // helper function: defines shape of axis (shape_ vector)
  void setAxisShape (int axis,int num);
  // helper function: sets up data fields of given shape for an axis
  void setNumCells (int iaxis,int num);
  // helper function: sets up a segment subrecord
  void setNumSegments (int iaxis,int nseg);
    
  // helper function to assign new vector to datarecord & to vec
  template<class T>
  void setRecVector (blitz::Array<T,1> &vec,const Hook &hook,int n);
  // helper function to init/get a subrecord
  DataRecord & getSubrecord (const Hook &hook);
  
    //##ModelId=3F86BFF80150
  CountedRef<Domain>  domain_;
    //##ModelId=3F86886E02AC
  LoShape       shape_;    
  std::vector<bool> defined_;
  LoVec_double  grid_     [Axis::MaxAxis];
  LoVec_double  cell_size_[Axis::MaxAxis];
  LoVec_int     seg_start_[Axis::MaxAxis];
  LoVec_int     seg_end_  [Axis::MaxAxis];
};

} //namespace Meq

inline std::ostream& operator << (std::ostream& os, const Meq::Cells& cells)
{
  cells.show(os);
  return os;
}


#endif
