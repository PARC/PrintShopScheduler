/*  pss_scheduler_main.cpp
 *
 *  Created by Rong Zhou on 05/01/09.
 *  Copyright 2009-16 Palo Alto Research Center Inc. All rights reserved.
 *
 *  main entry point for PSS single-site scheduler
 */
#include "pss_scheduler.hpp"

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

int PssSingleSiteMain(int argc, char **argv) {
  char const *shop_filename, *job_filename, *sched_filename, *jls_filename;

  if(argc < 4 || argc > 5) {
    std::cerr << "Usage: <shop file> <job file> <output schedule file> "
              "[<output job file>]" << std::endl;
    return 0;
  }
  shop_filename = argv[1];
  job_filename = argv[2];
  sched_filename = argv[3];
  jls_filename = (argc == 5) ? argv[4] : NULL;
  return PssSingleSiteSchedule(shop_filename, job_filename, sched_filename,
                               jls_filename);
}

int main(int argc, char **argv) {
  return PssSingleSiteMain(argc, argv);
}
