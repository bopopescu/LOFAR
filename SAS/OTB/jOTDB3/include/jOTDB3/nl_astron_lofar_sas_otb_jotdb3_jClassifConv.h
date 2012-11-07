#ifndef __nl_astron_lofar_sas_otb_jotdb3_jClassifConv__
#define __nl_astron_lofar_sas_otb_jotdb3_jClassifConv__

#include <jni.h>

#ifdef __cplusplus
extern "C"
{
#endif

JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jClassifConv_initClassifConv (JNIEnv *env, jobject);
JNIEXPORT jshort JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jClassifConv_get__Ljava_lang_String_2 (JNIEnv *env, jobject, jstring);
JNIEXPORT jstring JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jClassifConv_get__S (JNIEnv *env, jobject, jshort);
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jClassifConv_getTypes (JNIEnv *env, jobject);
JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jClassifConv_top (JNIEnv *env, jobject);
JNIEXPORT jboolean JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jClassifConv_next (JNIEnv *env, jobject);

#ifdef __cplusplus
}
#endif

#endif /* __nl_astron_lofar_sas_otb_jotdb3_jClassifConv__ */