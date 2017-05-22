/*  pss_sched_utils.cpp
 *
 *  Created by Rong Zhou on 05/01/09.
 *  Copyright 2009 Palo Alto Research Center Inc. All rights reserved.
 *
 *  implementation file for the scheduling algorithms
 */
#include <limits>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include "pss_sched_utils.hpp"
#include "pss_exception.hpp"

using namespace std;

namespace pss {

typedef unsigned seq_id;

void FindRoutes(vector<Route>      &routes,
                vector<Func> const &funcseqs,
                int                 maxcellhops,
                Route              &cur,
                set<seq_id>        &closed,
                set<string>        &rsrcs,
                One2Many const     &seq2cell) {
  vector<string>::const_iterator rin, rout;
  set<string>::const_iterator c;
  string lastcell;
  Fstep nextstep;
  int newmaxhops;
  bool apply;

  if(cur.size() == funcseqs.size()) {
    routes.push_back(cur);
    return;
  }
  if(!cur.empty())
    lastcell = cur.back().cell;
  else
    lastcell = "";
  for(seq_id s = 0; s < funcseqs.size(); ++s) {
    //cerr << "funcseqs[" << s << "] = " << funcseqs[s].name << endl;
    if(closed.find(s) != closed.end())
      continue;
    apply = true;
    for(rin = funcseqs[s].inrsrcs.begin();
        rin != funcseqs[s].inrsrcs.end();
        ++rin) {
      //cerr << " " << *rin;
      if(rsrcs.find(*rin) == rsrcs.end()) {
        apply = false;
        //cerr << " " << *rin << " -> not applicable" << endl;
        //copy(rsrcs.begin(), rsrcs.end(),ostream_iterator<string>(cerr, " "));
        break;
      }
    }
    if(apply) {
      //cerr << " -> applicable" << endl;
      closed.insert(s);
      nextstep.funcseq = funcseqs[s].name;
      nextstep.seqid = s;
      One2Many::const_iterator s2cItr = seq2cell.find(nextstep.funcseq);
      if(s2cItr == seq2cell.end()) {
        throw RuntimeException("Function sequence not found: " + nextstep.funcseq);
      }
      for(c = s2cItr->second.begin(); c != s2cItr->second.end(); ++c) {
        if(*c == lastcell || lastcell == "")
          newmaxhops = maxcellhops;
        else
          newmaxhops = maxcellhops - 1;
        if(newmaxhops >= 0) {
          nextstep.cell = *c;
          //cerr << "cell = " << *c << endl;
          cur.push_back(nextstep);
          rsrcs.insert(funcseqs[s].outrsrcs.begin(), funcseqs[s].outrsrcs.end());
          FindRoutes(routes, funcseqs, newmaxhops, cur, closed, rsrcs, seq2cell);
          for(rout = funcseqs[s].outrsrcs.begin();
              rout != funcseqs[s].outrsrcs.end();
              ++rout)
            rsrcs.erase(*rout);
          cur.pop_back();
        }
      }
      closed.erase(s);
    }
  }
}

int FindMinhopRoutes(vector<Route> &routes, vector<Func> const &funcseqs,
                     One2Many const &seq2cell) {
  Route cur;
  unsigned int maxcellhops;
  set<seq_id> closed;
  set<string> rsrcs;

  for(maxcellhops = 0; maxcellhops < funcseqs.size(); ++maxcellhops) {
    cur.clear();
    closed.clear();
    rsrcs.clear();
    FindRoutes(routes, funcseqs, maxcellhops, cur, closed, rsrcs, seq2cell);
    if(!routes.empty()) {
      //for debugging:
      //for (vector<Route>::const_iterator r = routes.begin(); r != routes.end(); ++r)
      //{
      //  cerr <<  "Minhop Route: ";
      //  for (Route::const_iterator s = (*r).begin(); s != (*r).end(); ++s)
      //  {
      //    cerr << '[' << (*s).funcseq;
      //    cerr << '(' << funcseqs[(*s).seqid].inrsrcs.front() << ')';
      //    cerr << ", " << (*s).cell << "] ";
      //  }
      //  cerr << endl;
      //}
      return (int)maxcellhops;
    }
  }
  //cerr << "Warning: route is empty!" << endl;
  return -1;
}

void SchedStepQuantities(SchedStep &schedstep, double setup0, double setup1,
                         double speedval, int quantity) {
  vector<Tintvl>::const_iterator t;
  time_t totdur, stime, dur;
  int qty;

  schedstep.quantities.clear();
  totdur = 0;
  stime = static_cast<time_t>(ceil(setup0));
  for(t = schedstep.mach_tintvls.begin();
      t != schedstep.mach_tintvls.end();
      ++t) {
    dur = (*t).end - (*t).start + 1;
    if(dur > stime)
      qty = static_cast<int>(floor((dur - stime) * speedval + .5));
    else
      qty = 0;
    if(quantity < qty)
      qty = quantity;
    schedstep.quantities.push_back(qty);
    totdur += dur;
    quantity -= qty;
    stime = static_cast<time_t>(ceil(setup1));
  }
  //for debugging:
  if(quantity) {
    stime = static_cast<time_t>(ceil(setup0));
    cerr << "quantity = " << quantity << endl;
    cerr << "setup = " << stime << endl;
    cerr << "speed = " << speedval << endl;
    totdur = 0;
    for(t = schedstep.mach_tintvls.begin();
        t != schedstep.mach_tintvls.end();
        ++t) {
      dur = (*t).end - (*t).start + 1;
      if(dur > stime)
        qty = static_cast<int>(floor((dur - stime) * speedval + .5));
      else
        qty = 0;
      totdur += dur;
      cerr << "qty = " << qty << ", dur = " << dur << ", totdur = "
           << totdur << endl;
      stime = static_cast<time_t>(ceil(setup1));
    }
    assert(quantity == 0);
  }
  assert(quantity == 0);
}

void PrintSchedDebugInfo(Sched &sched) {
  Sched::const_iterator s;
  vector<Tintvl>::const_iterator t;
  vector<int>::const_iterator q;
  string time_str;

  for(s = sched.begin(); s != sched.end(); ++s) {
    cerr << "Funcseq = " << (*s).step.funcseq;
    cerr << ", cell = " << (*s).step.cell;
    cerr << ", station = " << (*s).step.station << endl;
    for(t = (*s).mach_tintvls.begin(), q = (*s).quantities.begin();
        t != (*s).mach_tintvls.end();
        ++t, ++q) {
      GetTimeStr(time_str, (*t).start);
      cerr << "\tStart = " << time_str;
      GetTimeStr(time_str, (*t).end);
      cerr << "\tEnd = " << time_str;
      cerr << "\tQty = " << *q << endl;
    }
  }
}

void TintvlSetUnion(TintvlSet &set1, TintvlSet &set2, TintvlSet &tintvl_union) {
  tintvl_union.clear();
  set_union(set1.begin(), set1.end(), set2.begin(), set2.end(),
            inserter(tintvl_union, tintvl_union.begin()), LtTintvl());
}

void InsertMachTintvl(Rsrc2Tintvl &rsrc2tintvl, const SchedStep &schedstep) {
  rsrc2tintvl[schedstep.step.station].insert(schedstep.mach_tintvls.begin(),
      schedstep.mach_tintvls.end());
}

void RemoveMachTintvl(Rsrc2Tintvl &rsrc2tintvl, SchedStep &schedstep) {
  TintvlSet &tintvls = rsrc2tintvl[schedstep.step.station];
  vector<Tintvl>::const_iterator t;

  for(t = schedstep.mach_tintvls.begin(); t != schedstep.mach_tintvls.end(); ++t)
    tintvls.erase(*t);
}

void InsertMachTintvl(Rsrc2Tintvl &rsrc2tintvl, const Sched &sched) {
  Sched::const_iterator s;

  for(s = sched.begin(); s != sched.end(); ++s)
    rsrc2tintvl[(*s).step.station].insert((*s).mach_tintvls.begin(),
                                          (*s).mach_tintvls.end());
}

void AddOprTintvl(Rsrc2Tintvl &rsrc2tintvl, const Sched &sched) {
  Sched::const_iterator s;

  for(s = sched.begin(); s != sched.end(); ++s) {
    TintvlSet &oprtintvl = rsrc2tintvl[(*s).step.opr];
    TintvlSetAdd(oprtintvl, (*s).opr_tintvls, oprtintvl);
  }
}

void CommitSchedule(const Sched &sched, Rsrc2Tintvl &mach2tintvl,
                    Rsrc2Tintvl &opr2tintvl) {
  InsertMachTintvl(mach2tintvl, sched);
  AddOprTintvl(opr2tintvl, sched);
}

void SimplifyOprTintvl(Sched &sched) {
  Sched::iterator s;

  for(s = sched.begin(); s != sched.end(); ++s) {
    TintvlSetSimplify((*s).opr_tintvls);
  }
}

int CompareTintvlEndTime(const vector<Tintvl> &tintvls1,
                         const vector<Tintvl> &tintvls2) {
  vector<Tintvl>::const_reverse_iterator t1, t2;

  for(t1 = tintvls1.rbegin(), t2 = tintvls2.rbegin();
      t1 != tintvls1.rend() && t2 != tintvls2.rend();
      ++t1, ++t2) {
    if((*t1).end < (*t2).end)
      return -1;
    else if((*t1).end > (*t2).end)
      return 1;
  }
  return 0;
}

int CompareSchedEndTime(const Sched &sched1, const Sched &sched2) {
  Sched::const_reverse_iterator s1, s2;
  int tintvl_compare;

  for(s1 = sched1.rbegin(), s2 = sched2.rbegin();
      s1 != sched1.rend() && s2 != sched2.rend();
      ++s1, ++s2) {
    tintvl_compare = CompareTintvlEndTime((*s1).mach_tintvls,
                                          (*s2).mach_tintvls);
    if(tintvl_compare != 0)
      return tintvl_compare;
  }
  return 0;
}

time_t AttributeSetupTime(const map<string, time_t> &sfuncAttr,
                          const One2One &prevAttr, const One2One &curAttr) {
  time_t setupTime = 0;
  for(map<string, time_t>::const_iterator it = sfuncAttr.begin();
      it != sfuncAttr.end();
      ++it) {
    One2One::const_iterator it_prev, it_cur;
    it_prev = prevAttr.find(it->first);
    if(it_prev == prevAttr.end()) {
      throw RuntimeException("Unable to find an attribute with name: " + it->first);
    }
    it_cur = curAttr.find(it->first);
    if(it_cur == curAttr.end()) {
      throw RuntimeException("Unable to find an attribute with name: " + it->first);
    }
    if(it_cur->second.compare(it_prev->second) != 0) {
      setupTime += it->second;
    }
  }
  return setupTime;
}

time_t AttributeSetupTimeSum(const map<string, time_t> &sfuncAttr) {
  time_t setupTime = 0;
  for(map<string, time_t>::const_iterator it = sfuncAttr.begin();
      it != sfuncAttr.end();
      ++it) {
    setupTime += it->second;
  }
  return setupTime;
}

void FindSched(SchedInfo &schedInfo, const ShopJob *job,
               Route const &route, Rsrc2Tintvl &mach2tintvl,
               Rsrc2Tintvl &opr2tintvl, ShopInfo const &shopInfo,
               const vector<ShopJob *> &all_job_ptrs) {
  if(schedInfo.cur == route.size()) {
    if(!schedInfo.best.empty()) {
      time_t best_end = schedInfo.best.back().mach_tintvls.back().end;
      time_t cur_end = schedInfo.sched.back().mach_tintvls.back().end;
      //if (best_end > cur_end || (best_end == cur_end && CompareSchedEndTime(best, sched) > 0))
      if(best_end > cur_end) {
        schedInfo.best = schedInfo.sched;
        //PrintSchedDebugInfo(schedInfo.best);
      }
    } else {
      //PrintSchedDebugInfo(schedInfo.best);
      schedInfo.best = schedInfo.sched;
    }
    return;
  }
  const Fstep &curstep = route[schedInfo.cur];
  const unsigned curSeqId = curstep.seqid;
  const Func &curfuncseq = job->funcseqs[curSeqId];
  const One2One &curAttr = curfuncseq.attributes;
  SfuncSet const &sfuncset =
    shopInfo.seq2mach.find(curstep.cell)->second.find(curstep.funcseq)->second;
  SfuncSet::const_iterator s;
  SchedStep schedstep;
  Tintvl tintvl;
  time_t rsrctm, start, est_start, nxt_start, stime0, stime1, ends_before;
#ifdef PSS_TRADE_QUALITY_FOR_SPEED
  time_t min_ends_before = numeric_limits<time_t>::max();
  SfuncSet::const_iterator s_min_end = sfuncset.end();
  bool schedule_min_end = false;
#endif
  double unitdur;
  vector<string>::const_iterator rin, rout;
  TintvlSet::iterator i;
  int quantity;

  //assumes NOW (or earliest schedulable time) < arrival !!!
  //otherwise: est_start = max (arrival, earliest schedulable time of shop)

  est_start = job->arrival;
  //for debugging:
  //if (schedInfo.best.empty())
  //{
  //  cerr << "step.funcseq = " << curstep.funcseq << "\tcell = " << curstep.cell;
  //  cerr << "step.seqid = " << curstep.seqid << endl;
  //  cerr << "cur = " << cur << "\tfuncseq = " << curfuncseq.name << endl;
  //}
  //string time_str;
  for(rin = curfuncseq.inrsrcs.begin(); rin != curfuncseq.inrsrcs.end(); ++rin) {
    //cerr << "\trin = " << *rin;
    map<string, time_t>::iterator tm;
    tm = schedInfo.rsrc2tm.find(*rin);
    assert(tm != schedInfo.rsrc2tm.end());
    rsrctm = (*tm).second;
    //for debugging:
    //string time_str;
    //GetTimeStr(time_str, rsrctm);
    //cerr << "\ttime = " << time_str << endl;
    if(rsrctm > est_start)
      est_start = rsrctm;
  }
  //for debugging:
  //GetTimeStr(time_str, est_start);
  //if (schedInfo.best.empty())
  //  cerr << "\tearliestart = " << time_str << endl;
  //rsrcs.insert(curfuncseq.outrsrcs.begin(), curfuncseq.outrsrcs.end());

  assert(!sfuncset.empty());
  s = sfuncset.begin();
  while(s != sfuncset.end()) {
    schedstep.step = curstep;
    schedstep.step.station = (*s).station;
    schedstep.step.opr = "any";
    //cerr << "Step id = " << cur << "-> trying station: " << setw(10) << (*s).station;
    schedstep.mach_tintvls.clear();
    //assumes NOW (or earliest schedulable time) < est_start !!!
    //otherwise: schedstep.tintvl.start = max (est_start, earliest schedulable time of this station)
    stime0 = stime1 = static_cast<time_t>(ceil((*s).funcseq.funcinfo.setuptime));
    const map<string, time_t> &sfuncAttr = (*s).funcseq.attributes;
    time_t stimeAttr;
    quantity = 0;
    for(rout = curfuncseq.outrsrcs.begin(); rout != curfuncseq.outrsrcs.end(); ++rout)
      quantity += schedInfo.rsrc2quantity->find(*rout)->second;
    if(quantity <= 0) {
      cerr << "Funcseq '" << curfuncseq.name;
      cerr << "': Invalid output resource quantity (" << quantity << ')' << endl;
      ::exit(-1);
    }
    FuncInfo tmp = (*s).funcseq.funcinfo;
    unitdur = 1.0 / (*s).funcseq.funcinfo.speedval;
    TintvlSet utintvls;
    bool jobmach_empty = schedInfo.jobmach2tintvl[(*s).station].empty();
    if(!jobmach_empty)
      TintvlSetUnion(mach2tintvl[(*s).station],
                     schedInfo.jobmach2tintvl[(*s).station], utintvls);
    TintvlSet &tintvls = jobmach_empty ? mach2tintvl[(*s).station] : utintvls;
    TintvlVec2d const &weekts = shopInfo.mach2weekts.find((*s).station)->second;
    DayTs const &dayts = shopInfo.mach2dayts.find((*s).station)->second;
    start = est_start;
    bool is_first_batch = true;
    if(!tintvls.empty()) {
      tintvl.start = est_start;
      i = tintvls.upper_bound(tintvl);
      if(i == tintvls.end()) {
        --i;
        const One2One &prevAttr = all_job_ptrs[(*i).intid]->funcseqs[(*i).seqid].attributes;
        stimeAttr = AttributeSetupTime(sfuncAttr, prevAttr, curAttr);
        if((*i).end >= est_start) {
          start = (*i).end + 1;
          if((*i).intid == job->intid && (*i).seqid == curSeqId) {
            stime0 = 0;
            stimeAttr = 0;
            is_first_batch = false;
          }
        }
        //else if ((*i).intid == job->intid && UninterruptedTimespan((*i).end, est_start, weekts, dayts))
        //    stime0 = 0;
#ifdef PSS_TRADE_QUALITY_FOR_SPEED
        if(start < min_ends_before) {
          nxt_start = numeric_limits<time_t>::max();
          QuantityTest(quantity, unitdur, stime0 + stimeAttr, stime1, is_first_batch,
                       start, nxt_start, weekts, dayts, ends_before); //just to get "ends_before"
        }
#endif
      } else {
        nxt_start = (*i).start;
        if(i != tintvls.begin()) {
          --i;
          const One2One &prevAttr = all_job_ptrs[(*i).intid]->funcseqs[(*i).seqid].attributes;
          stimeAttr = AttributeSetupTime(sfuncAttr, prevAttr, curAttr);
          if((*i).end >= est_start) {
            start = (*i).end + 1;
            if((*i).intid == job->intid && (*i).seqid == curSeqId) {
              stime0 = 0;
              stimeAttr = 0;
              is_first_batch = false;
            }
          }
          //else if ((*i).intid == job->intid && UninterruptedTimespan((*i).end, est_start, weekts, dayts))
          //    stime0 = 0;
          ++i;
        } else { // compute total attribute setup time since machine is empty
          stimeAttr = AttributeSetupTimeSum(sfuncAttr);
        }
        if(
#ifdef PSS_TRADE_QUALITY_FOR_SPEED
          start < min_ends_before &&
#endif
          !QuantityTest(quantity, unitdur, stime0 + stimeAttr, stime1,
                        is_first_batch, start, nxt_start, weekts, dayts, ends_before)) {
          //cannot squeeze in between
          if((*i).intid != job->intid || (*i).seqid != curSeqId) {
            stime0 = stime1;
            const One2One &prevAttr =
              all_job_ptrs[(*i).intid]->funcseqs[(*i).seqid].attributes;
            stimeAttr = AttributeSetupTime(sfuncAttr, prevAttr, curAttr);
            is_first_batch = true;
          } else {
            stime0 = stimeAttr = 0;
            is_first_batch = false;
          }
          start = (*i++).end + 1;
          while(
#ifdef PSS_TRADE_QUALITY_FOR_SPEED
            start < min_ends_before &&
#endif
            i != tintvls.end() &&
            !QuantityTest(quantity, unitdur, stime0 + stimeAttr, stime1,
                          is_first_batch, start, (*i).start, weekts,
                          dayts, ends_before)) {
            if((*i).intid != job->intid || (*i).seqid != curSeqId) {
              stime0 = stime1;
              const One2One &prevAttr =
                all_job_ptrs[(*i).intid]->funcseqs[(*i).seqid].attributes;
              stimeAttr = AttributeSetupTime(sfuncAttr, prevAttr, curAttr);
              is_first_batch = true;
            } else {
              stime0 = stimeAttr = 0;
              is_first_batch = false;
            }
            start = (*i++).end + 1;
          }
        }
        //else //no need to change est_start
      }
    } else { //machine schedule is empty --> est_start does not need to be changed
      // compute total attribute setup time since machine is empty
      stimeAttr = AttributeSetupTimeSum(sfuncAttr);
    }
#ifdef PSS_TRADE_QUALITY_FOR_SPEED
    if(start < min_ends_before) {
      nxt_start = numeric_limits<time_t>::max();
      QuantityTest(quantity, unitdur, stime0 + stimeAttr, stime1, is_first_batch,
                   start, nxt_start, weekts, dayts, ends_before); //just to get "ends_before"
    }
#endif
    //for debugging:
    //if (stime0 < stime1)
    //    cerr << "stime0 < stime1 for job id: " << job->intid << endl;
#ifdef PSS_TRADE_QUALITY_FOR_SPEED
    if(schedule_min_end == false) {
      if(start < min_ends_before && ends_before < min_ends_before) {
        min_ends_before = ends_before;
        s_min_end = s;
        ++s;
        if(s != sfuncset.end())  //not the end
          continue;
        else {
          --s; // go to EarliestSlot() below
          schedule_min_end = true;
        }
      } else { // not min-end
        ++s;
        if(s == sfuncset.end()) {
          assert(s_min_end != sfuncset.end());
          s = s_min_end;
          schedule_min_end = true;
        }
        continue;
      }
    }
#endif
    EarliestSlot(schedstep.mach_tintvls, job->intid, curSeqId, is_first_batch,
                 start, stime0 + stimeAttr, stime1, quantity, unitdur,
                 weekts, dayts);

    //for debugging:
    //string time_str;
    //GetTimeStr(time_str, schedstep.mach_tintvls.front().start);
    //cerr << " [" <<  time_str << ", ";
    //GetTimeStr(time_str, schedstep.mach_tintvls.back().end);
    //cerr << time_str << ']' << endl;

    for(rout = curfuncseq.outrsrcs.begin();
        rout != curfuncseq.outrsrcs.end();
        ++rout)
      schedInfo.rsrc2tm[*rout] = schedstep.mach_tintvls.back().end + 1;
    SchedStepQuantities(schedstep, (double)(stime0 + stimeAttr),
                        (double)stime1, (*s).funcseq.funcinfo.speedval, quantity);
    schedInfo.sched.push_back(schedstep);
    InsertMachTintvl(schedInfo.jobmach2tintvl, schedstep);
    schedInfo.cur++;
    FindSched(schedInfo, job, route, mach2tintvl, opr2tintvl, shopInfo, all_job_ptrs);
    schedInfo.cur--;
    schedInfo.sched.pop_back();
    RemoveMachTintvl(schedInfo.jobmach2tintvl, schedstep);
#ifdef PSS_TRADE_QUALITY_FOR_SPEED
    break; // exit the while loop for speed
#else
    ++s; //continue the while loop for quality
#endif
  }
  for(rout = curfuncseq.outrsrcs.begin();
      rout != curfuncseq.outrsrcs.end();
      ++rout) {
    schedInfo.rsrc2tm.erase(*rout);
  }
}

set<string> const &EligibleOprs(bool const oprltd, bool const useoprskills,
                                set<string> const &skilled_oprs,
                                set<string> const &any_opr,
                                set<string> const &last_opr) {
  if(oprltd) {
    if(useoprskills) {
      if(!last_opr.empty() &&
          (skilled_oprs.find(*last_opr.begin()) != skilled_oprs.end()))
        return last_opr;
      return skilled_oprs; //last opr doesn't possess the skill
    } else if(!last_opr.empty())
      return last_opr;
    else
      return skilled_oprs;
  }
  return any_opr;
}

void FindSchedOprltd(SchedInfo &schedInfo, const ShopJob *job,
                     Route const &route, Rsrc2Tintvl &mach2tintvl,
                     Rsrc2Tintvl &opr2tintvl, ShopInfo const &shopInfo,
                     const vector<ShopJob *> &all_job_ptrs, string lastopr) {
  if(schedInfo.cur == route.size()) {
    if(!schedInfo.best.empty()) {
      time_t best_end = schedInfo.best.back().mach_tintvls.back().end;
      time_t cur_end = schedInfo.sched.back().mach_tintvls.back().end;
      //if (best_end > cur_end || (best_end == cur_end && CompareSchedEndTime(best, sched) > 0))
      if(best_end > cur_end) {
        schedInfo.best = schedInfo.sched;
        //PrintSchedDebugInfo(schedInfo.best);
      }
    } else {
      //PrintSchedDebugInfo(schedInfo.best);
      schedInfo.best = schedInfo.sched;
    }
    return;
  }
  const Fstep &curstep = route[schedInfo.cur];
  const unsigned curSeqId = curstep.seqid;
  const Func &curfuncseq = job->funcseqs[curSeqId];
  const One2One &curAttr = curfuncseq.attributes;
  SfuncSet const &sfuncset =
    shopInfo.seq2mach.find(curstep.cell)->second.find(curstep.funcseq)->second;
  set<string> any_opr, last_opr;
  const bool oprltd = shopInfo.cell2config.find(curstep.cell)->second.oprlimited;
  if(!oprltd)
    any_opr.insert("any");
  else if(!lastopr.empty())
    last_opr.insert(lastopr);
  const bool useoprskills = shopInfo.cell2config.find(curstep.cell)->second.useoprskills;
  const bool useoprschds = shopInfo.cell2config.find(curstep.cell)->second.useoprschds;
  map<string, pss::One2Many>::const_iterator c2s2oItr = shopInfo.seq2opr.find(curstep.cell);
  if(c2s2oItr == shopInfo.seq2opr.end()) {
    throw RuntimeException("Unable to find an operator for function sequence: "
                           + curstep.funcseq + " in cell: " + curstep.cell);
  }
  One2Many::const_iterator s2oItr = c2s2oItr->second.find(curstep.funcseq);
  if(s2oItr == c2s2oItr->second.end()) {
    throw RuntimeException("Unable to find an operator for function sequence: "
                           + curstep.funcseq + " in cell: " + curstep.cell);
  }
  set<string> const &skilled_oprs = s2oItr->second;
  set<string> const &oprs = EligibleOprs(oprltd, useoprskills,
                                         skilled_oprs, any_opr, last_opr);
  if(oprs.empty()) {
    throw RuntimeException("Unable to find an operator for function sequence: "
                           + curstep.funcseq + " in cell: " + curstep.cell);
  }
  SfuncSet::const_iterator s;
  set<string>::const_iterator o;
  SchedStep schedstep;
  Tintvl tintvl;
  time_t rsrctm, start, est_start, nxt_start, stime0, stime1, ends_before;
#ifdef PSS_TRADE_QUALITY_FOR_SPEED
  time_t min_ends_before = numeric_limits<time_t>::max();
  SfuncSet::const_iterator s_min_end = sfuncset.end();
  set<string>::const_iterator o_min_end = oprs.end();
  bool schedule_min_end = false;
#endif
  double unitdur;
  vector<string>::const_iterator rin, rout;
  TintvlSet::iterator i;
  int quantity, oprdemand;

  //assumes NOW (or earliest schedulable time) < arrival !!!
  //otherwise: est_start = max (arrival, earliest schedulable time of shop)

  //for debugging:
  //if (schedInfo.best.empty())
  //    PrintSchedDebugInfo(sched);

  est_start = job->arrival;
  //for debuggin:
  //if (schedInfo.best.empty())
  //{
  //  cerr << "step.funcseq = " << curstep.funcseq << "\tcell = " << curstep.cell;
  //  cerr << "step.seqid = " << curstep.seqid << endl;
  //  cerr << "cur = " << cur << "\tfuncseq = " << curfuncseq.name << endl;
  //}
  //string time_str;
  for(rin = curfuncseq.inrsrcs.begin(); rin != curfuncseq.inrsrcs.end(); ++rin) {
    //cerr << "\trin = " << *rin;
    map<string, time_t>::iterator tm;
    tm = schedInfo.rsrc2tm.find(*rin);
    assert(tm != schedInfo.rsrc2tm.end());
    rsrctm = (*tm).second;
    //for debugging:
    //string time_str;
    //GetTimeStr(time_str, rsrctm);
    //cerr << "\ttime = " << time_str << endl;
    if(rsrctm > est_start)
      est_start = rsrctm;
  }
  //for debugging:
  //GetTimeStr(time_str, est_start);
  //if (schedInfo.best.empty())
  //  cerr << "\tearliestart = " << time_str << endl;
  //rsrcs.insert(curfuncseq.outrsrcs.begin(), curfuncseq.outrsrcs.end());

  assert(!sfuncset.empty());
  s = sfuncset.begin();
  while(s != sfuncset.end()) {
    schedstep.step = curstep;
    schedstep.step.station = (*s).station;
    oprdemand = oprltd ?(int)((*s).funcseq.funcinfo.oprdemand * 100) : 0;
    stime0 = stime1 = static_cast<time_t>(ceil((*s).funcseq.funcinfo.setuptime));
    const map<string, time_t> &sfuncAttr = (*s).funcseq.attributes;
    time_t stimeAttr;
    quantity = 0;
    for(rout = curfuncseq.outrsrcs.begin(); rout != curfuncseq.outrsrcs.end(); ++rout)
      quantity += schedInfo.rsrc2quantity->find(*rout)->second;
    if(quantity <= 0) {
      cerr << "Funcseq '" << curfuncseq.name;
      cerr << "': Invalid output resource quantity (" << quantity << ')' << endl;
      ::exit(-1);
    }
    unitdur = 1.0 / (*s).funcseq.funcinfo.speedval;

    TintvlSet mach_tintvls_union;
    bool jobmach_empty = schedInfo.jobmach2tintvl[(*s).station].empty();
    if(!jobmach_empty)
      TintvlSetUnion(mach2tintvl[(*s).station],
                     schedInfo.jobmach2tintvl[(*s).station], mach_tintvls_union);
    TintvlSet &mach_tintvls =
      jobmach_empty ? mach2tintvl[(*s).station] : mach_tintvls_union;
    TintvlVec2d const &mweekts = shopInfo.mach2weekts.find((*s).station)->second;
    DayTs const &mdayts = shopInfo.mach2dayts.find((*s).station)->second;

    o = oprs.begin();
#ifdef PSS_TRADE_QUALITY_FOR_SPEED
    if(schedule_min_end == true) {
      assert(o_min_end != oprs.end());
      o = o_min_end;
    }
#endif
    while(o != oprs.end()) {
      schedstep.step.opr = *o;
      //cerr << "Step id = " << cur << "-> trying station: " << setw(10) << (*s).station;
      schedstep.mach_tintvls.clear();
      schedstep.opr_tintvls.clear();
      //assumes NOW (or earliest schedulable time) < est_start !!!
      //otherwise: schedstep.tintvl.start = max (est_start, earliest schedulable time of this station)
      TintvlSet opr_tintvls_sum;
      bool jobopr_empty = schedInfo.jobopr2tintvl[*o].empty();
      if(!jobopr_empty)
        TintvlSetAdd(opr2tintvl[*o], schedInfo.jobopr2tintvl[*o], opr_tintvls_sum);
      TintvlSet &opr_tintvls = jobopr_empty ? opr2tintvl[*o] : opr_tintvls_sum;
      TintvlVec2d const &oweekts =
        useoprschds ? shopInfo.opr2weekts.find(*o)->second :
        shopInfo.opr2weekts.find("any")->second;
      DayTs const &odayts = useoprschds ? shopInfo.opr2dayts.find(*o)->second :
                            shopInfo.opr2dayts.find("any")->second;

      start = est_start;
      bool is_first_batch = true;
      if(!mach_tintvls.empty()) {
        tintvl.start = est_start;
        i = mach_tintvls.upper_bound(tintvl);
        if(i == mach_tintvls.end()) {
          --i;
          const One2One &prevAttr =
            all_job_ptrs[(*i).intid]->funcseqs[(*i).seqid].attributes;
          stimeAttr = AttributeSetupTime(sfuncAttr, prevAttr, curAttr);
          if((*i).end >= est_start) {
            start = (*i).end + 1;
            if((*i).intid == job->intid && (*i).seqid == curSeqId) {
              stime0 = 0;
              stimeAttr = 0;
              is_first_batch = false;
            }
          }
          //else if ((*i).intid == job->intid && UninterruptedTimespan((*i).end, est_start, mweekts, mdayts))
          //    stime0 = 0;
#ifdef PSS_TRADE_QUALITY_FOR_SPEED
          if(start < min_ends_before) {
            nxt_start = numeric_limits<time_t>::max();
            QuantityTestOprltd(quantity, unitdur, stime0 + stimeAttr, stime1, is_first_batch,
                               start, nxt_start, mweekts, mdayts, oprdemand, opr_tintvls,
                               oweekts, odayts, ends_before); //just to get "ends_before"
          }
#endif
        } else {
          nxt_start = (*i).start;
          if(i != mach_tintvls.begin()) {
            --i;
            const One2One &prevAttr =
              all_job_ptrs[(*i).intid]->funcseqs[(*i).seqid].attributes;
            stimeAttr = AttributeSetupTime(sfuncAttr, prevAttr, curAttr);
            if((*i).end >= est_start) {
              start = (*i).end + 1;
              if((*i).intid == job->intid && (*i).seqid == curSeqId) {
                stime0 = 0;
                stimeAttr = 0;
                is_first_batch = false;
              }
            }
            //else if ((*i).intid == job->intid && UninterruptedTimespan((*i).end, est_start, mweekts, mdayts))
            //    stime0 = 0;
            ++i;
          } else { // compute total attribute setup time since machine is empty
            stimeAttr = AttributeSetupTimeSum(sfuncAttr);
          }
          if(
#ifdef PSS_TRADE_QUALITY_FOR_SPEED
            start < min_ends_before &&
#endif
            !QuantityTestOprltd(quantity, unitdur, stime0 + stimeAttr, stime1, is_first_batch, start, nxt_start, mweekts, mdayts,
                                oprdemand, opr_tintvls, oweekts, odayts, ends_before)) {
            //cannot squeeze in between
            if((*i).intid != job->intid || (*i).seqid != curSeqId) {
              stime0 = stime1;
              const One2One &prevAttr = all_job_ptrs[(*i).intid]->funcseqs[(*i).seqid].attributes;
              stimeAttr = AttributeSetupTime(sfuncAttr, prevAttr, curAttr);
              is_first_batch = true;
            } else {
              stime0 = stimeAttr = 0;
              is_first_batch = false;
            }
            start = (*i++).end + 1;
            while(
#ifdef PSS_TRADE_QUALITY_FOR_SPEED
              start < min_ends_before &&
#endif
              i != mach_tintvls.end() &&
              !QuantityTestOprltd(quantity, unitdur, stime0 + stimeAttr,
                                  stime1, is_first_batch, start,
                                  (*i).start, mweekts, mdayts,
                                  oprdemand, opr_tintvls, oweekts,
                                  odayts, ends_before)) {
              if((*i).intid != job->intid || (*i).seqid != curSeqId) {
                stime0 = stime1;
                //const One2One &prevAttr = all_job_ptrs[(*i).intid]->funcseqs[(*i).seqid].attributes;
                ShopJob *sjptr = all_job_ptrs[(*i).intid];
                vector<Func> &funcseqs = sjptr->funcseqs;
                Func &func = funcseqs[(*i).seqid];
                const One2One &prevAttr = func.attributes;
                stimeAttr = AttributeSetupTime(sfuncAttr, prevAttr, curAttr);
                is_first_batch = true;
              } else {
                stime0 = stimeAttr = 0;
                is_first_batch = false;
              }
              start = (*i++).end + 1;
            }
          }
          //else //no need to change est_start
        }
      } else { //machine schedule is empty --> est_start does not need to be changed
        // compute total attribute setup time since machine is empty
        stimeAttr = AttributeSetupTimeSum(sfuncAttr);
      }
#ifdef PSS_TRADE_QUALITY_FOR_SPEED
      if(start < min_ends_before) {
        nxt_start = numeric_limits<time_t>::max();
        QuantityTestOprltd(quantity, unitdur, stime0 + stimeAttr,
                           stime1, is_first_batch,
                           start, nxt_start, mweekts, mdayts,
                           oprdemand, opr_tintvls,
                           oweekts, odayts, ends_before); //just to get "ends_before"
      }
#endif
      //for debugging:
      //if (stime0 < stime1)
      //    cerr << "stime0 < stime1 for job id: " << job->intid << endl;
#ifdef PSS_TRADE_QUALITY_FOR_SPEED
      if(schedule_min_end == false) {
        if(start < min_ends_before && ends_before < min_ends_before) {
          min_ends_before = ends_before;
          s_min_end = s;
          o_min_end = o;
          //test if (s, o) happens to be the very last one
          //if so, then go directly to earliset_slot() below
          //without re-generating (s_min_end, o_min_end)
          ++o;
          if(o != oprs.end())  //not the end
            continue;
          else { // o == oprs.end()
            ++s; // advance sfuncset iterator
            if(s != sfuncset.end()) {  //test if end of sfuncset too
              --s; //++s executed in the outer loop
              continue;
            }
            // reached end of sfuncset AND oprs
            schedule_min_end = true;
            --o;
            --s; // go to EarliestSlot() below
          }
        } else { // not min-end (s, o)
          ++o;
          continue;
        }
      }
#endif
      EarliestSlotOprltd(schedstep.mach_tintvls, schedstep.opr_tintvls,
                         job->intid, curSeqId,
                         is_first_batch, start, stime0 + stimeAttr,
                         stime1, quantity, unitdur, mweekts, mdayts,
                         oprdemand, opr_tintvls, oweekts, odayts);

      //for debugging:
      //string time_str;
      //GetTimeStr(time_str, schedstep.mach_tintvls.front().start);
      //cerr << " [" <<  time_str << ", ";
      //GetTimeStr(time_str, schedstep.mach_tintvls.back().end);
      //cerr << time_str << ']' << endl;

      for(rout = curfuncseq.outrsrcs.begin(); rout != curfuncseq.outrsrcs.end(); ++rout)
        schedInfo.rsrc2tm[*rout] = schedstep.mach_tintvls.back().end + 1;

      SchedStepQuantities(schedstep, (double)(stime0 + stimeAttr),
                          (double)stime1, (*s).funcseq.funcinfo.speedval, quantity);
      schedInfo.sched.push_back(schedstep);
      InsertMachTintvl(schedInfo.jobmach2tintvl, schedstep);
      TintvlSet &joboprtintvl = schedInfo.jobopr2tintvl[schedstep.step.opr];
      TintvlSetAdd(joboprtintvl, schedstep.opr_tintvls, joboprtintvl);
      schedInfo.cur++;
      FindSchedOprltd(schedInfo, job, route, mach2tintvl, opr2tintvl,
                      shopInfo, all_job_ptrs, *o);
      schedInfo.cur--;
      schedInfo.sched.pop_back();
      RemoveMachTintvl(schedInfo.jobmach2tintvl, schedstep);

      TintvlSetSubtract(joboprtintvl, schedstep.opr_tintvls, joboprtintvl);
      //TintvlSetSimplify(joboprtintvl); //does not seem to help
#ifdef PSS_TRADE_QUALITY_FOR_SPEED
      break; // exit the while loop for speed
#else
      ++o; //continue the while loop for quality
#endif
    }
    ++s; // advance sfuncset iterator
#ifdef PSS_TRADE_QUALITY_FOR_SPEED
    if(schedule_min_end == false) {
      if(s != sfuncset.end())  //test if end of sfuncset
        continue;
      // reached end of sfuncset
      assert(s_min_end != sfuncset.end() && o_min_end != oprs.end());
      s = s_min_end;
      //o = o_min_end; must be set in the inner loop
      schedule_min_end = true;
    } else
      break; // exit the while loop for speed
#endif
  }
  for(rout = curfuncseq.outrsrcs.begin(); rout != curfuncseq.outrsrcs.end(); ++rout) {
    schedInfo.rsrc2tm.erase(*rout);
  }
}

//ignore the "unit" of a resource quantity for now
void BuildRsrcQuantityMap(Rsrc2Qty &rsrc2quantity,
                          const vector<Resource> &resources) {
  vector<Resource>::const_iterator r;

  rsrc2quantity.clear();
  for(r = resources.begin(); r != resources.end(); ++r) {
    if((*r).quantity.value == 0)
      cerr << "Error: resource quantity = 0" << endl;
    rsrc2quantity[(*r).baseinfo.name] = (*r).quantity.value;
  }
}

void BuildRsrcUnitMap(map<string, string> &rsrc2unit,
                      const vector<Resource> &resources) {
  vector<Resource>::const_iterator r;

  rsrc2unit.clear();
  for(r = resources.begin(); r != resources.end(); ++r) {
    if((*r).quantity.value == 0)
      cerr << "Error: resource quantity = 0" << endl;
    rsrc2unit[(*r).baseinfo.name] = (*r).quantity.unit;
  }
}

bool BatchRoute(const Route &route, map<string, CellConfig> const &cell2config) {
  Route::const_iterator s;

  for(s = route.begin(); s != route.end(); ++s) {
    if(cell2config.find((*s).cell)->second.batching == false)
      return false;
  }
  return true;
}

void RouteRsrcMinBatch(const Route &route, const vector<Resource> &resources,
                       const map<string, Rsrc2Qty> unit2minbatch,
                       Rsrc2Qty &route_rsrc2minbatch) {
  Route::const_iterator s;
  Rsrc2Qty u2q; //mapping from unit to minbatch quantity

  route_rsrc2minbatch.clear();
  for(s = route.begin(); s != route.end(); ++s) {
    map<string, Rsrc2Qty>::const_iterator cell_itr;
    cell_itr = unit2minbatch.find((*s).cell);
    if(cell_itr != unit2minbatch.end()) {
      Rsrc2Qty::const_iterator u2q_itr;
      for(u2q_itr = cell_itr->second.begin();
          u2q_itr != cell_itr->second.end();
          ++u2q_itr) {
        if(u2q[u2q_itr->first] < u2q_itr->second) {
          u2q[u2q_itr->first] = u2q_itr->second;
          //cerr << "route rsrc " << u2q_itr->first << " min batch = " << u2q_itr->second << endl;
        }
      }
    }
  }
  vector<Resource>::const_iterator r;
  for(r = resources.begin(); r != resources.end(); ++r)
    route_rsrc2minbatch[(*r).baseinfo.name] = u2q[(*r).quantity.unit];
}

unsigned BatchSplit(const Rsrc2Qty &rsrc2quantity, const Rsrc2Qty rsrc2minbatch,
                    Rsrc2Qty &rsrc2quantity1, unsigned &numbatch1,
                    Rsrc2Qty &rsrc2quantity2, unsigned &numbatch2,
                    unsigned maxbatch) {
  unsigned min, multiplier, maxmultiplier, surplus, numbatch;
  Rsrc2Qty::const_iterator r;

  assert(!rsrc2quantity.empty());
  rsrc2quantity1.clear();
  rsrc2quantity2.clear();
  min = numeric_limits<int>::max();
  for(r = rsrc2quantity.begin(); r != rsrc2quantity.end(); ++r) {
    if(r->second < min)
      min = r->second;
  }
  assert(min != 0);
  //cerr << "min = " << min << endl;
  maxmultiplier = 1;
  for(r = rsrc2quantity.begin(); r != rsrc2quantity.end(); ++r) {
    if(r->second % min != 0) {
      numbatch1 = 1;
      numbatch2 = 0;
      rsrc2quantity1 = rsrc2quantity;
      return 1;
    } else {
      Rsrc2Qty::const_iterator itr;
      itr = rsrc2minbatch.find(r->first);
      //cerr << "r->first = " << r->first << " r->second = " << r->second << endl;
      if(itr != rsrc2minbatch.end() && itr->second > r->second / min) {
        multiplier = static_cast<unsigned>(ceil(itr->second * min /
                                                static_cast<double>(r->second)));
        //cerr << "multiplier = " << multiplier << endl;
        maxmultiplier = max(maxmultiplier, multiplier);
        if(maxmultiplier >= min)
          break;
      }
    }
  }
  numbatch = min / maxmultiplier;
  surplus = min % maxmultiplier;
  if(numbatch <= 1 || maxbatch <= 1) {
    numbatch1 = 1;
    numbatch2 = 0;
    rsrc2quantity1 = rsrc2quantity;
    return 1;
  }
  if(numbatch > maxbatch) {
    numbatch = maxbatch;
    maxmultiplier = min / numbatch;
    numbatch1 = min % numbatch;
  } else if(numbatch > surplus) {
    numbatch1 = surplus;
  } else { //numbatch <= surplus
    maxmultiplier += surplus / numbatch;
    numbatch1 = surplus % numbatch;
  }
  numbatch2 = numbatch - numbatch1;
  //for debugging:
  //cerr << "maxmultipler = " << maxmultiplier << endl;
  //cerr << "numbatch = " << numbatch << endl;
  //cerr << "numbatch1 = " << numbatch1 << endl;
  //cerr << "numbatch2 = " << numbatch2 << endl;
  for(r = rsrc2quantity.begin(); r != rsrc2quantity.end(); ++r) {
    rsrc2quantity1[r->first] = (maxmultiplier + 1) * r->second / min;
    rsrc2quantity2[r->first] = maxmultiplier * r->second / min;
    //cerr << "r2q1[" << r->first << "] = " << (maxmultiplier + 1) * r->second / min;
    //cerr << ", r2q2[" << r->first << "] = " << maxmultiplier * r->second / min << endl;
  }
  return (numbatch1 + numbatch2);
}

bool TintvlLessThan(const Tintvl &tintvl1, const Tintvl &tintvl2) {
  return tintvl1.start < tintvl2.start;
}

//merge consecutive tintvls into a single one
unsigned SimplifySchedStep(SchedStep &simplified, const SchedStep step) {
  map<Tintvl, int, LtTintvl> t2q;
  map<Tintvl, int, LtTintvl>::const_iterator tq;
  simplified.step = step.step;
  assert(step.mach_tintvls.size() == step.quantities.size());
  unsigned s, size = (unsigned)step.quantities.size();
  for(s = 0; s < size; ++s)
    t2q[step.mach_tintvls[s]] = step.quantities[s];
  //sort(step.mach_tintvls.begin(), step.mach_tintvls.end(), TintvlLessThan);
  simplified.mach_tintvls.clear();
  simplified.quantities.clear();
  int quantity;
  Tintvl tintvl;
  unsigned merge_counter = 0;
  for(tq = t2q.begin(); tq != t2q.end(); ++tq) {
    if(tq != t2q.begin()) {
      assert(tintvl.intid == tq->first.intid);
      if(tq->first.start == tintvl.end + 1) {
        tintvl.end = tq->first.end;
        quantity += tq->second;
        ++merge_counter;
      } else {
        simplified.mach_tintvls.push_back(tintvl);
        simplified.quantities.push_back(quantity);
        tintvl = tq->first;
        quantity = tq->second;
      }
    } else {
      tintvl = tq->first;
      quantity = tq->second;
    }
  }
  if(!t2q.empty()) {
    simplified.mach_tintvls.push_back(tintvl);
    simplified.quantities.push_back(quantity);
  }
  return merge_counter;
}

struct LtStep {
  bool operator()(const Fstep &step1, const Fstep &step2) const {
    return step1.station < step2.station;
  }
};

unsigned MergeBatchScheds(Sched &best, vector<Sched> &batch_scheds) {
  assert(!batch_scheds.empty());
  unsigned b, batches = (unsigned)batch_scheds.size();
  unsigned s, steps = (unsigned)batch_scheds[0].size();
  map<Fstep, SchedStep, LtStep> stepmerged;
  map<Fstep, SchedStep, LtStep>::iterator m;
  unsigned min_merged = numeric_limits<unsigned>::max();

  for(s = 0; s < steps; ++s) {
    stepmerged.clear();
    for(b = 0; b < batches; ++b) {
      SchedStep &schedstep = batch_scheds[b][s];
      m = stepmerged.find(schedstep.step);
      if(m == stepmerged.end())
        stepmerged[schedstep.step] = schedstep;
      else {
        m->second.mach_tintvls.insert(m->second.mach_tintvls.end(),
                                      schedstep.mach_tintvls.begin(),
                                      schedstep.mach_tintvls.end());
        m->second.quantities.insert(m->second.quantities.end(),
                                    schedstep.quantities.begin(),
                                    schedstep.quantities.end());
        //for debugging:
        //if (station == "NCRPad") {
        //  cerr << "Batch #" << b << ", step #" << s << ':';
        //  for (vector<Tintvl>::iterator i = m->second.mach_tintvls.begin(); i != m->second.mach_tintvls.end(); ++i) {
        //    cerr << " [" << (*i).start << ((*i).start <= (*i).end ? " <= " : " > ") << (*i).end << ']';
        //  }
        //  cerr << endl;
        //}
      }
    }
    for(m = stepmerged.begin(); m != stepmerged.end(); ++m) {
      SchedStep simplified;
      unsigned merged = SimplifySchedStep(simplified, m->second);
      assert(merged <= batches &&
             "Number of merged batches less than total number of batches");
      if(merged < min_merged)
        min_merged = merged;
      best.push_back(simplified);
    }
  }
  if(min_merged == numeric_limits<unsigned>::max())
    return batches;
  return (batches - min_merged);
}

void PrintUnitRsrcTintvl(Rsrc2Tintvl &rsrc2tintvl, string rsrc) {
  TintvlSet &tintvls = rsrc2tintvl[rsrc];

  cerr << "Resource: " << rsrc;
  for(TintvlSet::iterator i = tintvls.begin(); i != tintvls.end(); ++i)
    cerr << " [" << (*i).start << ((*i).start <= (*i).end ? " <= " : " > ")
         << (*i).end << ']';
  cerr << endl;
}

void PrintUnitRsrcTintvl(Rsrc2Tintvl &rsrc2tintvl) {
  Rsrc2Tintvl::const_iterator r;

  for(r = rsrc2tintvl.begin(); r != rsrc2tintvl.end(); ++r) {
    cerr << "Resource: " << setw(10) << r->first;
    for(TintvlSet::const_iterator i = r->second.begin(); i != r->second.end(); ++i) {
      string time_str;
      GetTimeStr(time_str, (*i).start);
      cerr << " [" << time_str;
      //cerr << ((*i).start <= (*i).end ? " <= " : " > ");
      cerr << ", ";
      GetTimeStr(time_str, (*i).end);
      cerr << time_str << "] " << setw(5) << (*i).end - (*i).start + 1;
    }
    cerr << endl;
  }
}

double UnitRsrcUtilization(const TintvlSet &tintvls, const time_t start,
                           const time_t end, const TintvlVec2d &weekts,
                           const DayTs &dayts) {
  double dur = static_cast<double>(timespan(start, end, weekts, dayts));
  time_t utildur = 0;

  assert(end >= start);
  for(TintvlSet::const_iterator i = tintvls.begin();
      i != tintvls.end() && (*i).start <= end;
      ++i) {
    if((*i).start >= start) {
      if((*i).end <= end)
        utildur += (*i).end - (*i).start + 1;
      else
        utildur += end - (*i).start + 1;
    } else if((*i).end >= start) {
      if((*i).end <= end)
        utildur += (*i).end - start + 1;
      else
        utildur +=  end - start + 1;
    }
  }
  return (utildur / dur);
}

void PrintUnitRsrcUtilization(Rsrc2Tintvl &rsrc2tintvl,
                              time_t start, time_t end,
                              map<string, TintvlVec2d> &rsrc2weekts,
                              map<string, DayTs> &rsrc2dayts) {
  Rsrc2Tintvl::const_iterator r;

  for(r = rsrc2tintvl.begin(); r != rsrc2tintvl.end(); ++r) {
    cerr << "Machine '" << setw(30) << r->first;
    cerr << "': ";
    cerr << UnitRsrcUtilization(r->second, start, end, rsrc2weekts[r->first],
                                rsrc2dayts[r->first]) * 100.0 << '%' << endl;
  }
}

void ResetSchedInfo(SchedInfo &schedInfo) {
  schedInfo.best.clear();
  schedInfo.sched.clear();
  schedInfo.cur = 0;
  schedInfo.rsrc2tm.clear();
  //schedInfo.rsrc2quantity.clear(); don't clear rsrc2quantity
}

bool OprltdCellInRoute(const Route &route, map<string,
                       CellConfig> const &cell2config) {
  Route::const_iterator s;

  for(s = route.begin(); s != route.end(); ++s) {
    if(cell2config.find((*s).cell)->second.oprlimited == true)
      return true;
  }
  return false;
}

void FindMinCompletionSched(Sched &sched, ShopJob *job,
                            const vector<Route> &routes,
                            Rsrc2Tintvl &mach2tintvl,
                            Rsrc2Tintvl &opr2tintvl,
                            const ShopInfo &shopInfo,
                            const vector<ShopJob *> &all_job_ptrs) {
  Rsrc2Qty r2q, r2q1, r2q2;
  Sched best, batch_best, tmp;
  map<string, time_t> rsrc2tm;
  Rsrc2Tintvl jobmach2tintvl;
  vector<Route>::const_iterator r;
  time_t mincompletion = numeric_limits<time_t>::max();
  unsigned tot_batch, num_batches, num_batch1, num_batch2, max_batch;
  vector<Sched> batch_scheds;
  SchedInfo schedInfo;
  bool has_oprltd_cell;

  max_batch = (shopInfo.config.batchlimit == 0) ? 20 :
              shopInfo.config.batchlimit; //make 20 a parameter in future
  BuildRsrcQuantityMap(r2q, job->resources);
  for(r = routes.begin(); r != routes.end(); ++r) {
    best.clear();
    schedInfo.jobmach2tintvl.clear();
    schedInfo.jobopr2tintvl.clear();
    has_oprltd_cell = OprltdCellInRoute(*r, shopInfo.cell2config);
    if(BatchRoute(*r, shopInfo.cell2config)) {
      Rsrc2Qty route_rsrc2minbatch;
      RouteRsrcMinBatch(*r, job->resources, shopInfo.unit2minbatch,
                        route_rsrc2minbatch);
      batch_scheds.clear();
      tot_batch = BatchSplit(r2q, route_rsrc2minbatch, r2q1, num_batch1,
                             r2q2, num_batch2, max_batch);
      assert(tot_batch >= 1);
      if(num_batch1 > 0)
        schedInfo.rsrc2quantity = &r2q1;
      for(unsigned b = 0; b < tot_batch; ++b) {
        ResetSchedInfo(schedInfo);
        if(b == num_batch1)
          schedInfo.rsrc2quantity = &r2q2;

        if(!has_oprltd_cell)
          FindSched(schedInfo, job, *r, mach2tintvl, opr2tintvl,
                    shopInfo, all_job_ptrs);
        else
          FindSchedOprltd(schedInfo, job, *r, mach2tintvl, opr2tintvl,
                          shopInfo, all_job_ptrs, "");
        CommitSchedule(schedInfo.best, schedInfo.jobmach2tintvl,
                       schedInfo.jobopr2tintvl);
        batch_scheds.push_back(schedInfo.best);
        //for debugging:
        //cerr << "batch = " << b << endl;
        //PrintUnitRsrcTintvl(schedInfo.jobmach2tintvl);
        //cerr << "size of batch_scheds = " << batch_scheds.size() << endl;
      }
      num_batches = MergeBatchScheds(best, batch_scheds);
    } else {
      num_batches = 1;
      ResetSchedInfo(schedInfo);
      schedInfo.rsrc2quantity = &r2q;

      if(!has_oprltd_cell)
        FindSched(schedInfo, job, *r, mach2tintvl, opr2tintvl,
                  shopInfo, all_job_ptrs);
      else
        FindSchedOprltd(schedInfo, job, *r, mach2tintvl, opr2tintvl,
                        shopInfo, all_job_ptrs, "");

      best = schedInfo.best;
    }
    if(best.back().mach_tintvls.back().end < mincompletion) {
      mincompletion = best.back().mach_tintvls.back().end;
      sched = best;
      job->numbatches = num_batches;
    }
  }
}

bool CheckTintvl(Rsrc2Tintvl &rsrc2tintvl) {
  Rsrc2Tintvl::const_iterator r;
  TintvlSet::const_iterator t;
  Tintvl prev, cur;
  bool result = true;

  for(r = rsrc2tintvl.begin(); r != rsrc2tintvl.end(); ++r) {
    prev.start = prev.end = numeric_limits<time_t>::min();
    for(t = r->second.begin(); t != r->second.end(); ++t) {
      cur = *t;
      if(cur.start < 0 || cur.end < 0 || cur.start > cur.end) {
        cerr << "Invalid time interval [" << cur.start << ", " << cur.end
             << "] for resource: " << r->first << endl;
        result = false;
      }
      if(cur.start <= prev.end) {
        cerr << "Time interval [" << cur.start << ", " << cur.end
             << "] overlaps with previous interval [" << prev.start
             << ", " << prev.end
             << "] for resource: " << r->first << endl;
        result = false;
      }
    }
  }
  return result;
}

void BuildStepId2SeqIdMap(map<unsigned, unsigned> &stepid2seqid,
                          const vector<Func> &funcseqs) {
  unsigned seqid;
  vector<unsigned>::const_iterator f;

  stepid2seqid.clear();
  for(seqid = 0; seqid < funcseqs.size(); ++seqid) {
    for(f = funcseqs[seqid].stepids.begin(); f != funcseqs[seqid].stepids.end(); ++f)
      stepid2seqid[*f] = seqid;
  }
}

void BuildSeqId2SchedIdsMap(map<unsigned, vector<unsigned> > &seqid2schedids,
                            const Sched &sched) {
  unsigned s;

  seqid2schedids.clear();
  for(s = 0; s < sched.size(); ++s) {
    seqid2schedids[sched[s].step.seqid].push_back(s);
  }
}

int GetSchedStepQuantity(const SchedStep &schedstep) {
  vector<int>::const_iterator q;
  int sum = 0;

  for(q = schedstep.quantities.begin(); q != schedstep.quantities.end(); ++q)
    sum += *q;
  return sum;
}

time_t GetSchedEndTime(const Sched &sched) {
  Sched::const_iterator s;
  time_t sched_end = numeric_limits<time_t>::min();

  for(s = sched.begin(); s != sched.end(); ++s)
    sched_end = max(sched_end, (*s).mach_tintvls.back().end);

  //for debugging:
  //string time_str;
  //GetTimeStr(time_str, sched_end);
  //cerr << "sched end time = " << time_str << endl;

  return sched_end;
}

#ifdef _DEBUG
void ValidateSched(map<unsigned, Sched> &scheds, vector<ShopJob> &shop_jobs,
                   Rsrc2Tintvl &mach2tintvl) {
  vector<ShopJob>::const_iterator j;
  vector<unsigned>::const_iterator si;
  unsigned stepid;
  time_t sched_start, sched_end;
  Rsrc2Qty r2q, r2q_;
  map<string, string> r2u;
  map<unsigned, unsigned> stepid2seqid;
  map<unsigned, vector<unsigned> > seqid2schedids;
  vector<string>::const_iterator rout;
  Rsrc2Qty unit2quantity;

  for(j = shop_jobs.begin(); j != shop_jobs.end(); ++j) {
    BuildRsrcQuantityMap(r2q, (*j).resources);
    BuildRsrcUnitMap(r2u, (*j).resources);
    BuildStepId2SeqIdMap(stepid2seqid, (*j).funcseqs);
    Sched &sched = scheds[(*j).intid];
    BuildSeqId2SchedIdsMap(seqid2schedids, sched);
    sched_start = numeric_limits<time_t>::max();
    sched_end = numeric_limits<time_t>::min();
    r2q_.clear();
    for(stepid = 0; stepid < (*j).funcsteps.size(); ++stepid) {
      vector<unsigned> &schedids = seqid2schedids[stepid2seqid[stepid]];
      for(si = schedids.begin(); si != schedids.end(); ++si) {
        const SchedStep &step = sched[*si];
        assert(step.step.funcseq == (*j).funcseqs[step.step.seqid].name);
        int quantity = GetSchedStepQuantity(step);
        vector<Tintvl>::const_iterator t;
        for(t = step.mach_tintvls.begin(); t != step.mach_tintvls.end(); ++t) {
          assert(mach2tintvl[step.step.station].find(*t) !=
                 mach2tintvl[step.step.station].end());
          sched_start = min(sched_start, (*t).start);
          sched_end = max(sched_end, (*t).end);
        }
        const vector<string> &outrsrcs = (*j).funcsteps[stepid].outrsrcs;
        for(rout = outrsrcs.begin(); rout != outrsrcs.end(); ++rout) {
          r2q_[*rout] += quantity;
          unit2quantity[r2u[*rout]] += quantity;
        }
      }
    }
    if(r2q != r2q_) {
      cerr << "Error: Mismatching resource quantities for job " << (*j).id << endl;
      cerr << endl << "Original resource quantities: " << endl;
      for(Rsrc2Qty::const_iterator r = r2q.begin(); r != r2q.end(); ++r)
        cerr << r->first << " = " << r->second << endl;
      cerr << endl << "Scheduled resource quantities: " << endl;
      for(Rsrc2Qty::const_iterator r = r2q_.begin(); r != r2q_.end(); ++r)
        cerr << r->first << " = " << r->second << endl;
      cerr << endl;
    }
    assert(r2q == r2q_);
    assert(sched_start == (*j).released);
    assert(sched_end == (*j).completed);
  }
  //for debugging:
  //cerr << "        Unit :   Quantity" << endl;
  //Rsrc2Qty::const_iterator uq;
  //for (uq = unit2quantity.begin(); uq != unit2quantity.end(); ++uq)
  //{
  //  cerr <<  setw(12) << uq->first;
  //  cerr << " = " << setw(10) << uq->second << endl;
  //}
}
#endif

int CompareAttribute(const One2One &attr1, const One2One &attr2) {
  One2One::const_iterator itr1, itr2;
  for(itr1 = attr1.begin(), itr2 = attr2.begin();
      itr1 != attr1.end() && itr2 != attr2.end();
      ++itr1, ++itr2) {
    int compareName = itr1->first.compare(itr2->first);
    if(compareName == 0) {
      int compareValue = itr2->second.compare(itr2->second);
      if(compareValue < 0)
        return -1;
      else if(compareValue > 0)
        return 1;
    } else if(compareName < 0)
      return -1;
    else
      return 1;
  }
  if(itr1 == attr1.end()) {
    if(itr2 == attr2.end())
      return 0;
    else
      return -1;
  }
  return 1;
}

int CompareFuncVector(const vector<Func> &funcVec1, const vector<Func> &funcVec2) {
  vector<Func>::const_iterator itr1, itr2;

  for(itr1 = funcVec1.begin(), itr2 = funcVec2.begin();
      itr1 != funcVec1.end() && itr2 != funcVec2.end();
      ++itr1, ++itr2) {
    int attributeCompareCode = CompareAttribute(itr1->attributes, itr2->attributes);
    if(attributeCompareCode < 0)
      return -1;
    else if(attributeCompareCode > 0)
      return 1;
  }
  if(itr1 == funcVec1.end()) {
    if(itr2 == funcVec2.end())
      return 0;
    else
      return -1;
  }
  return 1;
}

bool FIFO(const ShopJob *job1, const ShopJob *job2) {
  if(job1->arrival < job2->arrival)
    return true;
  if(job1->arrival > job2->arrival)
    return false;
#ifndef PSS_NO_SORTED_ATTRIBUTE
  int funcseqCompareCode = CompareFuncVector(job1->funcseqs, job2->funcseqs);
  if(funcseqCompareCode < 0)
    return true;
  if(funcseqCompareCode > 0)
    return false;
#endif
  if(job1->due < job2->due)
    return true;
  if(job1->due > job2->due)
    return false;
  if(job1->due - job1->arrival < job2->due - job2->arrival)
    return true;
  return false;
}

bool EarliestDue(const ShopJob *job1, const ShopJob *job2) {
  if(job1->due < job2->due)
    return true;
  if(job1->due > job2->due)
    return false;
#ifndef PSS_NO_SORTED_ATTRIBUTE
  int funcseqCompareCode = CompareFuncVector(job1->funcseqs, job2->funcseqs);
  if(funcseqCompareCode < 0)
    return true;
  if(funcseqCompareCode > 0)
    return false;
#endif
  if(job1->due - job1->arrival < job2->due - job2->arrival)
    return true;
  if(job1->due - job1->arrival > job2->due - job2->arrival)
    return false;
  if(job1->arrival < job2->arrival)
    return true;
  return false;
}

bool LeastSlack(const ShopJob *job1, const ShopJob *job2) {
  if(job1->due - job1->arrival < job2->due - job2->arrival)
    return true;
  if(job1->due - job1->arrival > job2->due - job2->arrival)
    return false;
#ifndef PSS_NO_SORTED_ATTRIBUTE
  int funcseqCompareCode = CompareFuncVector(job1->funcseqs, job2->funcseqs);
  if(funcseqCompareCode < 0)
    return true;
  if(funcseqCompareCode > 0)
    return false;
#endif
  if(job1->due < job2->due)
    return true;
  if(job1->due > job2->due)
    return false;
  if(job1->arrival < job2->arrival)
    return true;
  return false;
}

bool ShortestProcTime(const ShopJob *job1, const ShopJob *job2) {
  if(job1->proctime < job2->proctime)
    return true;
  if(job1->proctime > job2->proctime)
    return false;
#ifndef PSS_NO_SORTED_ATTRIBUTE
  int funcseqCompareCode = CompareFuncVector(job1->funcseqs, job2->funcseqs);
  if(funcseqCompareCode < 0)
    return true;
  if(funcseqCompareCode > 0)
    return false;
#endif
  if(job1->due - job1->arrival < job2->due - job2->arrival)
    return true;
  if(job1->due - job1->arrival > job2->due - job2->arrival)
    return false;
  if(job1->due < job2->due)
    return true;
  if(job1->due > job2->due)
    return false;
  if(job1->arrival < job2->arrival)
    return true;
  return false;
}

time_t GetMakespan(const Rsrc2Tintvl &rsrc2tintvl, time_t &start, time_t &end) {
  Rsrc2Tintvl::const_iterator r;
  Tintvl tintvl;
  time_t mintime, maxtime;

  mintime = numeric_limits<time_t>::max();
  maxtime = numeric_limits<time_t>::min();
  for(r = rsrc2tintvl.begin(); r != rsrc2tintvl.end(); ++r) {
    if(!r->second.empty()) {
      tintvl = *(r->second.begin());
      if(tintvl.start < mintime)
        mintime = tintvl.start;
      tintvl = *(r->second.rbegin());
      if(tintvl.end > maxtime)
        maxtime = tintvl.end;
    }
  }
  start = mintime;
  end = maxtime;
  if(maxtime > mintime)
    return (maxtime - mintime);
  return 0;
}

void InitSeqPolicyDict(map<string, OrderFptr, CaseInsensitiveLess> &dict) {
  dict["firstInFirstOut"] = FIFO;
  dict["earliestDue"] = EarliestDue;
  dict["leastSlack"] = LeastSlack;
  dict["shortestProcTime"] = ShortestProcTime;
}

void SchedTintvl(Tintvl &tintvl, Sched &sched) {
  Sched::const_iterator s;
  time_t mintime, maxtime;

  mintime = numeric_limits<time_t>::max();
  maxtime = numeric_limits<time_t>::min();
  for(s = sched.begin(); s != sched.end(); ++s) {
    if((*s).mach_tintvls.front().start < mintime)
      mintime = (*s).mach_tintvls.front().start;
    if((*s).mach_tintvls.back().end > maxtime)
      maxtime = (*s).mach_tintvls.back().end;
  }
  tintvl.start = mintime;
  tintvl.end = maxtime;
  if(mintime > maxtime)
    cerr << "Invalid schedule start time: " << mintime <<
         ", and end time: " << maxtime << endl;
}

void ScheduleJob(Sched &sched, ShopJob *shopJob, unsigned priority,
                 Rsrc2Tintvl &mach2tintvl, Rsrc2Tintvl &opr2tintvl,
                 const ShopInfo &shopInfo, const vector<ShopJob *> &all_job_ptrs) {
  vector<Route> routes;
  try {
    FindMinhopRoutes(routes, shopJob->funcseqs, shopInfo.seq2cell);
  } catch(RuntimeException &e) {
    cerr << e.what() << endl;
    throw RuntimeException("Unable to find route that produces job with id: "
                           + shopJob->id);
  }
  if(routes.empty()) {
    throw RuntimeException("No cell route found to produce job with id: "
                           + shopJob->id);
  }
  FindMinCompletionSched(sched, shopJob, routes, mach2tintvl, opr2tintvl,
                         shopInfo, all_job_ptrs);
  SimplifyOprTintvl(sched);

  Tintvl tintvl;
  SchedTintvl(tintvl, sched);
  shopJob->priority = priority;
  shopJob->released = tintvl.start;
  shopJob->completed = tintvl.end;
}

void ShopJobPointers(vector<ShopJob *> &shop_job_pointers,
                     vector<ShopJob> &shop_jobs) {
  shop_job_pointers.clear();
  for(vector<ShopJob>::iterator it = shop_jobs.begin(); it != shop_jobs.end(); ++it) {
    shop_job_pointers.push_back(&*it);
  }
}

void DoSchedule(map<unsigned, Sched> &scheds, vector<ShopJob> &shop_jobs,
                const ShopInfo &shopInfo,
                Rsrc2Tintvl &mach2tintvl, Rsrc2Tintvl &opr2tintvl,
                const vector<ShopJob *> &all_job_ptrs) {
  map<string, OrderFptr, CaseInsensitiveLess> seqpolicydict_;
  InitSeqPolicyDict(seqpolicydict_);
  vector<ShopJob *> shop_job_ptrs;
  ShopJobPointers(shop_job_ptrs, shop_jobs);
  sort(shop_job_ptrs.begin(), shop_job_ptrs.end(),
       seqpolicydict_[shopInfo.config.sequencepolicy]);

  int priority = 1;
  //cerr << "total jobs = " << shop_jobs.size() << endl;
  for(vector<ShopJob *>::iterator j = shop_job_ptrs.begin();
      j != shop_job_ptrs.end();
      ++j) {
    //cerr << "(*j)->intid = " << (*j)->intid << endl;
    ScheduleJob(scheds[(*j)->intid], *j, priority, mach2tintvl,
                opr2tintvl, shopInfo, all_job_ptrs);
    //cerr << priority << " jobs scheduled\n";
    ++priority;
    CommitSchedule(scheds[(*j)->intid], mach2tintvl, opr2tintvl);
    //assert(CheckTintvl(mach2tintvl)); //optional
  }
  assert(CheckTintvl(mach2tintvl)); //optional
  //PrintUnitRsrcTintvl(mach2tintvl);
  //schedStats.makespan = GetMakespan(mach2tintvl, schedStats.start, schedStats.end);
  //PrintUnitRsrcUtilization(mach2tintvl, start, end, mach2weekts, mach2dayts);

#ifdef _DEBUG
  ValidateSched(scheds, shop_jobs, mach2tintvl);
#endif
  //GetSchedStats(schedStats, shop_jobs, mach2tintvl);
}

void GetSchedStats(SchedStats &schedStats, const vector<ShopJob *> &shop_jobs,
                   const Rsrc2Tintvl &mach2tintvl) {
  int delay, maxdelay, numlate, numjobs = (int)shop_jobs.size();
  double sumdelay, sumdelayonly, avgdelay, sumsquare, stdevdelay, sumtat, sumproct;

  schedStats.makespan = GetMakespan(mach2tintvl, schedStats.start, schedStats.end);
  maxdelay = numeric_limits<int>::min();
  numlate = 0;
  sumdelay = sumdelayonly = sumsquare = sumtat = sumproct = 0.0;
  for(vector<ShopJob *>::const_iterator j = shop_jobs.begin(); j != shop_jobs.end(); ++j) {
    delay = (int)((*j)->completed - (*j)->due);
    sumdelay += (double) delay;
    sumsquare += (double)delay * (double)delay;
    sumtat += (*j)->completed - (*j)->arrival;
    sumproct += (*j)->completed - (*j)->released;
    if(delay > 0) {
      ++numlate;
      sumdelayonly += (double) delay;
    }
    if(maxdelay < delay)
      maxdelay = delay;
  }
  sumdelay /= 3600.0;
  sumdelayonly /= 3600.0;
  sumsquare /= 3600.0 * 3600.0;
  sumtat /= 3600.0;
  sumproct /= 3600.0;
  avgdelay = sumdelay / (double) numjobs;
  stdevdelay = sqrt(sumsquare / numjobs - (double) avgdelay * (double) avgdelay);
  schedStats.jobs = numjobs;
  schedStats.latejobs = numlate;
  schedStats.latepercent = 100 * numlate / (double) numjobs;
  schedStats.avgdelay = avgdelay;
  schedStats.stdevdelay = stdevdelay;
  schedStats.avgdelay_lateonly = (numlate > 0)? sumdelayonly / (double) numlate : 0.0;
  schedStats.maxdelay = (maxdelay > 0)? maxdelay / 3600.0 : 0.0;
  schedStats.avg_tat = sumtat / numjobs;
  schedStats.avg_proc_time = sumproct / numjobs;
}

void PrintSchedStats(ostream &os, const SchedStats &schedStats,
                     unsigned shop_id, bool multiSite) {
  char buf[100], *bufptr;
  struct tm tmval, *tmptr;

#ifndef PSS_TRADE_QUALITY_FOR_SPEED
  os << "Scheduling mode = Quality" << endl;
#else
  os << "Scheduling mode = Speed" << endl;
#endif
  os << "CPU seconds = " << schedStats.cpu_sec << endl;
  os << "Makespan = " << schedStats.makespan << endl;
  if(schedStats.jobs > 0) {
    tmptr = LocaltimeSafe(&schedStats.start, &tmval);
    bufptr = AsctimeSafe(tmptr, buf, sizeof(buf));
    os << "Started on = " << bufptr;
    tmptr = LocaltimeSafe(&schedStats.end, &tmval);
    bufptr = AsctimeSafe(tmptr, buf, sizeof(buf));
    os << "Finished on = " << bufptr;
    os << "Number of processed jobs = " << schedStats.jobs << endl;
    if(multiSite) {
      os << "+ Number of outsourced jobs = " << schedStats.outsourced_jobs << " (";
      vector<unsigned>::const_iterator itr;
      int id;
      for(id = 0, itr = schedStats.outsourced_jobs_by_shop.begin();
          itr != schedStats.outsourced_jobs_by_shop.end();
          ++itr, ++id) {
        if(id > 0)
          os << ' ';
        if(id != (int)shop_id)
          os << *itr;
        else
          os << '-';
      }
      os << ")" << endl;
      os << "- Number of external jobs = " << schedStats.external_jobs << " (";
      for(id = 0, itr = schedStats.external_jobs_by_shop.begin();
          itr != schedStats.external_jobs_by_shop.end();
          ++itr, ++id) {
        if(id > 0)
          os << ' ';
        if(id != (int)shop_id)
          os << *itr;
        else
          os << '-';
      }
      os << ")" << endl;
      os << "= Number of submitted jobs = " <<
         schedStats.jobs + schedStats.outsourced_jobs - schedStats.external_jobs
         << endl;
    }
    os << "Number of late jobs = " << schedStats.latejobs << endl;
    os << "Number of late jobs (percent) = " << schedStats.latepercent << '%' << endl;
    os << "Average lateness = " << schedStats.avgdelay << endl;
    os << "Std. dev. lateness = " << schedStats.stdevdelay << endl;
    os << "Average lateness (late jobs only) = " << schedStats.avgdelay_lateonly << endl;
    os << "Maximum lateness = " << schedStats.maxdelay << endl;
    os << "Average turnaround time (TAT) = " << schedStats.avg_tat << endl;
    os << "Average processing time = " << schedStats.avg_proc_time << endl;
  } else {
    os << "Started on = N/A" << endl;
    os << "Finished on = N/A" << endl;
    os << "Number of processed jobs = 0" << endl;
    if(multiSite) {
      os << "+ Number of outsourced jobs = 0" << endl;
      os << "- Number of external jobs = 0" << endl;
      os << "= Number of submitted jobs = 0" << endl;
    }
    os << "Number of late jobs = 0" << endl;
    os << "Number of late jobs (percent) = 0%" << endl;
    os << "Average lateness = 0.0" << endl;
    os << "Std. dev. lateness = 0.0" << endl;
    os << "Average lateness (late jobs only) = 0.0" << endl;
    os << "Maximum lateness = 0.0" << endl;
    os << "Average turnaround time (TAT) = 0.0" << endl;
    os << "Average processing time = 0.0" << endl;
  }
}

unsigned GetRsrcsQuantity(vector<string> rsrcs, Rsrc2Qty &rsrc2quantity) {
  vector<string>::const_iterator r;
  unsigned quantity = 0;

  for(r = rsrcs.begin(); r != rsrcs.end(); ++r)
    quantity += rsrc2quantity[*r];
  return quantity;
}

void PrintSched(ostream &os, map<unsigned, Sched> &scheds,
                vector<ShopJob *> &shop_jobs, const char separator) {
  vector<ShopJob *>::const_iterator j;
  Rsrc2Qty r2q;
  unsigned f;
  vector<string> functions, route;
  vector<unsigned> quantities;
  string output_str;

  for(j = shop_jobs.begin(); j != shop_jobs.end(); ++j) {
    os << (*j)->id << separator << (*j)->partid << separator;
    BuildRsrcQuantityMap(r2q, (*j)->resources);
    functions.clear();
    quantities.clear();
    route.clear();
    functions.push_back((*j)->funcseqs[0].name);
    quantities.push_back(GetRsrcsQuantity((*j)->funcseqs[0].outrsrcs, r2q));
    Sched &sched = scheds[(*j)->intid];
    route.push_back(sched[0].step.cell);
    for(f = 1; f < (*j)->funcseqs.size(); ++f) {
      functions.push_back((*j)->funcseqs[f].name);
      quantities.push_back(GetRsrcsQuantity((*j)->funcseqs[f].outrsrcs, r2q));
      route.push_back(sched[f].step.cell);
    }
    VectorToStr(output_str, functions, ",");
    os << output_str;
    os << separator;
    VectorToStr(output_str, quantities, ",");
    os << output_str;
    os << separator << (*j)->numbatches << separator;
    VectorToStr(output_str, route, ",");
    os << output_str;
    os << separator << (*j)->priority;
    os << separator << (*j)->arrival;
    os << separator << (*j)->due;
    os << separator << (*j)->released;
    os << separator << (*j)->completed;
    os << separator << ((*j)->completed - (*j)->due)/3600.0;
    os << endl;
  }
}

void BuildFuncCellMap(map<string, string> &func2cell, const Sched &sched,
                      const One2Many &seq2func) {
  Sched::const_iterator s;
  set<string>::const_iterator f;

  func2cell.clear();
  for(s = sched.begin(); s != sched.end(); ++s) {
    //cerr << "step.funcseq = " << (*s).step.funcseq << endl;
    One2Many::const_iterator it = seq2func.find((*s).step.funcseq);
    if(it != seq2func.end()) {
      for(f = it->second.begin(); f != it->second.end(); ++f) {
        //cerr << "func = " << *f << ", cell = " << (*s).step.cell << endl;
        func2cell[*f] = (*s).step.cell;
      }
    }
  }
}

void PrintFuncSched(ostream &os, map<unsigned, Sched> &scheds,
                    vector<ShopJob *> &shop_jobs,
                    const One2Many &seq2func, const char separator) {
  vector<ShopJob *>::const_iterator j;
  Rsrc2Qty r2q;
  map<string, string> f2c;
  unsigned f;
  vector<string> functions, route;
  vector<unsigned> quantities;
  string output_str;

  map<string, string>::const_iterator fi;
  vector<Func>::const_iterator fs;

  for(j = shop_jobs.begin(); j != shop_jobs.end(); ++j) {
    os << (*j)->id << separator << (*j)->partid << separator;
    BuildRsrcQuantityMap(r2q, (*j)->resources);
    BuildFuncCellMap(f2c, scheds[(*j)->intid], seq2func);
    functions.clear();
    quantities.clear();
    route.clear();
    functions.push_back((*j)->funcsteps[0].name);
    quantities.push_back(GetRsrcsQuantity((*j)->funcsteps[0].outrsrcs, r2q));
    route.push_back(f2c[(*j)->funcsteps[0].name]);
    for(f = 1; f < (*j)->funcsteps.size(); ++f) {
      functions.push_back((*j)->funcsteps[f].name);
      quantities.push_back(GetRsrcsQuantity((*j)->funcsteps[f].outrsrcs, r2q));
      route.push_back(f2c[(*j)->funcsteps[f].name]);
    }
    VectorToStr(output_str, functions, ",");
    os << output_str;
    os << separator;
    VectorToStr(output_str, quantities, ",");
    os << output_str;
    os << separator << (*j)->numbatches << separator;
    VectorToStr(output_str, route, ",");
    os << output_str;
    os << separator << (*j)->priority;
    os << separator << (*j)->arrival;
    os << separator << (*j)->due;
    os << separator << (*j)->released;
    os << separator << (*j)->completed;
    os << separator << ((*j)->completed - (*j)->due)/3600.0;
    os << endl;
  }
}

void WriteScheduleEvents(vector<Job *> &rawjobs, map<unsigned, Sched> &scheds,
                         vector<ShopJob *> &shop_jobs, const ShopInfo &shopInfo,
                         const Rsrc2Tintvl &mach2tintvl,
                         const vector<ShopJob *> &all_job_ptrs) {
  const One2One &machfuncseq2id = shopInfo.machfuncseq2id;
  const map<string, map<string, SimpleFunc> > &station2seq = shopInfo.station2seq;
  vector<ShopJob *>::const_iterator j;
  vector<Job *>::const_iterator rj;
  vector<Step>::iterator p;
  vector<Event>::iterator e;
  map<unsigned, unsigned> stepid2seqid;
  map<unsigned, vector<unsigned> > seqid2schedids;
  //Rsrc2Qty r2q;
  vector<Tintvl>::const_iterator t;
  Event event;
  int quantity;
  vector<int>::const_iterator q;
  time_t step_start, step_end, job_end;
  vector<unsigned>::const_iterator si;
  unsigned stepid;

  for(j = shop_jobs.begin(), rj = rawjobs.begin();
      j != shop_jobs.end();
      ++j, ++rj) {
    Job *rawjob = *rj;
    GetDate(rawjob->released, (*j)->released);
    GetDate(rawjob->completed, (*j)->completed);
    BuildStepId2SeqIdMap(stepid2seqid, (*j)->funcseqs);
    Sched &sched = scheds[(*j)->intid];
    BuildSeqId2SchedIdsMap(seqid2schedids, sched);
    //r2q.clear();
    //BuildRsrcQuantityMap(r2q, (*j)->resources);
    job_end = GetSchedEndTime(sched);
    assert(job_end <= (*j)->completed); // < is possible when job is outsourced
    for(p = rawjob->steps.begin(), stepid = 0; p != rawjob->steps.end(); ++p, ++stepid) {
      (*p).events.clear();
      vector<unsigned> &schedids = seqid2schedids[stepid2seqid[stepid]];
      step_start = numeric_limits<time_t>::max();
      step_end = numeric_limits<time_t>::min();
      //quantity = GetRsrcsQuantity((*p).outputrsrcs, r2q);
      for(si = schedids.begin(); si != schedids.end(); ++si) {
        const SchedStep &step = sched[*si];
        step_start = min(step_start, step.mach_tintvls.front().start);
        step_end = max(step_end, step.mach_tintvls.back().end);
        quantity = GetSchedStepQuantity(step);
        for(t = step.mach_tintvls.begin(), q = step.quantities.begin();
            t != step.mach_tintvls.end();
            ++q) {
          assert(q != step.quantities.end());
          event.jobid = rawjob->jobinfo.jobid;
          //event.station = step.step.station + '-' + step.step.funcseq;
          event.station = step.step.station;
          //event.oper = "any"; //for now
          event.oper = step.step.opr;
          event.quantity = quantity;
          //compute setup time
          const SimpleFunc &sfunc =
            station2seq.find(step.step.station)->second.find(step.step.funcseq)->second;
          event.setup = static_cast<time_t>(sfunc.funcinfo.setuptime);
          //compute variable setup time
          const map<string, time_t> &sfuncAttr = sfunc.attributes;
          const One2One &curAttr = (*j)->funcseqs[stepid2seqid[stepid]].attributes;
          const TintvlSet &machtintvl = mach2tintvl.find(event.station)->second;
          TintvlSet::const_iterator titr = machtintvl.find(step.mach_tintvls.front());
          assert(titr != machtintvl.end());
          if(titr != machtintvl.begin()) {
            --titr;
            const One2One &prevAttr =
              all_job_ptrs[(*titr).intid]->funcseqs[(*titr).seqid].attributes;
            event.varsetup = AttributeSetupTime(sfuncAttr, prevAttr, curAttr);
          } else {
            event.varsetup = AttributeSetupTimeSum(sfuncAttr);
          }
          //convert setup time to milliseconds
          event.setup *= 1000;
          event.varsetup *= 1000;
          //event.funcseqid = machfuncseq2id[event.station];
          One2One::const_iterator seq2idItr
            = machfuncseq2id.find(step.step.station+'-'+step.step.funcseq);
          if(seq2idItr == machfuncseq2id.end()) {
            string errmsg("Unable to find station-function sequence pair: ");
            throw RuntimeException(errmsg + step.step.station + '-' + step.step.funcseq);
          }
          event.funcseqid = seq2idItr->second;
          GetDate(event.timestamp, (*t).start);
          if(t == step.mach_tintvls.begin())
            event.eventname = "Start";
          else
            event.eventname = "Restart";
          (*p).events.push_back(event);
          event.quantity = *q;
          quantity -= *q;
          GetDate(event.timestamp, (*t).end);
          if(++t == step.mach_tintvls.end())
            event.eventname = "Stop";
          else
            event.eventname = "Interrupt";
          (*p).events.push_back(event);
        }
        assert(quantity == 0);
        if(step.mach_tintvls.back().end == job_end) {
          event.eventname = "Completed";
          (*p).events.push_back(event);
        }
      }
      GetDate((*p).released, step_start);
      GetDate((*p).completed, step_end);
    }
  }
}
} // namespace pss
