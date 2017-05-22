/* pss_filler_scheduler.hpp
 *
 *  Created by Rong Zhou on 11/18/11.
 *  Copyright 2011-16 Palo Alto Research Center Inc. All rights reserved.
 *
 * class definition for pss single-site filler scheduler
 */
#ifndef PSS_FILLER_SCHEDULER_HPP_INCLUDED_
#define PSS_FILLER_SCHEDULER_HPP_INCLUDED_

#include "pss_scheduler.hpp"

#define PSS_DEFAULT_FILLER_MIN_DELTA_T (600) //10 minutes
#define PSS_DEFAULT_FILLER_MAX_DELTA_T (3600*24) //one day

namespace pss {
class FillerScheduler : public Scheduler {
 private:
  JobList filler_job_pattern_;
  int num_jobs_filled_;
  time_t min_delta_t_;   //min time gap between 2 consecutive filler job batches
  time_t max_delta_t_;   //max time gap between 2 consecutive filler job batches

  /*
  void shift_arrival_due_time(std::vector<ShopJob> &jobs, const time_t shift_t) {
    std::vector<ShopJob>::iterator it;
    for (it = jobs.begin(); it != jobs.end(); ++it) {
      it->arrival += shift_t;
      it->due += shift_t;
    }
  }

  void shift_job_intid(std::vector<ShopJob> &jobs, const int shift) {
    std::vector<ShopJob>::iterator it;
    for (it = jobs.begin(); it != jobs.end(); ++it) {
      it->intid += shift;
    }
  }

  void append_filler_job_pointers(std::vector<ShopJob*> &shop_job_pointers,
    std::vector<ShopJob> &fillerShopJobs) {
    for (std::vector<ShopJob>::iterator it = fillerShopJobs.begin();
      it != fillerShopJobs.end();
      ++it) {
      shop_job_pointers.push_back(&*it);
    }
  } */

 public:
  FillerScheduler(const char *shop_filename, const char *job_filename,
                  const char *sched_filename, const char *jls_filename,
                  const char *filler_job_filename,
                  const time_t min_delta_t_ = PSS_DEFAULT_FILLER_MIN_DELTA_T,
                  const time_t max_delta_t_ = PSS_DEFAULT_FILLER_MAX_DELTA_T);

  void ShiftDate(pss::Date &date, const time_t shift_time);

  void InsertFillerJobs(JobList &job_list,
                        std::vector<ShopJob *> &shop_job_pointers,
                        const JobList &filler_job_pattern_,
                        const time_t shift_t, const int shift_id);

  void RemoveFillerJobs(JobList &job_list,
                        std::vector<ShopJob *> &shop_job_pointers,
                        const unsigned num_filler_jobs);

  void Run(void);

  void PrintInfo(std::ostream &os);
};

} // namespace pss

#endif //PSS_FILLER_SCHEDULER_HPP_INCLUDED_