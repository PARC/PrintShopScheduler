/*  pss_multisite_scheduler_main.cpp
 *
 *  Created by Rong Zhou on 05/01/09.
 *  Copyright 2009-16 Palo Alto Research Center Inc. All rights reserved.
 *
 *  main entry point for PSS multi-site scheduler
 */
#include "pss_multisite_scheduler.hpp"

using namespace std;
using namespace pss;

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

int PssMultiSiteMain(int argc, char **argv) {
  char const *ms_shop_filename, *ms_job_filename;
  char const *ms_sched_filename, *ms_jls_filename;

  if(argc < 4 || argc > 5) {
    std::cerr << "Usage: <multi-site shop file> <multi-site job file> "
              "<multi-site output schedule file> [<multi-site output job file>]"
              << std::endl;
    return 0;
  }
  ms_shop_filename = argv[1];
  ms_job_filename = argv[2];
  ms_sched_filename = argv[3];
  ms_jls_filename = (argc == 5) ? argv[4] : NULL;
  return PssMultiSiteSchedule(ms_shop_filename, ms_job_filename,
                              ms_sched_filename, ms_jls_filename);
}

int main(int argc, char **argv) {
  return PssMultiSiteMain(argc, argv);
}
