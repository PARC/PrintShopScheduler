/*  pss_shop_job.cpp
 *
 *  Created by Rong Zhou on 05/01/09.
 *  Copyright 2009 Palo Alto Research Center Inc. All rights reserved.
 *
 *  implementation file for transforming raw jobs to shop jobs expressed
 *  in terms of the function sequences supported by a particular shop
 */
#include <algorithm>
#include "pss_shop_job.hpp"
#include "pss_exception.hpp"

#define JOB_GROUP_ATTRIBUTE_KEY "JobGroup"

using namespace std;

namespace pss {

//typedef vector<Step>::size_type StepId;
typedef unsigned StepId;

bool MatchInOutRsrcs(vector<string> outputs,
                     vector<string> inputs) {
  vector<string>::const_iterator rin, rout;

  for(rin = inputs.begin(); rin != inputs.end(); ++rin) {
    for(rout = outputs.begin(); rout != outputs.end() && *rin != *rout; ++rout);
    if(rout == outputs.end())
      return false;
  }
  return true;
}

void GetFuncSeq(Func &funcseq, vector<Step> const &steps, StepId cur,
                set<StepId> &open, set<StepId> &closed,
                set<string> const &sameseqfuncs,
                set<string> seqset, ShopInfo const &shop_info) {
  vector<string>::const_iterator rin, rout;
  Step const &curstep = steps[cur];
  set<string> sharedseqs, sharedfuncs;
  set<string>::const_iterator s;
  vector<StepId>::size_type size;

  for(StepId i = cur + 1; i < steps.size(); ++i) {
    if(closed.find(i) != closed.end())
      continue;
    set<string>::const_iterator f
      = sameseqfuncs.find(steps[i].baseinfo.name);
    if(f != sameseqfuncs.end()) {
      Step const &nextstep = steps[i];
      if(MatchInOutRsrcs(curstep.outputrsrcs, nextstep.inputrsrcs)) {
        set_intersection(shop_info.func2seq.find(*f)->second.begin(),
                         shop_info.func2seq.find(*f)->second.end(),
                         seqset.begin(), seqset.end(),
                         inserter(sharedseqs, sharedseqs.begin()));
        if(sharedseqs.empty())
          return;
        open.insert(i);
        size = open.size();
        for(s = sharedseqs.begin(); s != sharedseqs.end(); ++s) {
          if(size == shop_info.seq2func.find(*s)->second.size()) {
            funcseq.name = *s;
            funcseq.inrsrcs = steps[*open.begin()].inputrsrcs;
            funcseq.outrsrcs = steps[*open.rbegin()].outputrsrcs;
            break; //only one such funcseq should exist
          }
        }
        set_intersection(sameseqfuncs.begin(), sameseqfuncs.end(),
                         shop_info.func2sameseqfunc.find(*f)->second.begin(),
                         shop_info.func2sameseqfunc.find(*f)->second.end(),
                         inserter(sharedfuncs, sharedfuncs.begin()));
        if(sharedfuncs.empty())
          return;
        else
          GetFuncSeq(funcseq, steps, i, open, closed, sharedfuncs, sharedseqs,
                     shop_info);
      }
    }
  }
}

//currently unused code, reserved for future generality
void FilterDistinguishedAttributes(const One2One &inputAttrs,
                                   One2One &outputAttrs, One2One &filteredAttrs,
                                   const set<string> &distinguished_attribute_keys) {
  One2One::const_iterator itr;

  for(itr = inputAttrs.begin(); itr != inputAttrs.end(); ++itr) {
    //non distinguished key
    if(distinguished_attribute_keys.find(itr->first) ==
        distinguished_attribute_keys.end()) {
      outputAttrs.insert(*itr);
    } else { //distinguished key
      filteredAttrs.insert(*itr);
    }
  }
}

void FilterJobGroupAttribute(const One2One &inputAttrs, One2One &outputAttrs,
                             string &job_group) {
  One2One::const_iterator itr;

  job_group.clear();
  for(itr = inputAttrs.begin(); itr != inputAttrs.end(); ++itr) {
    if(itr->first != JOB_GROUP_ATTRIBUTE_KEY) {
      outputAttrs.insert(*itr);
    } else { //distinguished key
      job_group = itr->second;
    }
  }
}

void GetShopJob(ShopJob &shop_job, const Job &job, const ShopInfo &shop_info,
                const unsigned job_int_id) {
  set<StepId> closed;
  set<StepId> open;
  Func funcseq, funcstep;
  set<string>::const_iterator q;
  string job_group;

  shop_job.intid = job_int_id;
  shop_job.id = job.jobinfo.jobid;
  TranslateHtmlStr(shop_job.id);
  shop_job.partid = job.jobinfo.jobpartid;
  TranslateHtmlStr(shop_job.partid);
  shop_job.numbatches = 1;
  shop_job.arrival = GetTime(job.arrival.day, job.arrival.time);
  shop_job.due = GetTime(job.due.day, job.due.time);
  shop_job.released = GetTime(job.released.day, job.released.time);
  shop_job.completed = GetTime(job.completed.day, job.completed.time);
  shop_job.resources = job.resources;
  vector<Resource>::const_iterator r;
  double proctime = 0.0;
  for(r = job.resources.begin(); r != job.resources.end(); ++r) {
    if((*r).quantity.value == 0)
      throw RuntimeException("Resource '" + (*r).baseinfo.name +
                             "' has zero quantity in job " + job.baseinfo.name);
    map<string, double>::const_iterator r2sp_itr;
    r2sp_itr = shop_info.rsrc2speed.find((*r).quantity.unit);
    if(r2sp_itr == shop_info.rsrc2speed.end())
      throw RuntimeException("Resource '" + (*r).baseinfo.name +
                             "' has invalid unit '" + (*r).quantity.unit
                             + "' in job " + job.baseinfo.name);
    proctime += (*r).quantity.value / r2sp_itr->second;
  }
  shop_job.proctime = static_cast<time_t>(proctime);
  shop_job.funcseqs.clear();
  shop_job.funcsteps.clear();
  closed.clear();
  // for debugging:
  //if (job.baseinfo.name == "1" ) {
  //for (vector<Step>::const_iterator x = job.steps.begin(); x != job.steps.end(); ++x) {
  //  cerr << (*x).stepinfo.function << endl;
  //  cerr << "Inputs: ";
  //  copy((*x).inputrsrcs.begin(), (*x).inputrsrcs.end(), ostream_iterator<string>(cerr, " "));
  //  cerr << endl << "Outputs: ";
  //  copy((*x).outputrsrcs.begin(), (*x).outputrsrcs.end(), ostream_iterator<string>(cerr, " "));
  //  cerr << endl;
  //}
  //}
  for(StepId s = 0; s < job.steps.size(); ++s) {
    const string *func = &job.steps[s].stepinfo.function;
    funcstep.name = *func;
    //funcstep.stepid = s; no need for funcsteps
    funcstep.inrsrcs = job.steps[s].inputrsrcs;
    funcstep.outrsrcs = job.steps[s].outputrsrcs;
    shop_job.funcsteps.push_back(funcstep);
    if(closed.find(s) != closed.end())
      continue; //skip steps already merged into some funcseq
    funcseq.stepids.clear();
    if(shop_info.func2sameseqfunc.find(*func) == shop_info.func2sameseqfunc.end()) {
      throw RuntimeException("Job: " + job.baseinfo.name +
                             "\n\tFunction step not found: " + *func);
    }
    if(shop_info.func2sameseqfunc.find(*func)->second.empty()) {
      funcseq.name = *shop_info.func2seq.find(*func)->second.begin();
      funcseq.stepids.push_back(s); //needed for jobs with multiple identical funcseqs or steps
      //funcseq.attributes = job.steps[s].attributes;
      FilterJobGroupAttribute(job.steps[s].attributes, funcseq.attributes, job_group);
      if(!job_group.empty() && !shop_job.group.empty() && job_group != shop_job.group)
        throw RuntimeException("Inconsistent job group definition in job: '" +
                               job.baseinfo.name + "'\npreviuous group '" + shop_job.group +
                               "' != current group '" + job_group + "'");
      shop_job.group = job_group;
      funcseq.inrsrcs = job.steps[s].inputrsrcs;
      funcseq.outrsrcs = job.steps[s].outputrsrcs;
      closed.insert(s);
    } else {
      funcseq.name = "null";
      for(q = shop_info.func2seq.find(*func)->second.begin();
          q != shop_info.func2seq.find(*func)->second.end();
          ++q) {
        if(shop_info.seq2func.find(*q)->second.size() == 1) {
          //for debugging:
          //cerr << "func := " << *func << endl;
          //copy(shop_info.func2sameseqfunc.find(*func)->second.begin(), shop_info.func2sameseqfunc.find(*func)->second.end(),
          //ostream_iterator<string>(cerr, " "));

          funcseq.name = *q;
          //funcseq.stepids.push_back(s); //s will be inserted below
          funcseq.inrsrcs = job.steps[s].inputrsrcs;
          funcseq.outrsrcs = job.steps[s].outputrsrcs;
          break;//assume no two seqs have samet set of funcs
        }
      }
      open.clear();
      open.insert(s);
      GetFuncSeq(funcseq, job.steps, s, open, closed,
                 shop_info.func2sameseqfunc.find(*func)->second,
                 shop_info.func2seq.find(*func)->second, shop_info);
      funcseq.stepids.insert(funcseq.stepids.begin(), open.begin(), open.end());
      funcseq.attributes = job.steps[s].attributes; //assumption: same-sequence steps share the same attributes
      closed.insert(open.begin(), open.end());
      //for debugging:
      //cerr << ": " << funcseq.name << endl;
      //cerr << "Inputs: ";
      //copy(funcseq.inrsrcs.begin(), funcseq.inrsrcs.end(), ostream_iterator<string>(cerr, " "));
      //cerr << endl << "Outputs: ";
      //copy(funcseq.outrsrcs.begin(), funcseq.outrsrcs.end(), ostream_iterator<string>(cerr, " "));
      //cerr << endl;
    }
    shop_job.funcseqs.push_back(funcseq);
  }
}

void GetShopJobs(vector<ShopJob> &shop_jobs, const vector<Job> &jobs,
                 const ShopInfo &shop_info, const unsigned min_job_int_id) {
  vector<ShopJob>::iterator sj;
  vector<Job>::const_iterator j;
  shop_jobs.resize(jobs.size());
  unsigned job_int_id; //job internal id

  for(sj = shop_jobs.begin(), j = jobs.begin(), job_int_id = min_job_int_id;
      j != jobs.end();
      ++sj, ++j, ++job_int_id) {
    GetShopJob(*sj, *j, shop_info, job_int_id);
  }
}
} // namespace pss
