//#  jCommon.cc : Holds static OTDB connection, and coded the class converters
//#
//#  Copyright (C) 2002-2006
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/LofarLogger.h>
#include <jOTDB2/nl_astron_lofar_sas_otb_jotdb2_jCommon.h>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include <string>
#include <iostream>

using namespace boost::posix_time;
using namespace LOFAR::OTDB;
using namespace std;

extern TreeMaintenance* theirTM;
extern OTDBconnection*  theirConn;
extern TreeValue*       theirVal;


//
// C++ classes ----> java classes
//



jobject convertOTDBnode (JNIEnv *env, OTDBnode aNode)
{
  jobject jNode;
  jclass class_jOTDBnode = env->FindClass ("nl/astron/lofar/sas/otb/jotdb2/jOTDBnode");
  jmethodID mid_jOTDBnode_cons = env->GetMethodID (class_jOTDBnode, "<init>", "(IIII)V");
  jNode = env->NewObject (class_jOTDBnode, mid_jOTDBnode_cons, aNode.treeID (), aNode.nodeID (), 
			  aNode.parentID (), aNode.paramDefID ());
  
  jfieldID fid_jOTDBnode_name = env->GetFieldID (class_jOTDBnode, "name", "Ljava/lang/String;");
  jfieldID fid_jOTDBnode_index = env->GetFieldID (class_jOTDBnode, "index", "S");
  jfieldID fid_jOTDBnode_leaf = env->GetFieldID (class_jOTDBnode, "leaf", "Z");
  jfieldID fid_jOTDBnode_instances = env->GetFieldID (class_jOTDBnode, "instances", "S");
  jfieldID fid_jOTDBnode_limits = env->GetFieldID (class_jOTDBnode, "limits", "Ljava/lang/String;");
  jfieldID fid_jOTDBnode_description = env->GetFieldID (class_jOTDBnode, "description", "Ljava/lang/String;");
  
  env->SetObjectField (jNode, fid_jOTDBnode_name, env->NewStringUTF (aNode.name.c_str ()));
  env->SetShortField (jNode, fid_jOTDBnode_index, aNode.index);
  env->SetBooleanField (jNode, fid_jOTDBnode_leaf, aNode.leaf);
  env->SetShortField (jNode, fid_jOTDBnode_instances, aNode.instances);
  env->SetObjectField (jNode, fid_jOTDBnode_limits, env->NewStringUTF (aNode.limits.c_str ()));
  env->SetObjectField (jNode, fid_jOTDBnode_description, env->NewStringUTF (aNode.description.c_str ()));
 
  return jNode;	 
}


jobject convertOTDBtree(JNIEnv *env, OTDBtree aTree) {
  jobject jTree;
  jclass class_jOTDBtree = env->FindClass("nl/astron/lofar/sas/otb/jotdb2/jOTDBtree");
  
  jmethodID mid_jOTDBtree_cons = env->GetMethodID (class_jOTDBtree, "<init>", "(II)V");

  jTree = env->NewObject (class_jOTDBtree, mid_jOTDBtree_cons,aTree.treeID (),aTree.momID ());
  

  jfieldID fid_jOTDBtree_classification = env->GetFieldID (class_jOTDBtree, "classification", "S");
  jfieldID fid_jOTDBtree_creator = env->GetFieldID (class_jOTDBtree, "creator", "Ljava/lang/String;");
  jfieldID fid_jOTDBtree_creationDate = env->GetFieldID (class_jOTDBtree, "creationDate", "Ljava/lang/String;");
  jfieldID fid_jOTDBtree_type = env->GetFieldID (class_jOTDBtree, "type", "S");
  jfieldID fid_jOTDBtree_state = env->GetFieldID (class_jOTDBtree, "state", "S");
  jfieldID fid_jOTDBtree_originalTree = env->GetFieldID (class_jOTDBtree, "originalTree", "I");
  jfieldID fid_jOTDBtree_campaign = env->GetFieldID (class_jOTDBtree, "campaign", "Ljava/lang/String;");
  jfieldID fid_jOTDBtree_starttime = env->GetFieldID (class_jOTDBtree, "starttime","Ljava/lang/String;");
  jfieldID fid_jOTDBtree_stoptime = env->GetFieldID (class_jOTDBtree, "stoptime", "Ljava/lang/String;");
  jfieldID fid_jOTDBtree_description = env->GetFieldID (class_jOTDBtree, "description", "Ljava/lang/String;");
  
  env->SetShortField (jTree, fid_jOTDBtree_classification, aTree.classification);
  env->SetObjectField (jTree, fid_jOTDBtree_creator, env->NewStringUTF (aTree.creator.c_str ()));
  env->SetObjectField (jTree, fid_jOTDBtree_creationDate, env->NewStringUTF (to_simple_string(aTree.creationDate).c_str ()));
  env->SetShortField (jTree, fid_jOTDBtree_type, aTree.type);
  env->SetShortField (jTree, fid_jOTDBtree_state, aTree.state);
  env->SetIntField (jTree, fid_jOTDBtree_originalTree, aTree.originalTree);
  env->SetObjectField (jTree, fid_jOTDBtree_campaign, env->NewStringUTF (aTree.campaign.c_str ()));
  env->SetObjectField (jTree, fid_jOTDBtree_starttime, env->NewStringUTF(to_simple_string(aTree.starttime).c_str ()));
  env->SetObjectField (jTree, fid_jOTDBtree_stoptime, env->NewStringUTF (to_simple_string(aTree.stoptime).c_str ()));
  env->SetObjectField (jTree, fid_jOTDBtree_description, env->NewStringUTF (aTree.description.c_str()));
  
  return jTree;
}

