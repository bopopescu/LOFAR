/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class nl_astron_lofar_sas_otb_jotdb3_jTreeTypeConv */

#ifndef _Included_nl_astron_lofar_sas_otb_jotdb3_jTreeTypeConv
#define _Included_nl_astron_lofar_sas_otb_jotdb3_jTreeTypeConv
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeTypeConv
 * Method:    initTreeTypeConv
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeTypeConv_initTreeTypeConv
  (JNIEnv *, jobject);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeTypeConv
 * Method:    get
 * Signature: (Ljava/lang/String;)S
 */
JNIEXPORT jshort JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeTypeConv_get__Ljava_lang_String_2
  (JNIEnv *, jobject, jstring);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeTypeConv
 * Method:    get
 * Signature: (S)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeTypeConv_get__S
  (JNIEnv *, jobject, jshort);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeTypeConv
 * Method:    getTypes
 * Signature: ()Ljava/util/HashMap;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeTypeConv_getTypes
  (JNIEnv *, jobject);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeTypeConv
 * Method:    top
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeTypeConv_top
  (JNIEnv *, jobject);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jTreeTypeConv
 * Method:    next
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jTreeTypeConv_next
  (JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif
