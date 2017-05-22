/* pss_job_list.hpp
 *
 *  Created by Rong Zhou on 06/23/10.
 *  Copyright 2010 Palo Alto Research Center Inc. All rights reserved.
 *
 *  class definition for pss job list
 */

#ifndef PSS_JOB_LIST_HPP_INCLUDED_
#define PSS_JOB_LIST_HPP_INCLUDED_

#include "pss_jobs_parser.hpp"
#include "pss_shop_job.hpp"

namespace pss {
class JobList {
 private:
  unsigned minjobid_;
  JobListModel model_;
  std::vector<ShopJob> jobs_;

 public:
  JobList(const char *filename, const ShopInfo &shopinfo, const unsigned minjobid) {
    JobsParser jobs_parser(model_);
    jobs_parser.ParseFile(filename);
    this->minjobid_ = minjobid;
    GetShopJobs(jobs_, model_.jobs, shopinfo, minjobid);
  }

  friend class Scheduler;

  friend class FillerScheduler;

  friend class MultisiteScheduler;

  const std::vector<ShopJob> &GetJobs(void) const {
    return jobs_;
  }

  void AppendShopJobPointers(std::vector<ShopJob *> &shop_job_pointers) {
    for(std::vector<ShopJob>::iterator it = jobs_.begin();
        it != jobs_.end();
        ++it) {
      shop_job_pointers.push_back(&*it);
    }
  }

  void ShopJobPointers(std::vector<ShopJob *> &shop_job_pointers) {
    shop_job_pointers.clear();
    AppendShopJobPointers(shop_job_pointers);
  }

  void RawJobPointers(std::vector<Job *> &raw_job_pointers) {
    raw_job_pointers.clear();
    for(std::vector<Job>::iterator it = model_.jobs.begin();
        it != model_.jobs.end();
        ++it) {
      raw_job_pointers.push_back(&*it);
    }
  }

  const JobListModel &GetList(void) const {
    return model_;
  }

  int size(void) const {
    return (int)jobs_.size();
  }
};
} // namespace pss

#endif // PSS_JOB_LIST_HPP_INCLUDED_
