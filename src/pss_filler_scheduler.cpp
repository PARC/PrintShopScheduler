/* pss_filler_scheduler.cpp
 *
 *  Created by Rong Zhou on 11/18/11.
 *  Copyright 2011-16 Palo Alto Research Center Inc. All rights reserved.
 *
 * implementation file for pss single-site filler scheduler
 */
#include <limits>
#include "pss_filler_scheduler.hpp"
#include "pss_jobs_writer.hpp"

using namespace std;

namespace pss {

FillerScheduler::FillerScheduler(const char *shop_filename,
                                 const char *job_filename,
                                 const char *sched_filename,
                                 const char *jls_filename,
                                 const char *filler_job_filename,
                                 const time_t min_delta_t_,
                                 const time_t max_delta_t_) :
  Scheduler(shop_filename, job_filename, sched_filename, jls_filename),
  filler_job_pattern_(filler_job_filename, shop_.GetInfo(), 0),
  num_jobs_filled_(0), min_delta_t_(min_delta_t_), max_delta_t_(max_delta_t_) {
  time_t min_arrival_t = numeric_limits<time_t>::max();
  vector<ShopJob>::iterator it;
  for(it = filler_job_pattern_.jobs_.begin();
      it != filler_job_pattern_.jobs_.end();
      ++it) {
    min_arrival_t = min(min_arrival_t, it->arrival);
  }
  for(it = filler_job_pattern_.jobs_.begin();
      it != filler_job_pattern_.jobs_.end();
      ++it) {
    it->arrival -= min_arrival_t;
    it->due -= min_arrival_t;
  }
}

void FillerScheduler::ShiftDate(pss::Date &date, const time_t shift_time) {
  time_t time = GetTime(date.day, date.time);
  time += shift_time;
  GetDate(date, time);
}

void FillerScheduler::InsertFillerJobs(JobList &job_list,
                                       vector<ShopJob *> &shop_job_pointers,
                                       const JobList &filler_job_pattern_,
                                       const time_t shift_t, const int shift_id) {
  vector<Job>::const_iterator j;
  for(j = filler_job_pattern_.model_.jobs.begin();
      j != filler_job_pattern_.model_.jobs.end();
      ++j) {
    Job newJob(*j);
    ShiftDate(newJob.arrival, shift_t);
    ShiftDate(newJob.due, shift_t);
    job_list.model_.jobs.push_back(newJob);
  }
  vector<ShopJob>::const_iterator sj;
  for(sj = filler_job_pattern_.jobs_.begin();
      sj != filler_job_pattern_.jobs_.end();
      ++sj) {
    ShopJob newShopJob(*sj);
    newShopJob.arrival += shift_t;
    newShopJob.due += shift_t;
    newShopJob.intid += shift_id;
    //may trigger realloc that invalidates the pointers
    job_list.jobs_.push_back(newShopJob);
    //shop_job_pointers.push_back(&job_list.jobs.back()); cannot do this, see above
    //do this from scratch in case realloc occurred
    job_list.ShopJobPointers(shop_job_pointers);
  }
}

void FillerScheduler::RemoveFillerJobs(JobList &job_list,
                                       vector<ShopJob *> &shop_job_pointers,
                                       const unsigned num_filler_jobs) {
  for(unsigned n = 0; n < num_filler_jobs; ++n) {
    job_list.model_.jobs.pop_back();
    job_list.jobs_.pop_back();
    shop_job_pointers.pop_back();
  }
}

void FillerScheduler::Run(void) {
  vector<pss::ShopJob> &jobs(job_list_.jobs_);
  JobListModel &list(job_list_.model_);
  const ShopInfo &shopInfo(shop_.GetInfo());
  Rsrc2Tintvl mach2tintvl, opr2tintvl;
  vector<ShopJob *> shop_job_pointers;
  job_list_.ShopJobPointers(shop_job_pointers);
  clock_t start = clock();
  DoSchedule(scheds_, jobs, shopInfo, mach2tintvl, opr2tintvl, shop_job_pointers);
  GetSchedStats(stats_, shop_job_pointers, mach2tintvl);
  time_t fillerStart;
  unsigned num_filler_jobs = filler_job_pattern_.size();
  int num_filling_attempts = 0;
  //the "filling" period is determined by stats.start and stats.end
  //user can define dummy start or end jobs to change this period
  //consider adding command line options in the future to allow more direct control
  time_t deltaT = min_delta_t_;
  double prevFillerJobStartTime = 0.0, sumDeltaT = 0.0;
  for(fillerStart = stats_.start; fillerStart < stats_.end; fillerStart += deltaT) {
    int priority = (int)shop_job_pointers.size();
    InsertFillerJobs(job_list_, shop_job_pointers, filler_job_pattern_,
                     fillerStart, priority);
    map<unsigned, Sched> fillerScheds;
    Rsrc2Tintvl mach2tintvlTmp(mach2tintvl), opr2tintvlTmp(opr2tintvl);
    bool success = true;
    //for (vector<ShopJob*>::iterator jItr = firstFillerItr; jItr != shop_job_pointers.end(); ++jItr) {
    for(; priority < (int)shop_job_pointers.size(); ++priority) {
      //cout << shop_job_pointers[priority]->intid << endl;
      ScheduleJob(fillerScheds[shop_job_pointers[priority]->intid],
                  shop_job_pointers[priority], priority, mach2tintvlTmp,
                  opr2tintvlTmp, shopInfo, shop_job_pointers);
      if(shop_job_pointers[priority]->completed > shop_job_pointers[priority]->due) {
        //job is delayed --> roll back
        RemoveFillerJobs(job_list_, shop_job_pointers, num_filler_jobs);
        success = false;
        if(deltaT < max_delta_t_) {
          deltaT = min(max_delta_t_, deltaT * 2);
        }
        break;
      }
      //preliminary commit
      CommitSchedule(fillerScheds[shop_job_pointers[priority]->intid],
                     mach2tintvlTmp, opr2tintvlTmp);
    }
    //final commit
    if(success) {
      mach2tintvl = mach2tintvlTmp;
      opr2tintvl = opr2tintvlTmp;
      scheds_.insert(fillerScheds.begin(), fillerScheds.end());
      ++num_jobs_filled_;
      if(num_jobs_filled_ > 1)
        sumDeltaT += (double)(fillerStart - prevFillerJobStartTime);
      prevFillerJobStartTime = (double)fillerStart;
      if(deltaT > min_delta_t_) {
        deltaT = max(min_delta_t_, deltaT / 2);
      }
    }
    ++num_filling_attempts;
  }
  stats_.cpu_sec = (clock() - start) / (double) CLOCKS_PER_SEC;
  PrintFuncSched(sched_file_, scheds_, shop_job_pointers, shopInfo.seq2func, ';');
  cout << "Number of jobs filled / attempted = " << num_jobs_filled_ << " / "
       << num_filling_attempts << endl;
  cout << "Minimum and maximum time gap bewteen two consecutive filler jobs = ["
       << min_delta_t_ << ", " << max_delta_t_ << "] seconds" << endl;
  cout << "Average time gap bewteen two consecutive filler jobs = "
       << sumDeltaT / (num_jobs_filled_ - 1) << endl;
  if(jls_file_.is_open()) {
    vector<Job *> row_job_pointers;
    job_list_.RawJobPointers(row_job_pointers);
    WriteScheduleEvents(row_job_pointers, scheds_, shop_job_pointers, shopInfo,
                        mach2tintvl, shop_job_pointers);
    jls_file_ << list;
    jls_file_.close();
  }
}

void FillerScheduler::PrintInfo(ostream &os) {
  PrintShopConfig(os, shop_.GetInfo());
  PrintSchedStats(os, stats_, 0, false);
}

} // namespace pss
