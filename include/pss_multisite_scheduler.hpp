/* pss_multisite_scheduler.hpp
 *
 *  Created by Rong Zhou on 06/23/10.
 *  Copyright 2010 Palo Alto Research Center Inc. All rights reserved.
 *
 *  class definition for pss multi-site scheduler
 */

#ifndef PSS_MULTISITE_SCHEDULER_HPP_INCLUDED_
#define PSS_MULTISITE_SCHEDULER_HPP_INCLUDED_

#include <boost/thread.hpp>
#include "pss_sched_utils.hpp"
#include "pss_multisite_job_list.hpp"

namespace pss {
class MultisiteScheduler {
 private:
  pss::MultisiteShop shop_;
  pss::MultisiteJobList job_list_;
  std::vector<std::map<unsigned, pss::Sched> > scheds_;
  std::vector<std::vector<pss::Job *> > raw_jobs_;
  std::vector<std::vector<pss::ShopJob *> > shop_jobs_;
  std::vector<pss::SchedStats> stats_;
  std::vector<std::ofstream> jls_files_;
  std::string sched_filename_prefix_;
  std::string sched_filename_suffix_;
  std::string jls_filename_prefix_;
  std::string jls_filename_suffix_;
  std::map<std::string, OrderFptr, CaseInsensitiveLess> seqpolicydict_;
  bool output_jls_files_;

  void FilenamePrefixSuffix(const char *filename, std::string &prefix,
                            std::string &suffix);

  struct MultisiteScheduleContext {
    int listId;
    std::vector<pss::Rsrc2Tintvl> &mach2tintvl;
    std::vector<pss::Rsrc2Tintvl> &opr2tintvl;
    std::vector<pss::ShopJob *> &allShopJobPointers;
    MultisiteScheduleContext(int _listId,
                             std::vector<pss::Rsrc2Tintvl> &_mach2tintvl,
                             std::vector<pss::Rsrc2Tintvl> &_opr2tintvl,
                             std::vector<pss::ShopJob *> &_allShopJobPointers) :
      listId(_listId), mach2tintvl(_mach2tintvl), opr2tintvl(_opr2tintvl),
      allShopJobPointers(_allShopJobPointers) {};
  };

  void ScheduleNonOutsourceableJobs(MultisiteScheduleContext &msc);

 public:
  MultisiteScheduler(const char *msShopFileName, const char *msJobFileName,
                     const char *msSchedFileName, const char *msJlsFileName);

  //greedily pick the best shop_ to outsource the first job within a group such that
  //all other, subsequent jobs in the same group are constrained to be outsourced to
  //the same "best" shop_, if outsourced
  //Side effect: 'outsourceDB' contains only exactly 1 option for each job in the group
  void ConstrainOutsourceDBforGroupJob(
    std::map<unsigned,
    std::map<unsigned, pss::ShopJob> > &outsourceDB,
    std::vector<pss::SchedStats> &stats_,
    const ShopJob *shopJob, const unsigned homeshop_id,
    const std::map<std::string, std::set<unsigned> > &group2shopIds,
    const std::map<std::string, std::set<unsigned> > &group2jobIds,
    const std::vector<Rsrc2Tintvl> &mach2tintvl,
    const std::vector<Rsrc2Tintvl> &opr2tintvl,
    const std::vector<ShopJob *> &allShopJobPointers);

  void Run(void);

  void PrintInfo(std::ostream &os);

};
} // namespace pss

#endif //PSS_MULTISITE_SCHEDULER_HPP_INCLUDED_