jobject convertTreeState (JNIEnv *env, TreeState aTreeState)
{
  jobject jTreeState;
  jclass class_jTreeState = env->FindClass ("nl/astron/lofar/otb/jotdb2/jTreeState");
  jmethodID mid_jTreeState_cons = env->GetMethodID (class_jTreeState, "<init>", "()V");
  jTreeState = env->NewObject (class_jTreeState, mid_jTreeState_cons);


  jfieldID fid_jTreeState_treeID = env->GetFieldID (class_jTreeState, "treeID", "I");
  jfieldID fid_jTreeState_momID = env->GetFieldID (class_jTreeState, "momID", "I");
  jfieldID fid_jTreeState_newState = env->GetFieldID (class_jTreeState,"newState", "S");
  jfieldID fid_jTreeState_username = env->GetFieldID (class_jTreeState, "username", "Ljava/lang/String;");
  jfieldID fid_jTreeState_timestamp = env->GetFieldID (class_jTreeState, "timestamp", "Ljava/lang/String;");

  env->SetIntField (jTreeState, fid_jTreeState_treeID, aTreeState.treeID);
  env->SetIntField (jTreeState, fid_jTreeState_momID, aTreeState.momID);
  env->SetShortField (jTreeState, fid_jTreeState_newState, aTreeState.newState);
  env->SetObjectField (jTreeState, fid_jTreeState_username, env->NewStringUTF(aTreeState.username.c_str()));
  env->SetObjectField (jTreeState, fid_jTreeState_timestamp, env->NewStringUTF(to_simple_string(aTreeState.timestamp).c_str()));

  return jTreeState;
}

jobject convertVICnodeDef (JNIEnv *env, VICnodeDef aNodeDef)
{

  jobject jVICNodeDef;
  jclass class_jVICnodeDef = env->FindClass ("nl/astron/lofar/sas/otb/jotdb2/jVICnodeDef");
  jmethodID mid_jVICnodeDef_cons = env->GetMethodID (class_jVICnodeDef, "<init>", "()V");
  jVICNodeDef = env->NewObject (class_jVICnodeDef, mid_jVICnodeDef_cons);
  
  jfieldID fid_jVICnodeDef_name = env->GetFieldID (class_jVICnodeDef, "name", "Ljava/lang/String;");
  jfieldID fid_jVICnodeDef_version = env->GetFieldID (class_jVICnodeDef, "version", "I");
  jfieldID fid_jVICnodeDef_classif = env->GetFieldID (class_jVICnodeDef, "classif", "S");
  jfieldID fid_jVICnodeDef_constraints = env->GetFieldID (class_jVICnodeDef, "constraints", "Ljava/lang/String;");
  jfieldID fid_jVICnodeDef_description = env->GetFieldID (class_jVICnodeDef, "description", "Ljava/lang/String;");
  jfieldID fid_jVICnodeDef_itsNodeID = env->GetFieldID (class_jVICnodeDef, "itsNodeID", "I");
  
  env->SetObjectField (jVICNodeDef, fid_jVICnodeDef_name, env->NewStringUTF (aNodeDef.name.c_str()));
  env->SetIntField (jVICNodeDef, fid_jVICnodeDef_version, aNodeDef.version);
  env->SetShortField (jVICNodeDef, fid_jVICnodeDef_classif, aNodeDef.classif);
  env->SetObjectField (jVICNodeDef, fid_jVICnodeDef_constraints, env->NewStringUTF (aNodeDef.constraints.c_str()));
  env->SetObjectField (jVICNodeDef, fid_jVICnodeDef_description, env->NewStringUTF (aNodeDef.description.c_str()));
  env->SetIntField (jVICNodeDef, fid_jVICnodeDef_itsNodeID, aNodeDef.nodeID());
  
  return jVICNodeDef;
}

