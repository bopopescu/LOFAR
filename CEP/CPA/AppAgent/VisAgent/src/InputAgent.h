//  VisInputAgent.h: agent for input of VisTiles
//
//  Copyright (C) 2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$

#ifndef _VISAGENT_INPUTAGENT_H
#define _VISAGENT_INPUTAGENT_H 1
    
#include <AppAgent/AppAgent.h>
#include <AppAgent/AppEventAgentBase.h>
#include <VisAgent/VisAgentVocabulary.h>
#include <VisCube/VisTile.h>
class AppEventSink;

namespace VisAgent 
{

//##ModelId=3DF9FECD0169
//##Documentation
//## VisInputAgent is an specialized agent representing an input stream of
//## visibility data. Applications that work with visibilities talk to
//## VisInputAgents.
//## 
//## VisInputAgent  is an abstract interface. Specific visibility data sources
//## (e.g.: input from an AIPS++ MS, input from a BOIO file, input from an
//## OCTOPUSSY WorkProcess) are implemented as derived classes.
//## 
//## A visibility stream is represented by a header (a DataRecord), followed by
//## a number of VisTiles. 
class InputAgent : public AppEventAgentBase
{
  public:
    //##ModelId=3E41433F01DD
    explicit InputAgent (const HIID &initf = AidInput)
      : AppEventAgentBase(initf) {}
    //##ModelId=3E41433F037E
    InputAgent (AppEventSink &sink, const HIID &initf = AidInput)
      : AppEventAgentBase(sink,initf) {}
    //##ModelId=3E50FAB702CB
    InputAgent(AppEventSink *sink, int dmiflags, const HIID &initf = AidInput)
      : AppEventAgentBase(sink,dmiflags,initf) {}

    
    //##ModelId=3E42350F01EB
    //##Documentation
    //## Agent initialization method. Called by the application to initialize
    //## or reinitialize an agent. Agent parameters are supplied via a
    //## DataRecord.
    virtual bool init (const DataRecord &data);
  
    //##ModelId=3DF9FECF0310
    //##Documentation
    //## Gets visibilities header from input stream. If wait=True, blocks until
    //## one has arrived.
    //## Returns: SUCCESS   on success
    //##          WAIT      header has not yet arrived (only for wait=False)
    //##          CLOSED    stream closed
    //##          OUTOFSEQ  next object in stream is not a header (i.e. a tile)
      virtual int getHeader   (DataRecord::Ref &hdr,int wait = AppEvent::WAIT);
  
    //##ModelId=3DF9FECF03A9
    //##Documentation
    //## Gets next available tile from input stream. If wait=True, blocks until
    //## one has arrived.
    //## Returns: SUCCESS   on success
    //##          WAIT      a tile has not yet arrived (only for wait=False)
    //##          CLOSED    stream closed
    //##          OUTOFSEQ  next object in stream is not a tile (i.e. a header)
      virtual int getNextTile (VisTile::Ref &tile,int wait = AppEvent::WAIT);
      
    //##ModelId=3DF9FED0005A
    //##Documentation
    //## Checks if a header is waiting in the stream. Return value: same as
    //## would have been returned by getHeader(wait=False).
      virtual int hasHeader   () const;
      
    //##ModelId=3DF9FED00076
    //##Documentation
    //## Checks if a tile is waiting in the stream. Return value: same as would
    //## have been returned by getNextTile(wait=False).
      virtual int hasTile     () const;
      
    //##ModelId=3DF9FED00090
    //##Documentation
    //## Tells the agent to suspend the input stream. Agents are not obliged to
    //## implement this. An application can use suspend() to "advise" the input
    //## agent that it is not keeping up with the input data rate; the agent
    //## can use this to determine it queuing or load balancing strategy.
      virtual void suspend     ();
      
    //##ModelId=3DF9FED000AA
    //##Documentation
    //## Resumes a stream after a suspend(). 
      virtual void resume      ();
      
    //##ModelId=3E4273B30013
      bool suspended() const;
      
    //##ModelId=3DF9FED000C6
    //##Documentation
    //## Returns the number of tiles queued up in a stream. Agents are not
    //## obliged to implement this. Control agents are meant to use this method
    //## in combination with suspend()/resume() to control the data flow. 
      virtual int  numPendingTiles ()
      { return hasTile() == AppEvent::SUCCESS ? 1 : 0; }
      
    //##ModelId=3E41141D029F
      virtual string sdebug(int detail = 1, const string &prefix = "", const char *name = 0) const;
      
    //##ModelId=3E9BD6400048
      DefineRefTypes(InputAgent,Ref);

  private:
    //##ModelId=3E4165150313
    InputAgent();

    //##ModelId=3E41651601E4
    InputAgent(const InputAgent& right);

    //##ModelId=3E416516038F
    InputAgent& operator=(const InputAgent& right);
    
    //##ModelId=3E42350E030A
    bool suspended_;

};

//##ModelId=3E4273B30013
inline bool InputAgent::suspended() const
{
    return suspended_;
}

} // namespace VisAgent
 
#endif

