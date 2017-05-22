/*  pss_scheduler.cpp
 *
 *  Created by Rong Zhou on 06/23/10.
 *  Copyright 2010 Palo Alto Research Center Inc. All rights reserved.
 *
 *  implementation file for pss single-site scheduler
 */

#include "pss_scheduler.hpp"
#include "pss_jobs_writer.hpp"

using namespace std;

namespace pss {

Scheduler::Scheduler(const char *shop_filename, const char *job_filename,
                     const char *sched_filename, const char *jls_filename) :
  shop_(shop_filename),
  job_list_(job_filename, shop_.GetInfo(), 0),
  sched_file_(sched_filename) {
  if(jls_filename) {
    jls_file_.open(jls_filename);
  }
}

void Scheduler::Run(void) {
  vector<pss::ShopJob> &jobs(job_list_.jobs_);
  JobListModel &list(job_list_.model_);
  const ShopInfo &shop_info(shop_.GetInfo());
  Rsrc2Tintvl mach2tintvl, opr2tintvl;
  vector<ShopJob *> shop_job_pointers;
  job_list_.ShopJobPointers(shop_job_pointers);
  clock_t start = clock();
  DoSchedule(scheds_, jobs, shop_info, mach2tintvl, opr2tintvl, shop_job_pointers);
  stats_.cpu_sec = (clock() - start) / (double) CLOCKS_PER_SEC;
  GetSchedStats(stats_, shop_job_pointers, mach2tintvl);
  PrintFuncSched(sched_file_, scheds_, shop_job_pointers, shop_info.seq2func, ';');
  if(jls_file_.is_open()) {
    vector<Job *> raw_job_pointers;
    job_list_.RawJobPointers(raw_job_pointers);
    WriteScheduleEvents(raw_job_pointers, scheds_, shop_job_pointers, shop_info,
                        mach2tintvl, shop_job_pointers);
    jls_file_ << list;
    jls_file_.close();
  }
}

void Scheduler::PrintInfo(ostream &os) {
  PrintShopConfig(os, shop_.GetInfo());
  PrintSchedStats(os, stats_, 0, false);
}

} // namespace pss