jobject convertOTDBparam (JNIEnv *env, OTDBparam aParam)
{

  jobject jOTDBparam;
  jclass class_jOTDBparam = env->FindClass ("nl/astron/lofar/sas/otb/jotdb2/jOTDBparam");
  jmethodID mid_jOTDBparam_cons = env->GetMethodID (class_jOTDBparam, "<init>", "(III)V");
  jOTDBparam = env->NewObject (class_jOTDBparam, mid_jOTDBparam_cons,aParam.treeID(),aParam.paramID(),aParam.nodeID());
  
  jfieldID fid_jOTDBparam_name = env->GetFieldID (class_jOTDBparam, "name", "Ljava/lang/String;");
  jfieldID fid_jOTDBparam_index = env->GetFieldID (class_jOTDBparam, "index", "S");
  jfieldID fid_jOTDBparam_type = env->GetFieldID (class_jOTDBparam, "type", "S");
    jfieldID fid_jOTDBparam_unit = env->GetFieldID (class_jOTDBparam, "unit", "S");
    jfieldID fid_jOTDBparam_pruning = env->GetFieldID (class_jOTDBparam, "pruning", "S");
    jfieldID fid_jOTDBparam_valMoment = env->GetFieldID (class_jOTDBparam, "valMoment", "S");
    jfieldID fid_jOTDBparam_runtimeMod = env->GetFieldID (class_jOTDBparam, "runtimeMod", "Z");
  jfieldID fid_jOTDBparam_limits = env->GetFieldID (class_jOTDBparam, "limits", "Ljava/lang/String;");
  jfieldID fid_jOTDBparam_description = env->GetFieldID (class_jOTDBparam, "description", "Ljava/lang/String;");


  
  env->SetObjectField (jOTDBparam, fid_jOTDBparam_name, env->NewStringUTF (aParam.name.c_str()));
  env->SetShortField (jOTDBparam, fid_jOTDBparam_index, aParam.index);
  env->SetShortField (jOTDBparam, fid_jOTDBparam_type, aParam.type);
  env->SetShortField (jOTDBparam, fid_jOTDBparam_unit, aParam.unit);
  env->SetShortField (jOTDBparam, fid_jOTDBparam_pruning, aParam.pruning);
  env->SetShortField (jOTDBparam, fid_jOTDBparam_valMoment, aParam.valMoment);
  env->SetBooleanField (jOTDBparam, fid_jOTDBparam_runtimeMod, aParam.runtimeMod);
  env->SetObjectField (jOTDBparam, fid_jOTDBparam_limits, env->NewStringUTF (aParam.limits.c_str()));  
  env->SetObjectField (jOTDBparam, fid_jOTDBparam_description, env->NewStringUTF (aParam.description.c_str()));
  
  return jOTDBparam;
}


//
// java classes ----> c++ classes
//



