/*  pss_scheduler.hpp
 *
 *  Created by Rong Zhou on 06/23/10.
 *  Copyright 2010 Palo Alto Research Center Inc. All rights reserved.
 *
 *  class definition for pss single-site scheduler
 */

#ifndef PSS_SCHEDULER_HPP_INCLUDED_
#define PSS_SCHEDULER_HPP_INCLUDED_

#include "pss_shop.hpp"
#include "pss_job_list.hpp"
#include "pss_sched_utils.hpp"

namespace pss {

class Scheduler {
 protected:
  Shop shop_;
  JobList job_list_;
  std::map<unsigned, Sched> scheds_;
  std::ofstream sched_file_;
  std::ofstream jls_file_;
  SchedStats stats_;

 public:
  Scheduler(const char *shop_filename, const char *job_filename,
            const char *sched_filename, const char *jls_filename);
  void Run(void);
  void PrintInfo(std::ostream &os);
};

} // namespace pss

#endif //PSS_SCHEDULER_HPP_INCLUDED_