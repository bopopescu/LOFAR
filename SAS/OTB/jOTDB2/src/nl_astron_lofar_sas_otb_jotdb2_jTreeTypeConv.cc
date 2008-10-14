//#  jTreeTypeConv.cc: 
//#
//#  Copyright (C) 2002-2007
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

//# Includes
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <jni.h>
#include <jOTDB2/nl_astron_lofar_sas_otb_jotdb2_jTreeTypeConv.h>
#include <OTDB/OTDBtypes.h>
#include <string>
#include <iostream>
#include <jOTDB2/nl_astron_lofar_sas_otb_jotdb2_jCommon.h>
#include <OTDB/TreeTypeConv.h>

using namespace LOFAR::OTDB;
using namespace std;


static TreeTypeConv* treeTypeConv;
extern OTDBconnection* theirConn;

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeTypeConv
 * Method:    initTreeTypeConv
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeTypeConv_initTreeTypeConv(JNIEnv *env, jobject) {
  try {
    treeTypeConv = new TreeTypeConv(theirConn);
  } catch (exception &ex) {
    cout << "Exception during new TreeTypeConv::top" << ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeTypeConv
 * Method:    get
 * Signature: (Ljava/lang/String;)S
 */
JNIEXPORT jshort JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeTypeConv_get__Ljava_lang_String_2(JNIEnv *env, jobject, jstring aConv) {
  
  const char* chars = env->GetStringUTFChars (aConv, 0);
  const string str (chars);
  
  short ret;
  try {
    ret= treeTypeConv->get (str);
  
    env->ReleaseStringUTFChars (aConv, chars);	     
  } catch (exception &ex) {
    cout << "Exception during TreeTypeConv::get(" << str << ") " << ex.what() << endl; 

    env->ReleaseStringUTFChars (aConv, chars);	     
    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  
  return ret;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeTypeConv
 * Method:    get
 * Signature: (S)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeTypeConv_get__S(JNIEnv *env, jobject, jshort aConv) {
  jstring jstr;
  try {
    jstr= env->NewStringUTF (treeTypeConv->get(aConv).c_str());
  } catch (exception &ex) {
    cout << "Exception during TreeTypeConv::get(" << aConv << ") " << ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return jstr;
}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeTypeConv
 * Method:    getTypes
 * Signature: ()Ljava/util/HashMap;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeTypeConv_getTypes(JNIEnv *env, jobject) {

  
  // Construct java Map
  jobject result;
  jclass mapClass, shortClass;
  jmethodID mapInit, put, shortInit;
    
  mapClass = env->FindClass("java/util/HashMap");
  mapInit = env->GetMethodID(mapClass, "<init>", "()V");
  result = env->NewObject(mapClass, mapInit);

  shortClass = env->FindClass("java/lang/Short");
  shortInit = env->GetMethodID(shortClass, "<init>", "(S)V");
  

  if ( env->ExceptionOccurred() )
    return 0;
    
  put= env->GetMethodID(mapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

  if ( env->ExceptionOccurred() )
    return 0;
 
  try {

    treeTypeConv->top();
    do {
      treeType key;
      string value;

      if (treeTypeConv->get(key, value)) {
        env->CallObjectMethod(result, put, env->NewObject(shortClass,
							  shortInit,
							  (jshort)key), 
							  env->NewStringUTF(value.c_str()));

        if ( env->ExceptionOccurred() )
  	  return 0;
     }
    } while (treeTypeConv->next());
  } catch (exception &ex) {
    cout << "Exception during TreeTypeConv::getTypes(" << ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return result;

}

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeTypeConv
 * Method:    top
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeTypeConv_top(JNIEnv *env, jobject) {
  try {
    treeTypeConv->top();
  } catch (exception &ex) {
    cout << "Exception during TreeTypeConv::top(" << ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

}


/*
 * Class:     nl_astron_lofar_sas_otb_jotdb2_jTreeTypeConv
 * Method:    next
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb2_jTreeTypeConv_next(JNIEnv *env, jobject) {
  jboolean aBool;
  try {
    aBool=treeTypeConv->next();
  } catch (exception &ex) {
    cout << "Exception during TreeTypeConv::next(" << ex.what() << endl; 

    env->ThrowNew(env->FindClass("java/lang/Exception"),ex.what());
  }

  return aBool;
}
