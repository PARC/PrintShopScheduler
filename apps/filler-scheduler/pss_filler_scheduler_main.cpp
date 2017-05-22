/*  pss_filler_scheduler_main.cpp
 *
 *  Created by Rong Zhou on 05/01/09.
 *  Copyright 2009-16 Palo Alto Research Center Inc. All rights reserved.
 *
 *  main entry point for PSS single-site filler scheduler
 */
#include "pss_filler_scheduler.hpp"

using namespace std;
using namespace pss;

int PssSingleSiteFillSchedule(const char *shop_filename,
                              const char *job_filename,
                              const char *sched_filename,
                              const char *jls_filename,
                              const char *filler_job_filename,
                              const time_t min_delta_t_,
                              const time_t max_delta_t_) {
  try {
    FillerScheduler scheduler(shop_filename, job_filename, sched_filename,
                              jls_filename, filler_job_filename,
                              min_delta_t_, max_delta_t_);
    scheduler.Run();
    scheduler.PrintInfo(std::cout);
  } catch(RuntimeException &e) {
    std::cout << "PSS runtime error: " << e.what() << std::endl;
    return 0;
  }
  return 1;
}

int PssSingleSiteFillMain(int argc, char **argv) {
  char const *shop_filename, *job_filename, *sched_filename,
       *jls_filename, *filler_job_filename;
  time_t min_delta_t_, max_delta_t_;

  if(argc < 6 || argc > 8) {
    std::cerr << "Usage: <shop file> <job file> <output schedule file> "
              "<output job file> <filler job file> "
              "[<filler min delta T> <filler max delta T>]" << std::endl;
    return 0;
  }
  shop_filename = argv[1];
  job_filename = argv[2];
  sched_filename = argv[3];
  jls_filename = argv[4];
  filler_job_filename = argv[5];
  min_delta_t_ = (argc >= 7) ? atoi(argv[6]) : PSS_DEFAULT_FILLER_MIN_DELTA_T;
  max_delta_t_ = (argc == 8) ? atoi(argv[7]) : PSS_DEFAULT_FILLER_MAX_DELTA_T;

  return PssSingleSiteFillSchedule(shop_filename, job_filename, sched_filename,
                                   jls_filename, filler_job_filename,
                                   min_delta_t_, max_delta_t_);
}

int main(int argc, char **argv) {
  return PssSingleSiteFillMain(argc, argv);
}
