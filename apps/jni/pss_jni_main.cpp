/*  pss_jni_main.cpp
 *
 *  Created by Rong Zhou on 05/01/09.
 *  Copyright 2009-16 Palo Alto Research Center Inc. All rights reserved.
 *
 *  main entry point for PSS Scheduler JNI library
 */
#include "pss_scheduler.hpp"
#include "pss_multisite_scheduler.hpp"
#include "com_parc_pss_SchedulerInterface.h"

using namespace std;
using namespace pss;

int PssSingleSiteSchedule(const char *shop_filename,
                          const char *job_filename,
                          const char *sched_filename,
                          const char *jls_filename) {
  try {
    Scheduler scheduler(shop_filename, job_filename,
                        sched_filename, jls_filename);
    scheduler.Run();
    scheduler.PrintInfo(std::cout);
  } catch(RuntimeException &e) {
    std::cout << "PSS runtime error: " << e.what() << std::endl;
    return 0;
  }
  return 1;
}

int PssMultiSiteSchedule(const char *ms_shop_filename,
                         const char *ms_job_filename,
                         const char *ms_sched_filename,
                         const char *ms_jls_filename) {
  try {
    MultisiteScheduler scheduler(ms_shop_filename, ms_job_filename,
                                 ms_sched_filename, ms_jls_filename);
    scheduler.Run();
    scheduler.PrintInfo(std::cout);
  } catch(RuntimeException &e) {
    std::cout << "PSS runtime error: " << e.what() << std::endl;
    return 0;
  }
  return 1;
}


JNIEXPORT jint JNICALL Java_com_parc_pss_SchedulerInterface_Scheduler
(JNIEnv *env, jobject obj, jstring shopfname, jstring jobfname,
 jstring schedfname, jstring jlsfname) {
  char shop_filename[200], job_filename[200],
       sched_filename[200], jls_filename[200];
  const char *str;
  (void)obj; //avoid warning
  str = (env)->GetStringUTFChars(shopfname, NULL);
  Snprintf(shop_filename, sizeof(shop_filename), "%s", str);
  (env)->ReleaseStringUTFChars(shopfname, str);
  str = (env)->GetStringUTFChars(jobfname, NULL);
  Snprintf(job_filename, sizeof(job_filename), "%s", str);
  (env)->ReleaseStringUTFChars(jobfname, str);
  str = (env)->GetStringUTFChars(schedfname, NULL);
  Snprintf(sched_filename, sizeof(sched_filename), "%s", str);
  (env)->ReleaseStringUTFChars(schedfname, str);
  str = (env)->GetStringUTFChars(jlsfname, NULL);
  Snprintf(jls_filename, sizeof(jls_filename), "%s", str);
  (env)->ReleaseStringUTFChars(jlsfname, str);
  return PssSingleSiteSchedule(shop_filename, job_filename,
                               sched_filename, jls_filename);
}

JNIEXPORT jint JNICALL Java_com_parc_pss_SchedulerInterface_MultisiteScheduler
(JNIEnv *env, jobject obj, jstring shopfname, jstring jobfname,
 jstring schedfname, jstring jlsfname) {
  char shop_filename[200], job_filename[200],
       sched_filename[200], jls_filename[200];
  const char *str;
  (void)obj; //avoid warning
  str = (env)->GetStringUTFChars(shopfname, NULL);
  Snprintf(shop_filename, sizeof(shop_filename), "%s", str);
  (env)->ReleaseStringUTFChars(shopfname, str);
  str = (env)->GetStringUTFChars(jobfname, NULL);
  Snprintf(job_filename, sizeof(job_filename), "%s", str);
  (env)->ReleaseStringUTFChars(jobfname, str);
  str = (env)->GetStringUTFChars(schedfname, NULL);
  Snprintf(sched_filename, sizeof(sched_filename), "%s", str);
  (env)->ReleaseStringUTFChars(schedfname, str);
  str = (env)->GetStringUTFChars(jlsfname, NULL);
  Snprintf(jls_filename, sizeof(jls_filename), "%s", str);
  (env)->ReleaseStringUTFChars(jlsfname, str);
  return PssMultiSiteSchedule(shop_filename, job_filename,
                              sched_filename, jls_filename);
}