OTDBnode convertjOTDBnode (JNIEnv *env, jobject jNode)
{
  jclass class_jOTDBnode = env->GetObjectClass (jNode);
  jfieldID fid_jOTDBnode_name = env->GetFieldID (class_jOTDBnode, "name", "Ljava/lang/String;");
  jfieldID fid_jOTDBnode_index = env->GetFieldID (class_jOTDBnode, "index", "S");
  jfieldID fid_jOTDBnode_leaf = env->GetFieldID (class_jOTDBnode, "leaf", "Z");
  jfieldID fid_jOTDBnode_instances = env->GetFieldID (class_jOTDBnode, "instances", "S");
  jfieldID fid_jOTDBnode_limits = env->GetFieldID (class_jOTDBnode, "limits", "Ljava/lang/String;");
  jfieldID fid_jOTDBnode_description = env->GetFieldID (class_jOTDBnode, "description", "Ljava/lang/String;");
  jmethodID mid_jOTDBnode_treeID = env->GetMethodID (class_jOTDBnode, "treeID", "()I");
  jmethodID mid_jOTDBnode_nodeID = env->GetMethodID (class_jOTDBnode, "nodeID", "()I");
  
  // Get original OTDB node
  OTDBnode aNode = theirTM->getNode (env->CallIntMethod (jNode, mid_jOTDBnode_treeID), 
				      env->CallIntMethod (jNode, mid_jOTDBnode_nodeID));		       
  

  // name
  jstring str = (jstring)env->GetObjectField (jNode, fid_jOTDBnode_name);
  jboolean isCopy;
  const char* n = env->GetStringUTFChars (str, &isCopy);
  const string name (n);
  aNode.name = name;
  env->ReleaseStringUTFChars (str, n);
  
  // index
  aNode.index = (short)env->GetShortField (jNode, fid_jOTDBnode_index);
  
  // leaf
  aNode.leaf = (bool)env->GetBooleanField (jNode, fid_jOTDBnode_leaf);
  
  // instances
  aNode.instances = (short)env->GetShortField (jNode, fid_jOTDBnode_instances);
  
  // limits
  str = (jstring)env->GetObjectField (jNode, fid_jOTDBnode_limits);
  const char* l = env->GetStringUTFChars (str, &isCopy);
  const string limits (l);
  aNode.limits = limits;
  env->ReleaseStringUTFChars (str, l);
  
  // description
  str = (jstring)env->GetObjectField (jNode, fid_jOTDBnode_description);
  const char* d = env->GetStringUTFChars (str, &isCopy);
  const string description (d);
  aNode.description = description;
  env->ReleaseStringUTFChars (str, d);
  
  return aNode;
}


VICnodeDef convertjVICnodeDef (JNIEnv *env, jobject jNode)
{
  jclass class_jVICnodeDef = env->GetObjectClass (jNode);
  jfieldID fid_jVICnodeDef_name = env->GetFieldID (class_jVICnodeDef, "name", "Ljava/lang/String;");
  jfieldID fid_jVICnodeDef_version = env->GetFieldID (class_jVICnodeDef, "version", "I");
  jfieldID fid_jVICnodeDef_classif = env->GetFieldID (class_jVICnodeDef, "classif", "S");
  jfieldID fid_jVICnodeDef_constraints = env->GetFieldID (class_jVICnodeDef, "constraints", "Ljava/lang/String;");
  jfieldID fid_jVICnodeDef_description = env->GetFieldID (class_jVICnodeDef, "description", "Ljava/lang/String;");
  jmethodID mid_jVICnodeDef_nodeID = env->GetMethodID (class_jVICnodeDef, "nodeID", "()I");


  // Get original VICnodeDef 
  VICnodeDef aNode = theirTM->getComponentNode(env->CallIntMethod (jNode, mid_jVICnodeDef_nodeID));		       
  
  // name
  jstring str = (jstring)env->GetObjectField (jNode, fid_jVICnodeDef_name);
  jboolean isCopy;
  const char* n = env->GetStringUTFChars (str, &isCopy);
  const string name (n);
  aNode.name = name;
  env->ReleaseStringUTFChars (str, n);
  
  // version
  aNode.version = (int)env->GetIntField (jNode, fid_jVICnodeDef_version);

  // classif
  aNode.classif = (short)env->GetShortField (jNode, fid_jVICnodeDef_classif);
  
  // constraints
  str = (jstring)env->GetObjectField (jNode, fid_jVICnodeDef_constraints);
  const char* l = env->GetStringUTFChars (str, &isCopy);
  const string constraints (l);
  aNode.constraints = constraints;
  env->ReleaseStringUTFChars (str, l);
  
  // description
  str = (jstring)env->GetObjectField (jNode, fid_jVICnodeDef_description);
  const char* d = env->GetStringUTFChars (str, &isCopy);
  const string description (d);
  aNode.description = description;
  env->ReleaseStringUTFChars (str, d);
  
  return aNode;
}
 
