/* pss_multisite_scheduler.cpp
 *
 *  Created by Rong Zhou on 06/23/10.
 *  Copyright 2010-2016 Palo Alto Research Center Inc. All rights reserved.
 *
 *  implementation file for pss multi-site scheduler
 */
#include "pss_jobs_writer.hpp"
#include "pss_multisite_scheduler.hpp"

using namespace std;

namespace pss {

void MultisiteScheduler::FilenamePrefixSuffix(const char *filename,
    string &prefix, string &suffix) {
  prefix.clear();
  suffix.clear();
  string filename_str(filename);
  size_t found = filename_str.rfind('.');
  if(found != string::npos) {
    prefix.insert(prefix.begin(), filename_str.begin(),
                  filename_str.begin() + found);
    suffix.insert(suffix.begin(), filename_str.begin() + found,
                  filename_str.end());
  } else {
    prefix = filename_str;
  }
}

void MultisiteScheduler::ScheduleNonOutsourceableJobs(MultisiteScheduleContext &msc) {
  int listId = msc.listId;
  assert(listId >= 0);
  assert(!job_list_.IsOutsourceableList(listId));
  unsigned homeshop_id = job_list_.GetHomeShopId(listId);
  clock_t start = clock();
  DoSchedule(scheds_[homeshop_id], job_list_.lists_[listId].jobs_,
             shop_.GetShop(homeshop_id).GetInfo(),
             msc.mach2tintvl[homeshop_id], msc.opr2tintvl[homeshop_id],
             msc.allShopJobPointers);
  stats_[homeshop_id].cpu_sec += (clock() - start) / (double)CLOCKS_PER_SEC;
  for(vector<pss::ShopJob>::iterator it = job_list_.lists_[listId].jobs_.begin();
      it != job_list_.lists_[listId].jobs_.end();
      ++it) {
    shop_jobs_[homeshop_id].push_back(&(*it));
  }
  for(vector<pss::Job>::iterator it = job_list_.lists_[listId].model_.jobs.begin();
      it != job_list_.lists_[listId].model_.jobs.end();
      ++it) {
    raw_jobs_[homeshop_id].push_back(&(*it));
  }
}

MultisiteScheduler::MultisiteScheduler(const char *ms_shop_filename,
                                       const char *ms_job_filename,
                                       const char *ms_sched_filename,
                                       const char *ms_jls_filename) :
  shop_(ms_shop_filename), job_list_(ms_job_filename, shop_),
  scheds_(shop_.NumOfShops()),
  raw_jobs_(shop_.NumOfShops()), shop_jobs_(shop_.NumOfShops()),
  stats_(shop_.NumOfShops() + 1), output_jls_files_(ms_jls_filename != NULL) {
  FilenamePrefixSuffix(ms_sched_filename, sched_filename_prefix_,
                       sched_filename_suffix_);
  FilenamePrefixSuffix(ms_jls_filename, jls_filename_prefix_, jls_filename_suffix_);
  InitSeqPolicyDict(seqpolicydict_);
  for(vector<pss::SchedStats>::iterator itr = stats_.begin();
      itr != stats_.end();
      ++itr) {
    itr->cpu_sec = 0.0;
    itr->outsourced_jobs = 0;
    itr->outsourced_jobs_by_shop.resize(shop_.NumOfShops());
    itr->external_jobs = 0;
    itr->external_jobs_by_shop.resize(shop_.NumOfShops());
  }
}

//greedily pick the best shop_ to outsource the first job within a group such that
//all other, subsequent jobs in the same group are constrained to be outsourced to
//the same "best" shop_, if outsourced
//Side effect: 'outsource_db_' contains only exactly 1 option for each job in the group
void MultisiteScheduler::ConstrainOutsourceDBforGroupJob(map<unsigned,
    map<unsigned, pss::ShopJob> > &outsource_db_,
    vector<pss::SchedStats> &stats_,
    const ShopJob *shopJob, const unsigned homeshop_id,
    const map<string, set<unsigned> > &group2shop_ids_,
    const map<string, set<unsigned> > &group2job_ids_,
    const vector<Rsrc2Tintvl> &mach2tintvl,
    const vector<Rsrc2Tintvl> &opr2tintvl,
    const vector<ShopJob *> &allShopJobPointers) {
  vector<Rsrc2Tintvl> m2tvl, o2tvl;
  map<string, set<unsigned> >::const_iterator g2sItr, g2jItr;
  set<unsigned>::const_iterator shopItr, jobItr;
  time_t miniMaxCompletionTime = numeric_limits<time_t>::max();
  unsigned bestShopId = numeric_limits<unsigned>::max();
  clock_t start;

  assert(!shopJob->group.empty());
  g2sItr = group2shop_ids_.find(shopJob->group);
  assert(g2sItr != group2shop_ids_.end());
  g2jItr = group2job_ids_.find(shopJob->group);
  assert(g2jItr != group2job_ids_.end());
  for(shopItr = g2sItr->second.begin();
      shopItr != g2sItr->second.end();
      ++shopItr) {
    unsigned shop_id;
    Sched sched;
    time_t maxCompletionTime;

    m2tvl = mach2tintvl;
    o2tvl = opr2tintvl;
    maxCompletionTime = numeric_limits<time_t>::min();
    shop_id = *shopItr;
    for(jobItr = g2jItr->second.begin();
        jobItr != g2jItr->second.end();
        ++jobItr) {
      map<unsigned, map<unsigned, ShopJob> >::iterator oj;
      oj = outsource_db_.find(*jobItr);
      assert(oj != outsource_db_.end());
      map<unsigned, ShopJob>::iterator s2jItr;
      start = clock();
      s2jItr = oj->second.find(shop_id);
      assert(s2jItr != oj->second.end());
      ScheduleJob(sched, &s2jItr->second, 0 /* priority */,
                  m2tvl[shop_id], o2tvl[shop_id],
                  shop_.GetShop(shop_id).GetInfo(), allShopJobPointers);
      CommitSchedule(sched, m2tvl[shop_id], o2tvl[shop_id]);
      stats_[shop_id].cpu_sec += (clock() - start) / (double)CLOCKS_PER_SEC;
      s2jItr->second.completed += shop_.GetDelay(shop_id, homeshop_id);
      if(s2jItr->second.completed > maxCompletionTime) {
        maxCompletionTime = s2jItr->second.completed;
      }
    }
    if(miniMaxCompletionTime > maxCompletionTime) {
      miniMaxCompletionTime = maxCompletionTime;
      bestShopId = shop_id;
    }
  }
  assert(bestShopId != numeric_limits<unsigned>::max());
  for(jobItr = g2jItr->second.begin();
      jobItr != g2jItr->second.end();
      ++jobItr) {
    map<unsigned, map<unsigned, ShopJob> >::iterator oj;
    oj = outsource_db_.find(*jobItr);
    assert(oj != outsource_db_.end());
    map<unsigned, ShopJob>::iterator s2jItr;
    for(s2jItr = oj->second.begin(); s2jItr != oj->second.end();) {
      if(s2jItr->first != bestShopId)
        oj->second.erase(s2jItr++);
      else
        ++s2jItr;
    }
  }
}

void MultisiteScheduler::Run(void) {
  clock_t startTime = clock();
  unsigned numOfLists = job_list_.NumOfLists();
  unsigned num_of_shops = shop_.NumOfShops();
  //list of shops that can process all jobs in a group
  map<string, set<unsigned> > &group2shop_ids_ = job_list_.group2shop_ids_;
  //list of job ids belong to the same group
  map<string, set<unsigned> > &group2job_ids_ = job_list_.group2job_ids_;
  vector<Rsrc2Tintvl> mach2tintvl(num_of_shops);
  vector<Rsrc2Tintvl> opr2tintvl(num_of_shops);
  vector<ShopJob *> allShopJobPointers;
  job_list_.ShopJobPointers(allShopJobPointers);
  MultisiteScheduleContext msc(-1, mach2tintvl, opr2tintvl, allShopJobPointers);
#ifdef PSS_MULTI_THREADING
  vector<boost::thread *> schedThreads(numOfLists);
  vector<MultisiteScheduleContext> mscThreads(numOfLists, msc);
#endif
  for(unsigned listId = 0; listId < numOfLists; ++listId) {
    if(!job_list_.IsOutsourceableList(listId)) {
      /*
      unsigned homeshop_id = job_list_.GetHomeShopId(listId);
      clock_t start = clock();
      Schedule(scheds_[homeshop_id], job_list_.lists_[listId].jobs,
      shop_.GetShop(homeshop_id).GetInfo(),
      mach2tintvl[homeshop_id], opr2tintvl[homeshop_id], allShopJobPointers);
      stats_[homeshop_id].cpu_sec = (clock() - start) / (double) CLOCKS_PER_SEC;
      for (vector<shop_job_t>::iterator it = job_list_.lists_[listId].jobs.begin();
      it != job_list_.lists_[listId].jobs.end();
      ++it) {
      shop_jobs_[homeshop_id].push_back(&(*it));
      }
      for (vector<job_t>::iterator it = job_list_.lists_[listId].list.jobs.begin();
      it != job_list_.lists_[listId].list.jobs.end();
      ++it) {
      raw_jobs_[homeshop_id].push_back(&(*it));
      } */
      msc.listId = listId;
#ifdef PSS_MULTI_THREADING
      mscThreads[listId].listId = listId;
      schedThreads[listId] =
        new boost::thread(boost::bind(&MultisiteScheduler::ScheduleNonOutsourceableJobs,
                                      this, mscThreads[listId]));
#else
      ScheduleNonOutsourceableJobs(msc);
#endif //PSS_MULTI_THREADING
    }
  }

#ifdef PSS_MULTI_THREADING
  for(unsigned listId = 0; listId < numOfLists; ++listId) {
    if(!job_list_.IsOutsourceableList(listId))
      schedThreads[listId]->join();
  }

  for(unsigned listId = 0; listId < numOfLists; ++listId) {
    if(!job_list_.IsOutsourceableList(listId))
      delete schedThreads[listId];
  }
#endif //PSS_MULTI_THREADING

  for(unsigned listId = 0; listId < numOfLists; ++listId) {
    if(job_list_.IsOutsourceableList(listId)) {
      unsigned homeshop_id = job_list_.GetHomeShopId(listId);
      const ShopInfo &homeshopInfo = shop_.GetShop(homeshop_id).GetInfo();
      vector<ShopJob> &jobs(job_list_.lists_[listId].jobs_);
      vector<Job> &list(job_list_.lists_[listId].model_.jobs);
      vector<Job>::iterator rj;
      //vector<shop_job_t>::iterator sj;
      unsigned firstShopJobIntId = jobs[0].intid;
      //quick and dirty check of whether the "intid" field
      //is consecutively numbered
      assert(jobs[jobs.size() - 1].intid == firstShopJobIntId + jobs.size() - 1);
      vector<ShopJob *> shop_job_ptrs;
      ShopJobPointers(shop_job_ptrs, jobs);
      sort(shop_job_ptrs.begin(), shop_job_ptrs.end(),
           seqpolicydict_[homeshopInfo.config.sequencepolicy]);
      for(vector<ShopJob *>::iterator jp = shop_job_ptrs.begin();
          jp != shop_job_ptrs.end();
          ++jp) {
        //for (rj = list.begin(), sj = jobs.begin();
        //rj != list.end() && sj != jobs.end(); ++rj, ++sj) {
        ShopJob *sj = *jp;
        unsigned job_int_id = sj->intid;
        Sched sched, bestSched;
        unsigned bestShopId = homeshop_id;
        clock_t start = clock();
        ScheduleJob(bestSched, &*sj, (unsigned)scheds_[homeshop_id].size() + 1,
                    mach2tintvl[homeshop_id], opr2tintvl[homeshop_id],
                    homeshopInfo, allShopJobPointers);
        stats_[homeshop_id].cpu_sec += (clock() - start) / (double)CLOCKS_PER_SEC;
        time_t minCompletionTime = sj->completed;
        //job_t *rawJob = &(*rj);
        Job *rawJob = &list[sj->intid - firstShopJobIntId];
        ShopJob *bestShopJob = &(*sj);
        map<unsigned, map<unsigned, ShopJob> >::iterator oj;
        oj = job_list_.outsource_db_.find(job_int_id);
        if(oj != job_list_.outsource_db_.end()) {
          assert(oj->second.size() > 0);
          //> 1 shop_ candidate for job group
          if(!sj->group.empty() && oj->second.size() > 1) {
            ConstrainOutsourceDBforGroupJob(job_list_.outsource_db_, stats_,
                                            sj, homeshop_id,
                                            group2shop_ids_, group2job_ids_,
                                            mach2tintvl, opr2tintvl,
                                            allShopJobPointers);
            assert(oj == job_list_.outsource_db_.find(job_int_id));
            assert(oj->second.size() == 1);
          }
          map<unsigned, ShopJob>::iterator s2j;
          for(s2j = oj->second.begin(); s2j != oj->second.end(); ++s2j) {
            unsigned shop_id = s2j->first;
            start = clock();
            ScheduleJob(sched, &s2j->second, (unsigned)scheds_[shop_id].size() + 1,
                        mach2tintvl[shop_id], opr2tintvl[shop_id],
                        shop_.GetShop(shop_id).GetInfo(), allShopJobPointers);
            stats_[shop_id].cpu_sec += (clock() - start) / (double)CLOCKS_PER_SEC;
            s2j->second.completed += shop_.GetDelay(shop_id, homeshop_id);
            if(s2j->second.completed < minCompletionTime) {
              bestSched = sched;
              minCompletionTime = s2j->second.completed;
              bestShopId = shop_id;
              bestShopJob = &s2j->second;
            }
          }
        }
        CommitSchedule(bestSched, mach2tintvl[bestShopId], opr2tintvl[bestShopId]);
        scheds_[bestShopId][bestShopJob->intid] = bestSched;
        raw_jobs_[bestShopId].push_back(rawJob);
        shop_jobs_[bestShopId].push_back(bestShopJob);
        if(bestShopId != homeshop_id) {
          ++stats_[homeshop_id].outsourced_jobs;
          ++stats_[homeshop_id].outsourced_jobs_by_shop[bestShopId];
          ++stats_[bestShopId].external_jobs;
          ++stats_[bestShopId].external_jobs_by_shop[homeshop_id];
        }
        assert(scheds_[bestShopId].size() == bestShopJob->priority);
      }
    }
  }
  stats_[num_of_shops].cpu_sec = (clock() - startTime) / (double)CLOCKS_PER_SEC;
  for(unsigned shop_id = 0; shop_id < num_of_shops; ++shop_id) {
    const ShopInfo &shopInfo = shop_.GetShop(shop_id).GetInfo();
    ofstream sched_file;
    string sched_filename(sched_filename_prefix_ + '_' +
                          shop_.GetShopName(shop_id) + sched_filename_suffix_);
    sched_file.open(sched_filename.c_str());
    PrintFuncSched(sched_file, scheds_[shop_id], shop_jobs_[shop_id],
                   shopInfo.seq2func, ';');
    GetSchedStats(stats_[shop_id], shop_jobs_[shop_id], mach2tintvl[shop_id]);
    BaseInfo baseinfo;
    if(output_jls_files_) {
      ofstream jls_file;
      string jls_filename(jls_filename_prefix_ + '_' +
                          shop_.GetShopName(shop_id) + jls_filename_suffix_);
      jls_file.open(jls_filename.c_str());
      WriteScheduleEvents(raw_jobs_[shop_id], scheds_[shop_id], shop_jobs_[shop_id],
                          shopInfo, mach2tintvl[shop_id], allShopJobPointers);
      baseinfo.name = jls_filename_prefix_ + '_' + shop_.GetShopName(shop_id);
      WriteJobList(jls_file, baseinfo, raw_jobs_[shop_id]);
      jls_file.close();
    }
  }
}

void MultisiteScheduler::PrintInfo(ostream &os) {
  unsigned num_of_shops = shop_.NumOfShops();
  for(unsigned shop_id = 0; shop_id < num_of_shops; ++shop_id) {
    os << "Shop: " << shop_.GetShopName(shop_id) << endl;
    PrintShopConfig(os, shop_.GetShop(shop_id).GetInfo());
    PrintSchedStats(os, stats_[shop_id], shop_id, true);
    os << endl;
  }
  os << "Total CPU seconds = " << stats_[num_of_shops].cpu_sec << endl;
#ifdef PSS_MULTI_THREADING
  os << "Multi-threading = ON" << endl;
#endif
}

} // namespace pss
