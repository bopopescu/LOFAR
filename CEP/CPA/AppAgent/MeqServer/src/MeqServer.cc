#include "MeqServer.h"
#include "AID-MeqServer.h"
#include <DMI/AID-DMI.h>
#include <MEQ/AID-Meq.h>
#include <MeqGen/AID-MeqGen.h>
#include <MEQ/Request.h>
#include <MEQ/Result.h>
    
using Debug::ssprintf;
using namespace AppControlAgentVocabulary;
using namespace VisRepeaterVocabulary;
using namespace VisVocabulary;
using namespace VisAgent;
    
namespace Meq 
{
  
static int dum =  aidRegistry_MeqServer() + 
                  aidRegistry_Meq() + 
                  aidRegistry_MeqGen();

const HIID DataProcessingError = AidData|AidProcessing|AidError;
  
InitDebugContext(MeqServer,"MeqServer");
  
//##ModelId=3F5F195E0140
MeqServer::MeqServer()
    : data_mux(forest)
{
  command_map["Create.Node"] = &MeqServer::createNode;
  command_map["Delete.Node"] = &MeqServer::deleteNode;
  command_map["Resolve.Children"] = &MeqServer::resolveChildren;
  command_map["Get.Node.List"] = &MeqServer::getNodeList;
  
  command_map["Node.Get.State"] = &MeqServer::getNodeState;
  command_map["Node.Set.State"] = &MeqServer::setNodeState;
  command_map["Node.Execute"] = &MeqServer::getNodeResult;
}

//##ModelId=3F6196800325
Node & MeqServer::resolveNode (const DataRecord &rec)
{
  int nodeindex = rec[AidNodeIndex].as<int>(-1);
  string name = rec[AidName].as<string>("");
  if( nodeindex>0 )
  {
    Node &node = forest.get(nodeindex);
    FailWhen( name.length() && node.name() != name,"node specified by index is "+ 
        node.name()+", which does not match specified name "+name); 
    return node;
  }
  FailWhen( !name.length(),"either nodeindex or name must be specified");
  cdebug(3)<<"looking up node name "<<name<<endl;
  return forest.findNode(name);
}


void MeqServer::createNode (DataRecord::Ref &out,DataRecord::Ref::Xfer &initrec)
{
  cdebug(2)<<"creating node ";
  cdebug(3)<<initrec->sdebug(3);
  cdebug(2)<<endl;
  int nodeindex;
  const Node::Ref &ref = forest.create(nodeindex,initrec);
  // add to spigot mux if necessary
  data_mux.addNode(ref());
  // form a response message
  const string & name = ref->name();
  string classname = ref->className();
  
  out()[AidNodeIndex] = nodeindex;
  out()[AidName] = name;
  out()[AidClass] = classname;
  out()[AidMessage] = ssprintf("created node %d:%s of class %s",
                        nodeindex,name.c_str(),classname.c_str());
}

void MeqServer::deleteNode (DataRecord::Ref &out,DataRecord::Ref::Xfer &in)
{
  int nodeindex = (*in)[AidNodeIndex].as<int>(-1);
  if( nodeindex<0 )
  {
    string name = (*in)[AidName].as<string>("");
    cdebug(3)<<"looking up node name "<<name<<endl;
    FailWhen( !name.length(),"either nodeindex or name must be specified");
    nodeindex = forest.findIndex(name);
    FailWhen( nodeindex<0,"node '"+name+"' not found");
  }
  const Node::Ref &noderef = forest.getRef(nodeindex);
  string name = noderef->name();
  cdebug(2)<<"deleting node "<<name<<"("<<nodeindex<<")\n";
  // remove from the spigot mux if necessary
  data_mux.removeNode(noderef());
  // remove from forest
  forest.remove(nodeindex);
  out()[AidMessage] = ssprintf("node %d (%s) deleted",nodeindex,name.c_str());
}

void MeqServer::getNodeState (DataRecord::Ref &out,DataRecord::Ref::Xfer &in)
{
  Node & node = resolveNode(*in);
  cdebug(3)<<"getState for node "<<node.name()<<" ";
  cdebug(4)<<in->sdebug(3);
  cdebug(3)<<endl;
  out.attach(node.state(),DMI::READONLY|DMI::ANON);
}

void MeqServer::setNodeState (DataRecord::Ref &out,DataRecord::Ref::Xfer &in)
{
  DataRecord::Ref rec = in;
  Node & node = resolveNode(*rec);
  cdebug(3)<<"setState for node "<<node.name()<<endl;
  rec.privatize(DMI::WRITE|DMI::DEEP);
  node.setState(rec()[AidState].as_wr<DataRecord>());
  out.attach(node.state(),DMI::READONLY|DMI::ANON);
}

//##ModelId=3F98D91A03B9
void MeqServer::resolveChildren (DataRecord::Ref &out,DataRecord::Ref::Xfer &in)
{
  Node & node = resolveNode(*in);
  cdebug(3)<<"resolveChildren for node "<<node.name()<<endl;
  node.resolveChildren();
}

void MeqServer::getNodeList (DataRecord::Ref &out,DataRecord::Ref::Xfer &)
{
  cdebug(2)<<"getNodeList: building list"<<endl;
  DataRecord &list = out <<= new DataRecord;
  int count=0;
  for( int i=1; i<=forest.maxNodeIndex(); i++ )
  {
    const Node::Ref & node = forest.getRef(i);
    if( node.valid() ) // skip deleted nodes
    {
      DataRecord & rec = list[AtomicID(i)] <<= new DataRecord;
      rec[AidName] = node->name();
      rec[AidNodeIndex] = i;
      rec[AidClass] = node->className();
      count++;
    }
  }
  cdebug(2)<<"getNodeList: built list of "<<count<<" nodes"<<endl;
}

//##ModelId=3F98D91B0064
void MeqServer::getNodeResult (DataRecord::Ref &out,DataRecord::Ref::Xfer &in)
{
  DataRecord::Ref rec = in;
  Node & node = resolveNode(*rec);
  cdebug(2)<<"getNodeResult for node "<<node.name()<<endl;
  // take request object out of record
  rec.privatize(DMI::WRITE|DMI::DEEP);
  Request &req = rec()[AidRequest].as_wr<Request>();
  cdebug(3)<<"    request is "<<req.sdebug(DebugLevel-1,"    ")<<endl;
  Result::Ref resref;
  int flags = node.execute(resref,req);
  cdebug(2)<<"  getResult returns flags "<<flags<<" with result"<<endl;
  cdebug(3)<<"    result is "<<resref.sdebug(DebugLevel-1,"    ")<<endl;
  if( DebugLevel>3 && resref.valid() )
  {
    for( int i=0; i<resref->numVellSets(); i++ ) 
    {
      const VellSet &vs = resref->vellSetConst(i);
      if( vs.isFail() ) {
        cdebug(4)<<"  vellset "<<i<<": FAIL"<<endl;
      } else {
        cdebug(4)<<"  vellset "<<i<<": "<<vs.getValue()<<endl;
      }
    }
  }
  out <<= new DataRecord;
  out()[AidResult|AidCode] = flags;
  if( resref.valid() )
    out()[AidResult] <<= resref;
}

//##ModelId=3F608106021C
void MeqServer::run ()
{
  verifySetup(True);
  DataRecord::Ref initrec;
  // keep running as long as start() on the control agent succeeds
  while( control().start(initrec) == AppState::RUNNING )
  {
    // [re]initialize i/o agents with record returned by control
    if( (*initrec)[input().initfield()].exists() )
    {
      cdebug(1)<<"initializing input agent\n";
      if( !input().init(*initrec) )
      {
        control().postEvent(InputInitFailed);
        continue;
      }
    }
    if( (*initrec)[output().initfield()].exists() )
    {
      cdebug(1)<<"initializing output agent\n";
      if( !output().init(*initrec) )
      {
        control().postEvent(OutputInitFailed);
        continue;
      }
    }
    // init the data mux
    data_mux.init(*initrec);
    // get params from control record
    int ntiles = 0;
    DataRecord::Ref header;
    ObjRef cached_header;
    bool reading_data=False,writing_data=False;
    HIID vdsid,datatype;
    
    control().setStatus(StStreamState,"none");
    control().setStatus(StNumTiles,0);
    control().setStatus(StVDSID,vdsid);
    
    // run main loop
    while( control().state() > 0 )  // while in a running state
    {
      // check for any incoming data
      DataRecord::Ref eventrec;
      eventrec.detach();
      cdebug(4)<<"checking input\n";
      HIID id;
      ObjRef ref,header_ref;
      int instat = input().getNext(id,ref,0,AppEvent::WAIT);
      if( instat > 0 )
      { 
        string stage,error_str,output_message;
        HIID output_event;
        try
        {
          // process data event
          if( instat == DATA )
          {
            stage = "processing input DATA event";
            VisTile::Ref tileref = ref.ref_cast<VisTile>().copy(DMI::WRITE);
            cdebug(4)<<"received tile "<<tileref->tileId()<<endl;
            if( !reading_data )
            {
              control().setStatus(StStreamState,"DATA");
              reading_data = True;
            }
            ntiles++;
            if( !(ntiles%100) )
              control().setStatus(StNumTiles,ntiles);
            // deliver tile to data mux
            int result = data_mux.deliverTile(tileref);
            if( result&Node::RES_UPDATED )
            {
              cdebug(3)<<"tile is updated, posting to output"<<endl;
              // post to output only if writing some data
              writing_data = True;
              if( cached_header.valid() ) // but first dump out cached header
              {
                output().put(HEADER,cached_header);
                cached_header.detach();
              }
              output().put(DATA,ref);
            }
          }
          else if( instat == FOOTER )
          {
            cdebug(2)<<"received footer"<<endl;
            reading_data = False;
            eventrec <<= new DataRecord;
            if( header.valid() )
              eventrec()[AidHeader] <<= header.copy();
            if( ref.valid() )
              eventrec()[AidFooter] <<= ref.copy();
            output_event = DataSetFooter;
            output_message = ssprintf("received footer for dataset %s, %d tiles written",
                id.toString().c_str(),ntiles);
            control().setStatus(StStreamState,"END");
            control().setStatus(StNumTiles,ntiles);
            // post to output only if writing some data
            if( writing_data )
              output().put(FOOTER,ref);
          }
          else if( instat == HEADER )
          {
            cdebug(2)<<"received header"<<endl;
            reading_data = writing_data = False;
            cached_header = ref;
            header = cached_header.ref_cast<DataRecord>().copy();
            data_mux.deliverHeader(*header);
            output_event = DataSetHeader;
            output_message = "received header for dataset "+id.toString();
            if( !datatype.empty() )
              output_message += ", " + datatype.toString();
            control().setStatus(StStreamState,"HEADER");
            control().setStatus(StNumTiles,ntiles=0);
            control().setStatus(StVDSID,vdsid = id);
            control().setStatus(FDataType,datatype);
          }
          // generate output event if one was queued up
          if( !output_event.empty() )
            postDataEvent(output_event,output_message,eventrec);
        }
        catch( AipsError &x )
        {
          error_str = x.what();
          cdebug(2)<<"got AipsError while processing input: "<<x.what()<<endl;
        }
        catch( std::exception &exc )
        {
          error_str = exc.what();
          cdebug(2)<<"got exception while processing input: "<<exc.what()<<endl;
        }
        catch( ... )
        {
          error_str = "unknown exception";
          cdebug(2)<<"unknown exception processing input"<<endl;
        }
        // in case of error, generate event
        if( error_str.length() )
        {
          DataRecord::Ref retval(DMI::ANONWR);
          retval()[AidError] = error_str;
          retval()[AidData|AidId] = id;
          control().postEvent(DataProcessingError,retval);
        }
      }
      
      // check for any commands from the control agent
      HIID cmdid;
      DataRecord::Ref cmddata;
      if( control().getCommand(cmdid,cmddata,AppEvent::WAIT) == AppEvent::SUCCESS 
          && cmdid.matches(AppCommandMask) )
      {
        // strip off the App.Control.Command prefix -- the -1 is not very
        // nice because it assumes a wildcard is the last thing in the mask.
        // Which it usually will be
        cmdid = cmdid.subId(AppCommandMask.length()-1);
        cdebug(3)<<"received app command "<<cmdid.toString()<<endl;
        int request_id = 0;
        string error_str;
        DataRecord::Ref retval(DMI::ANONWR);
        try
        {
          cmddata.privatize(DMI::WRITE);
          request_id = (*cmddata)[FRequestId].as<int>(0);
          ObjRef ref = cmddata()[AidArgs].remove();
          DataRecord::Ref args;
          if( ref.valid() )
          {
            FailWhen(!ref->objectType()==TpDataRecord,"invalid args field");
            args = ref.ref_cast<DataRecord>();
          }
          CommandMap::const_iterator iter = command_map.find(cmdid);
          if( iter != command_map.end() )
          {
            // execute the command, catching any errors
            (this->*(iter->second))(retval,args);
          }
          else // command not found
            error_str = "unknown command "+cmdid.toString();
        }
        catch( std::exception &exc )
        {
          error_str = exc.what();
        }
        catch( ... )
        {
          error_str = "unknown exception while processing command";
        }
        // in case of error, insert error message into return value
        if( error_str.length() )
          retval()[AidError] = error_str;
        HIID reply_id = CommandResultPrefix|cmdid;
        if( request_id )
          reply_id |= request_id;
        control().postEvent(reply_id,retval);
      }
      // .. but ignore them since we only watch for state changes anyway
    }
    // go back up for another start() call
  }
  cdebug(1)<<"exiting with control state "<<control().stateString()<<endl;
  control().close();
}

//##ModelId=3F5F195E0156
string MeqServer::sdebug(int detail, const string &prefix, const char *name) const
{
  return "MeqServer";
}

};
