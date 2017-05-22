/*  pss_sched_utils.hpp
 *
 *  Created by Rong Zhou on 05/01/09.
 *  Copyright 2009 Palo Alto Research Center Inc. All rights reserved.
 *
 *  header file for utility data types and functions of the scheduler
 */

#ifndef PSS_SCHED_UTILS_HPP_INCLUDED_
#define PSS_SCHED_UTILS_HPP_INCLUDED_

#include "pss_shop_job.hpp"

namespace pss {

struct Fstep {
  std::string funcseq;
  std::string cell;
  std::string station;
  std::string opr;
  unsigned seqid;  //index to corresponding funcseqs[] in Job
};

typedef std::vector<Fstep> Route;

typedef unsigned StepId;

typedef std::map<std::string, TintvlSet> Rsrc2Tintvl;

struct SchedStep {
  Fstep step;
  std::vector<Tintvl> mach_tintvls;
  std::vector<Tintvl> opr_tintvls;
  std::vector<int> quantities;
};

typedef std::vector<SchedStep> Sched;

struct SchedInfo {
  Sched best;
  Sched sched;
  StepId cur;
  std::map<std::string, time_t> rsrc2tm;
  std::map<std::string, unsigned> *rsrc2quantity;
  Rsrc2Tintvl jobmach2tintvl;
  Rsrc2Tintvl jobopr2tintvl;
};

struct SchedStats { // schedule statistics
  int jobs; // total number of jobs
  int outsourced_jobs; //number of jobs processed by another shop
  //number of outsourced jobs broken down into shops
  std::vector<unsigned> outsourced_jobs_by_shop;
  int external_jobs; //number of jobs processed for another shop
  //number of external jobs broken down into shops
  std::vector<unsigned> external_jobs_by_shop;
  int latejobs; // number of late jobs
  double latepercent; // percentage of late jobs
  double avgdelay; // average delay
  double stdevdelay; // standard deviations of lateness
  double avgdelay_lateonly; // average delay for late jobs only
  double maxdelay; // maximum delay
  double avg_tat; // average turn around time (TAT)
  double avg_proc_time; // Average processing time
  time_t start; // start time of the first job
  time_t end; // end time of the last job
  time_t makespan; // = end - start
  double cpu_sec; // cpu seconds spent by the scheduler
};

struct CaseInsensitiveLess : std::binary_function<std::string, std::string, bool> {
  struct nocase_compare :
    public std::binary_function<unsigned char, unsigned char, bool> {
    bool operator()(const unsigned char &c1, const unsigned char &c2) const {
      return tolower(c1) < tolower(c2);
    }
  };
  bool operator()(const std::string &s1, const std::string &s2) const {
    return std::lexicographical_compare(s1.begin(), s1.end(),
                                        s2.begin(), s2.end(),
                                        nocase_compare());
  }
};

typedef bool (*OrderFptr)(const ShopJob *, const ShopJob *);

int FindMinhopRoutes(std::vector<Route> &routes,
                     std::vector<Func> const &funcseqs,
                     One2Many const &seq2cell);

void ScheduleJob(Sched &sched, ShopJob *shopJob, unsigned priority,
                 Rsrc2Tintvl &mach2tintvl, Rsrc2Tintvl &opr2tintvl,
                 const ShopInfo &shopInfo,
                 const std::vector<ShopJob *> &all_job_ptrs);

void CommitSchedule(const Sched &sched, Rsrc2Tintvl &mach2tintvl,
                    Rsrc2Tintvl &opr2tintvl);

void DoSchedule(std::map<unsigned, Sched> &scheds,
                std::vector<ShopJob> &shop_jobs,
                const ShopInfo &shopInfo, Rsrc2Tintvl &mach2tintvl,
                Rsrc2Tintvl &opr2tintvl,
                const std::vector<ShopJob *> &all_job_ptrs);

void PrintSched(std::ostream &os, std::map<unsigned, Sched> &scheds,
                std::vector<ShopJob *> &shop_jobs, const char separator);

void GetSchedStats(SchedStats &schedStats,
                   const std::vector<ShopJob *> &shop_jobs,
                   const Rsrc2Tintvl &mach2tintvl);

void PrintSchedStats(std::ostream &os, const SchedStats &schedStats,
                     unsigned shop_id, bool multiSite);

void PrintFuncSched(std::ostream &os, std::map<unsigned, Sched> &scheds,
                    std::vector<ShopJob *> &shop_jobs,
                    const One2Many &seq2func, const char separator);

void WriteScheduleEvents(std::vector<Job *> &rawjobs,
                         std::map<unsigned, Sched> &scheds,
                         std::vector<ShopJob *> &shop_jobs,
                         const ShopInfo &shopInfo,
                         const Rsrc2Tintvl &mach2tintvl,
                         const std::vector<ShopJob *> &all_job_ptrs);

void ShopJobPointers(std::vector<ShopJob *> &shop_job_pointers,
                     std::vector<ShopJob> &shop_jobs);

void InitSeqPolicyDict(std::map<std::string, OrderFptr, CaseInsensitiveLess> &dict);

} // namespace pss

#endif // PSS_SCHED_UTILS_HPP_INCLUDED_