OTDBparam convertjOTDBparam (JNIEnv *env, jobject jParam) {
  jclass class_jOTDBparam = env->FindClass ("nl/astron/lofar/sas/otb/jotdb2/jOTDBparam");
  jfieldID fid_jOTDBparam_name = env->GetFieldID (class_jOTDBparam, "name", "Ljava/lang/String;");
  jfieldID fid_jOTDBparam_index = env->GetFieldID (class_jOTDBparam, "index", "S");
  jfieldID fid_jOTDBparam_type = env->GetFieldID (class_jOTDBparam, "type", "S");
  jfieldID fid_jOTDBparam_unit = env->GetFieldID (class_jOTDBparam, "unit", "S");
  jfieldID fid_jOTDBparam_pruning = env->GetFieldID (class_jOTDBparam, "pruning", "S");
  jfieldID fid_jOTDBparam_valMoment = env->GetFieldID (class_jOTDBparam, "valMoment", "S");
  jfieldID fid_jOTDBparam_runtimeMod = env->GetFieldID (class_jOTDBparam, "runtimeMod", "Z");
  jfieldID fid_jOTDBparam_limits = env->GetFieldID (class_jOTDBparam, "limits", "Ljava/lang/String;");
  jfieldID fid_jOTDBparam_description = env->GetFieldID (class_jOTDBparam, "description", "Ljava/lang/String;");
  jmethodID mid_jOTDBparam_treeID = env->GetMethodID (class_jOTDBparam, "treeID", "()I");
  jmethodID mid_jOTDBparam_paramID = env->GetMethodID (class_jOTDBparam, "paramID", "()I");
  
  // Get original OTDB param
  OTDBparam aParam = theirTM->getParam (env->CallIntMethod (jParam, mid_jOTDBparam_treeID), 
					 env->CallIntMethod (jParam, mid_jOTDBparam_paramID));
  
  // name
  jstring str = (jstring)env->GetObjectField (jParam, fid_jOTDBparam_name);
  jboolean isCopy;
  const char* n = env->GetStringUTFChars (str, &isCopy);
  const string name (n);
  aParam.name = name;
  env->ReleaseStringUTFChars (str, n);
  
  aParam.index = (short)env->GetShortField (jParam, fid_jOTDBparam_index);
  aParam.type = (short)env->GetShortField (jParam, fid_jOTDBparam_type);
  aParam.unit = (short)env->GetShortField (jParam, fid_jOTDBparam_unit);
  aParam.pruning = (short)env->GetShortField (jParam, fid_jOTDBparam_pruning);
  aParam.valMoment = (short)env->GetShortField (jParam, fid_jOTDBparam_valMoment);
  aParam.runtimeMod = (short)env->GetBooleanField (jParam, fid_jOTDBparam_runtimeMod);
  
  // limits
  str = (jstring)env->GetObjectField (jParam, fid_jOTDBparam_limits);
  const char* l = env->GetStringUTFChars (str, &isCopy);
  const string limits (l);
  aParam.limits = limits;
  env->ReleaseStringUTFChars (str, l);
  
  // description
  str = (jstring)env->GetObjectField (jParam, fid_jOTDBparam_description);
  const char* d = env->GetStringUTFChars (str, &isCopy);
  const string description (d);
  aParam.description = description;
  env->ReleaseStringUTFChars (str, d);

  return aParam;
}

OTDBvalue convertjOTDBvalue (JNIEnv *env, jobject jvalue)
{
  jclass class_jOTDBvalue = env->GetObjectClass (jvalue);
  jfieldID fid_jOTDBvalue_name = env->GetFieldID (class_jOTDBvalue, "name", "Ljava/lang/String;");
  jfieldID fid_jOTDBvalue_value = env->GetFieldID (class_jOTDBvalue, "value", "Ljava/lang/String;");
  jfieldID fid_jOTDBvalue_time = env->GetFieldID (class_jOTDBvalue, "time", "Ljava/lang/String;");
  jmethodID mid_jOTDBvalue_nodeID = env->GetMethodID (class_jOTDBvalue, "nodeID", "()I");
  
  // Get original OTDB node
  OTDBvalue aValue;// = new OTDBvalue();

  // name
  jstring nstr = (jstring)env->GetObjectField (jvalue, fid_jOTDBvalue_name);
  const char* n = env->GetStringUTFChars (nstr, 0);
  const string name (n);
  aValue.name = name;
  env->ReleaseStringUTFChars (nstr, n);

  // value
  jstring vstr = (jstring)env->GetObjectField (jvalue, fid_jOTDBvalue_value);
  const char* v = env->GetStringUTFChars (vstr, 0);
  const string value (v);
  aValue.value = value;
  env->ReleaseStringUTFChars (vstr, v);

  // time
  jstring tstr = (jstring)env->GetObjectField (jvalue, fid_jOTDBvalue_time);
  const char* t = env->GetStringUTFChars (tstr, 0);
  const string time (t);
  const ptime apt (time_from_string(time)); 
  aValue.time = apt;
  env->ReleaseStringUTFChars (tstr, t);

  return aValue;
}

