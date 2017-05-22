/*  pss_utils.cpp
 *
 *  Created by Rong Zhou on 04/15/09.
 *  Copyright 2009 Palo Alto Research Center Inc. All rights reserved.
 *
 *  implementation for shared data types and functions
 */
#include <stdio.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <sstream>
#include <string.h>
#include <functional>
#include <stdarg.h>
#include "pss_utils.hpp"

using namespace std;

namespace pss {

int Vsnprintf(char *str, size_t count, char const *format, va_list args) {
  int len;

  len = vsnprintf(str, count, format, args);
#ifdef _WIN32
  /* Windows does NOT put the null-terminator if strlen(str) >= count */
  if(len < 0 || len == (int)count) {
    str[count - 1] = '\0';
    len = (int)count;
  }
#else
  /* Linux always put the null-terminator in the end, but it may return
   * a value greater than count, if the actual string is longer than 'str'
   */
  if(len > (int)count) len = (int)count;
#endif
  return len;
}

/* exactly same as snprintf() on Linux, but works also on Windows now
 * Note: Windows' native snprintf() implementation is
 *       slightly different from that of Linux
 */
int Snprintf(char *str, size_t count, char const *format, ...) {
  int   len;
  va_list args;

  va_start(args, format);
  len = Vsnprintf(str, count, format, args);
  va_end(args);
  return len;
}

tm *LocaltimeSafe(const time_t *timep, tm *result) {
#ifndef PSS_THREAD_SAFE_TIME_UNAVAILABLE
#ifdef WIN32
  localtime_s(result, timep); //thread-safe in Windows
#else
  localtime_r(timep, result);
#endif
#else
  return localtime(timep);
#endif
  return result;
}

tm *GmtimeSafe(const time_t *timep, tm *result) {
#ifndef PSS_THREAD_SAFE_TIME_UNAVAILABLE
#ifdef WIN32
  gmtime_s(result, timep);
#else
  gmtime_r(timep, result);
#endif
#else
  return gmtime(timep);
#endif
  return result;
}

char *AsctimeSafe(const tm *timeptr, char *buf, unsigned bufsize) {
#ifndef PSS_THREAD_SAFE_TIME_UNAVAILABLE
#ifdef WIN32
  asctime_s(buf, bufsize, timeptr);
#else
  (void)bufsize; //avoid warning
  asctime_r(timeptr, buf);
#endif
#else
  return asctime(timeptr);
#endif
  return buf;
}

int GetTimezoneOffset(void) {
  time_t t = time(NULL);
  tm tm, *when;
  when = LocaltimeSafe(&t, &tm);
  when->tm_isdst = 0;
  time_t local_time = mktime(when);
  when = GmtimeSafe(&t, &tm);
  when->tm_isdst = 0;
  time_t gm_time = mktime(when);
  return (int)(local_time - gm_time) / 3600;
}

time_t GetTime(const Day day, const TimeofDay time) {
  tm when;

  when.tm_year = day.year - 1900;
  when.tm_mon = day.month - 1;
  when.tm_mday = day.day;
  when.tm_hour = time.hour;
  when.tm_min = time.minute;
  when.tm_sec = time.second;
  when.tm_isdst = -1;
  return mktime(&when);
}

void GetDate(Date &date, const time_t time) {
  tm tm, *when;

  //::localtime_r(&time, &when);
  when = LocaltimeSafe(&time, &tm);
  date.day.year = when->tm_year + 1900;
  date.day.month = when->tm_mon + 1;
  date.day.day = when->tm_mday;
  date.time.hour = when->tm_hour;
  date.time.minute = when->tm_min;
  date.time.second = when->tm_sec;
}

void GetStdTmDay(Day &stdtm_day, const Day &day) {
  stdtm_day.year = day.year - 1900;
  stdtm_day.month = day.month - 1;
  stdtm_day.day = day.day;
}

void GetStdTmDay(Day &stdtm_day, const tm *when) {
  stdtm_day.year = when->tm_year;
  stdtm_day.month = when->tm_mon;
  stdtm_day.day = when->tm_mday;
}

void GetStdTmDay(Day &stdtm_day, const time_t time) {
  tm tm, *when;

  //::localtime_r(&time, &when);
  when = LocaltimeSafe(&time, &tm);
  //stdtm_day.year = when.tm_year;
  //stdtm_day.month = when.tm_mon;
  //stdtm_day.day = when.tm_mday;
  GetStdTmDay(stdtm_day, when);
}

void GetEveryYearDay(Day &everyyear_day, const Day &day) {
  everyyear_day.year = numeric_limits<int>::min();
  everyyear_day.month = day.month;
  everyyear_day.day = day.day;
}

time_t GetTimeOffset(const TimeofDay time) {
  time_t local_origin;
  tm when;

  when.tm_year = 100;
  when.tm_mon = 0;
  when.tm_mday = 1;
  when.tm_isdst = 0;
  when.tm_hour = when.tm_min = when.tm_sec = 0;
  local_origin = mktime(&when);

  when.tm_hour = time.hour;
  when.tm_min = time.minute;
  when.tm_sec = time.second;
  return (mktime(&when) - local_origin);
}

void GetTimeStr(string &str, const time_t time) {
  unsigned const buffer_size = 1024;
  char buffer[buffer_size];
  //char const *format = "%b %d, %Y %I:%M:%S %p";
  char const *format = "%H:%M:%S";
  struct tm tm, *when;

  //::localtime_r(&time, &when);
  when = LocaltimeSafe(&time, &tm);
  strftime(buffer, buffer_size, format, when);
  str = buffer;
}

int GetWeekDay(const string &weekday) {
  if(weekday == "Sunday")
    return 0;
  else if(weekday == "Monday")
    return 1;
  else if(weekday == "Tuesday")
    return 2;
  else if(weekday == "Wednesday")
    return 3;
  else if(weekday == "Thursday")
    return 4;
  else if(weekday == "Friday")
    return 5;
  else if(weekday == "Saturday")
    return 6;
  else if(weekday == "Daily")
    return 7;
  else {
    cerr << "Invalid week day: '" << weekday << "'" << endl;
    return 8;
  }
}

int GetYearDay(const Day day) {
  tm when, tm, *noon;
  time_t noontime;

  when.tm_year = day.year - 1900;
  when.tm_mon = day.month - 1;
  when.tm_mday = day.day;
  when.tm_hour = 12;
  when.tm_min = 0;
  when.tm_sec = 0;
  when.tm_isdst = -1;
  noontime = mktime(&when);
  //::localtime_r(&noontime, &noon);
  noon = LocaltimeSafe(&noontime, &tm);
  return noon->tm_yday;
}

struct SchedTmSlot {
  int weekday;
  Day day;
  bool everyyear;
  vector<Tintvl> tmslots;
};

struct TmSlotTranslator : public unary_function<TimeSlot, Tintvl> {
  Tintvl operator()(TimeSlot timeslot) const {
    Tintvl tintvl;

    tintvl.start = GetTimeOffset(timeslot.start);
    tintvl.end = GetTimeOffset(timeslot.stop);
    return tintvl;
  }
};

class SchedTmSlotVisitor : public boost::static_visitor<SchedTmSlot> {
 public:
  SchedTmSlot operator()(const DateSchd &dateschd) {
    SchedTmSlot schd_tmslot;

    schd_tmslot.weekday = -1;
    GetStdTmDay(schd_tmslot.day, dateschd.day);
    schd_tmslot.everyyear = dateschd.everyyear;
    schd_tmslot.tmslots.resize(dateschd.timeslots.size());
    transform(dateschd.timeslots.begin(), dateschd.timeslots.end(),
              schd_tmslot.tmslots.begin(), TmSlotTranslator());
    return schd_tmslot;
  }

  SchedTmSlot operator()(const WeekSchd &weekschd) {
    SchedTmSlot schd_tmslot;

    schd_tmslot.weekday = GetWeekDay(weekschd.weekday);
    //                if (weekschd.timeslots.size() > 1)
    //                    cerr << "More than one time slot is specified for: " << weekschd.weekday << endl;
    schd_tmslot.tmslots.resize(weekschd.timeslots.size());
    transform(weekschd.timeslots.begin(), weekschd.timeslots.end(),
              schd_tmslot.tmslots.begin(), TmSlotTranslator());
    sort(schd_tmslot.tmslots.begin(), schd_tmslot.tmslots.end(), LtTintvl());
    return schd_tmslot;
  }
};

struct WeekDayEqual : public binary_function<SchedTmSlot, int, bool> {
  bool operator()(SchedTmSlot tms, int weekday) const {
    return tms.weekday == weekday;
  }
};

void GetTmslot247(TintvlVec2d &weekts, DayTs &dayts) {
  vector<Tintvl> tmslot_24_7;
  Tintvl tintvl_24;

  memset(&tintvl_24, 0, sizeof(tintvl_24));
  tintvl_24.start = 0;
  tintvl_24.end = 24 * 3600 - 1;
  weekts.clear();
  weekts.resize(7); //number of days in a week
  tmslot_24_7.push_back(tintvl_24);
  fill(weekts.begin(), weekts.end(), tmslot_24_7);
  dayts.once.clear();
  dayts.everyyear.clear();
}

void GetTmslot(TintvlVec2d &weekts, DayTs &dayts, const vector<Schedule> schds) {
  vector<SchedTmSlot> schd_tms;
  vector<SchedTmSlot>::const_iterator i;
  SchedTmSlotVisitor visitor;
  vector<Tintvl> tmslot_default;
  Day everyyear_day;

  schd_tms.resize(schds.size());
  transform(schds.begin(), schds.end(), schd_tms.begin(),
            boost::apply_visitor(visitor));
  weekts.resize(7); //number of days in a week
  //7 means "Daily"
  i = find_if(schd_tms.begin(), schd_tms.end(), bind2nd(WeekDayEqual(), 7));
  if(i != schd_tms.end()) {
    tmslot_default = (*i).tmslots;
    //cerr << "Daily: start = " << tmslot_default.begin()->start;
    //cerr << ", end = " << tmslot_default.rbegin()->end << endl;
  }
  fill(weekts.begin(), weekts.end(), tmslot_default);
  //cerr << "Weekday timeslots:" << endl;
  for(i = schd_tms.begin(); i != schd_tms.end(); ++i) {
    if((*i).weekday >= 0 && (*i).weekday < 7) {
      weekts[(*i).weekday] = (*i).tmslots;
      //cerr << (*i).weekday << ": [" << (*i).tmslot.begin().start << ", " << (*i).tmslot.end().end << "]\n";
    } else if((*i).weekday == -1) { //dateschd
      if(!(*i).everyyear) {
        dayts.once[(*i).day] = (*i).tmslots;
      } else {
        GetEveryYearDay(everyyear_day, (*i).day);
        dayts.everyyear[everyyear_day] = (*i).tmslots;
      }
    }
  }
  // for debugging:
  //            for (int d = 0; d < 7; ++d)
  //            {
  //                if (!weekts[d].empty())
  //                  cerr << d << ": [" << weekts[d].front().start << ", " << weekts[d].back().end << "]\n";
  //                else
  //                    cerr << d << ": [-1, -1]" << endl;
  //            }
  //            for (DayMap::iterator dmi = dayts.once.begin(); dmi != dayts.once.end(); ++dmi)
  //            {
  //                cerr << "Day = " << (*dmi).first.day;
  //                cerr << ", Month = " << (*dmi).first.month + 1;
  //                cerr << ", Year = " << (*dmi).first.year + 1900;
  //                cerr << ", EveryYear = false";
  //                cerr << ", Timeslots =";
  //                for (vector<Tintvl>::iterator ti = (*dmi).second.begin(); ti != (*dmi).second.end(); ++ti)
  //                {
  //                    cerr << " [" << (*ti).start << ", " << (*ti).end << ']';
  //                }
  //                cerr << endl;
  //            }
  //            for (DayMap::iterator dmi = dayts.everyyear.begin(); dmi != dayts.everyyear.end(); ++dmi)
  //            {
  //                cerr << "Day = " << (*dmi).first.day;
  //                cerr << ", Month = " << (*dmi).first.month + 1;
  //                cerr << ", Year = " << (*dmi).first.year + 1900;
  //                cerr << ", EveryYear = true";
  //                cerr << ", Timeslots =";
  //                for (vector<Tintvl>::iterator ti = (*dmi).second.begin(); ti != (*dmi).second.end(); ++ti)
  //                {
  //                    cerr << " [" << (*ti).start << ", " << (*ti).end << ']';
  //                }
  //                cerr << endl;
  //            }
}

struct TintvlGtEq : public binary_function<Tintvl, time_t, bool> {
  bool operator()(Tintvl tintvl, time_t time) const {
    return tintvl.end >= time;
  }
};

//assumes tintvls are ordered chronologically (i.e., earliest intvl first)
//returns false if no such time interval exists in "tintvls"
bool TintvlSince(Tintvl &tintvl, const time_t time, const vector<Tintvl> &tintvls) {
  vector<Tintvl>::const_iterator t;

  t = find_if(tintvls.begin(), tintvls.end(), bind2nd(TintvlGtEq(), time));
  if(t != tintvls.end()) {
    if((*t).start < time)
      tintvl.start = time;
    else
      tintvl.start = (*t).start;
    tintvl.end = (*t).end;
    return true;
  } else {
    tintvl.start = tintvl.end = -1;
    return false;
  }
}

void TintvlShift(Tintvl &tintvl, const time_t delta) {
  tintvl.start += delta;
  tintvl.end += delta;
}

//if true, then "tintvl" stores the earliest time interval since "time"
bool SpecialDay(const Day &stdtm_day, const time_t &time,
                const time_t &daytime, const DayTs &dayts, Tintvl &tintvl) {
  Day everyyear_day;
  DayMap::const_iterator d;

  //GetStdTmDay(stdtm_day, time); supplied by the caller
  d = dayts.once.find(stdtm_day);
  if(d == dayts.once.end()) {
    GetEveryYearDay(everyyear_day, stdtm_day);
    d = dayts.everyyear.find(everyyear_day);
    if(d == dayts.everyyear.end())
      return false;
    else {
      if(TintvlSince(tintvl, daytime, (*d).second))
        TintvlShift(tintvl, time - daytime);
      return true;
    }
  } else {
    if(TintvlSince(tintvl, daytime, (*d).second))
      TintvlShift(tintvl, time - daytime);
    return true;
  }
}

bool IsLeapYear(int stdtm_year) {
  int year = stdtm_year + 1900;

  if(year % 4 == 0) {
    if(year % 100 == 0) {
      if(year % 400 == 0)
        return true;
      return false;
    }
    return true;
  }
  return false;
}

int EndOfStdTmMonthDay(const Day &stdtm_day) {
  switch(stdtm_day.month) {
  case 0: //January
    return 31;
  case 1: //February
    if(IsLeapYear(stdtm_day.year) == false)
      return 28;
    return 29;
  case 2: //March
    return 31;
  case 3: //April
    return 30;
  case 4: //May
    return 31;
  case 5: //June
    return 30;
  case 6: //July
    return 31;
  case 7: //August
    return 31;
  case 8: //September
    return 30;
  case 9: //October
    return 31;
  case 10: //November
    return 30;
  case 11: //December
    return 31;
  default:
    assert(false);
    return -1;
  }
}

void NextStdTmDay(Day &stdtm_day) {
  int end_of_month_day = EndOfStdTmMonthDay(stdtm_day);

  if(stdtm_day.day < end_of_month_day)
    ++stdtm_day.day;
  else {
    assert(stdtm_day.day == end_of_month_day);
    stdtm_day.day = 1;
    if(stdtm_day.month < 11)
      ++stdtm_day.month;
    else {
      stdtm_day.month = 0;
      ++stdtm_day.year;
    }
  }
}

void NextDay(int &weekday, Day &stdtm_day, time_t &time, time_t &daytime) {
  weekday = (weekday + 1) % 7;
  NextStdTmDay(stdtm_day);
  time += 60 * 60 * 24 - daytime;//remaining seconds of a day
  daytime = 0;
}

void EarliestUnitRsrcTintvl(Tintvl &tintvl, time_t &time, time_t &daytime,
                            Day &stdtm_day, const DayTs &dayts, int &weekday,
                            const TintvlVec2d &weekts) {
  for(;;) {
    if(SpecialDay(stdtm_day, time, daytime, dayts, tintvl)) {
      if(tintvl.start >= 0)
        return;
    } else if(TintvlSince(tintvl, daytime, weekts[weekday])) {
      TintvlShift(tintvl, time - daytime);
      return;
    }
    NextDay(weekday, stdtm_day, time, daytime);
  }
}

//finds the earliest _consecutive_ time intvl within [tintvl.start, tintvl.end] s.t. intid <= quota
//returns true if such time intvl exists, and the value of 'tintvl' is updated accordingly
//otherwise it returns false
bool EarliestTintvlWithinQuota(Tintvl &tintvl, const unsigned quota,
                               const TintvlSet &rsrctintvls) {
  TintvlSet::const_iterator i;

  if(rsrctintvls.empty()) return true;
  i = rsrctintvls.upper_bound(tintvl);
  if(i != rsrctintvls.end()) {
    if(i != rsrctintvls.begin()) {
      --i;
      if((*i).end >= tintvl.start && (*i).intid > quota)
        tintvl.start = (*i).end + 1;
      ++i;
    }
    for(; i != rsrctintvls.end(); ++i) {
      if((*i).start <= tintvl.end && (*i).intid > quota) {
        tintvl.end = (*i).start - 1;
        break;
      }
    }
  } else { // i == rsrctintvls.end()
    --i; //go to the last tintvl, since rsrctintvls is not empty
    if((*i).end >= tintvl.start && (*i).intid > quota)
      tintvl.start = (*i).end + 1;
  }
  if(tintvl.start <= tintvl.end)
    return true;
  return false;
}

void EarliestMultiRsrcTintvl(Tintvl &tintvl, time_t &time, time_t &daytime,
                             Day &stdtm_day, const DayTs &dayts, int weekday,
                             const TintvlVec2d &weekts, const int rsrcdemand,
                             const TintvlSet &rsrctintvls) {
  int quota = 100 - rsrcdemand;
  assert(daytime >= 0 && daytime < 86400);
  assert(quota >= 0 && quota <= 100);
  for(;;) {
    if(SpecialDay(stdtm_day, time, daytime, dayts, tintvl)) {
      if(tintvl.start >= 0 && EarliestTintvlWithinQuota(tintvl, quota, rsrctintvls))
        return;
    } else if(TintvlSince(tintvl, daytime, weekts[weekday]) &&
              EarliestTintvlWithinQuota(tintvl, quota, rsrctintvls)) {
      TintvlShift(tintvl, time - daytime);
      return;
    }
    NextDay(weekday, stdtm_day, time, daytime);
  }
}

void EarliestSlot(vector<Tintvl> &tintvls,
                  const unsigned int jobintid,
                  const unsigned int seqid,
                  const bool is_first_batch,
                  const time_t start,
                  const time_t setup0,
                  const time_t setup1,
                  const int quantity,
                  const double unitdur,
                  const TintvlVec2d &weekts,
                  const DayTs &dayts) {
  int weekday, produced;
  time_t est, est_daytime, dur_tintvl, setup, spdur, nextstart;
  Tintvl tintvl;
  Day stdtm_day;
  tm tm, *lctime;

  //::localtime_r(&start, &lctime);
  lctime = LocaltimeSafe(&start, &tm);
  est = start;
  est_daytime = lctime->tm_hour * 3600 + lctime->tm_min * 60 + lctime->tm_sec;
  weekday = lctime->tm_wday;
  GetStdTmDay(stdtm_day, lctime);
  EarliestUnitRsrcTintvl(tintvl, est, est_daytime, stdtm_day,
                         dayts, weekday, weekts);
  assert(tintvl.start >= 0);
  dur_tintvl = tintvl.end - tintvl.start + 1;
  assert(dur_tintvl > 0);
  setup = (is_first_batch || tintvl.start == start) ? setup0 : setup1;
  spdur = setup + static_cast<time_t>(ceil(quantity * unitdur));
  if(dur_tintvl >= spdur) {
    tintvl.end = tintvl.start + spdur - 1;
    tintvl.intid = jobintid;
    tintvl.seqid = seqid;
    tintvls.push_back(tintvl);
  } else {
    time_t setup0next = setup1;
    nextstart = tintvl.end + 1;
    if(dur_tintvl > setup) {
      produced = static_cast<int>(floor((double)(dur_tintvl - setup) / unitdur));
      if(produced > 0) {
        //tintvl.end = tintvl.start + setup + static_cast<time_t>(floor(produced * unitdur + .5)) - 1;
        tintvl.end = tintvl.start + setup +
                     static_cast<time_t>(ceil(produced * unitdur)) - 1;
        tintvl.intid = jobintid;
        tintvl.seqid = seqid;
        tintvls.push_back(tintvl);
      }
    } else {
      produced = 0;
      //conservative assumption: setup time cannot be sub-divided
      setup0next = setup;
    }
    EarliestSlot(tintvls, jobintid, seqid, is_first_batch, nextstart,
                 setup0next, setup1, quantity - produced,
                 unitdur, weekts, dayts);
  }
}

//returns true iff "quantity" @ speed of "unitdur" fits in [start, end)
//NOTE: end is the start time of the next job already scheduled
//output parameter: actualEnd = the earliest time a future next job can start
//NOTE: actualEnd is 1 second past the end time of the job being tested
bool QuantityTest(const int quantity, const double unitdur,
                  const time_t setup0, const time_t setup1,
                  const bool is_first_batch, const time_t start,
                  const time_t end, const TintvlVec2d &weekts,
                  const DayTs &dayts, time_t &actualEnd) {
  int weekday, produced;
  time_t est, est_daytime, dur_tintvl, setup, spdur;
  Tintvl tintvl;
  Day stdtm_day;
  tm tm, *lctime;

  //::localtime_r(&start, &lctime);
  lctime = LocaltimeSafe(&start, &tm);
  assert(quantity >= 0);
  est = start;
  est_daytime = lctime->tm_hour * 3600 + lctime->tm_min * 60 + lctime->tm_sec;
  weekday = lctime->tm_wday;
  GetStdTmDay(stdtm_day, lctime);
  EarliestUnitRsrcTintvl(tintvl, est, est_daytime, stdtm_day,
                         dayts, weekday, weekts);
  assert(tintvl.start >= 0);
  if(tintvl.start >= end) {  //end is the start time of the next job!
    actualEnd = numeric_limits<time_t>::max();
    return false;
  }
  dur_tintvl = tintvl.end - tintvl.start + 1;
  assert(dur_tintvl > 0);
  setup = (is_first_batch || tintvl.start == start)? setup0 : setup1;
  spdur = setup + static_cast<time_t>(ceil(quantity * unitdur));
  if(dur_tintvl >= spdur) {
    if(tintvl.start + spdur <= end) {  //end can be the start of another job
      actualEnd = tintvl.start + spdur;
      return true;
    }
    actualEnd = numeric_limits<time_t>::max();
    return false;
  } else {
    time_t setup0next = setup1;
    if(tintvl.end >= end) {
      actualEnd = numeric_limits<time_t>::max();
      return false;
    }
    if(dur_tintvl > setup)
      produced = static_cast<int>(floor((double)(dur_tintvl - setup) / unitdur));
    else {
      produced = 0;
      //conservative assumption: setup time cannot be sub-divided
      setup0next = setup;
    }
    return QuantityTest(quantity - produced, unitdur, setup0next, setup1,
                        is_first_batch, tintvl.end + 1, end, weekts,
                        dayts, actualEnd);
  }
}

//computes the corresponding day time of "time", given a reference time "timeref"
//and its corresponding day time "daytimeref"
time_t DayTimeByRef(time_t time, time_t timeref, time_t daytimeref) {
  if(time >= timeref)
    return (time - timeref + daytimeref) % 86400;
  else {
    time_t timediff = (timeref - time) % 86400;
    return (daytimeref - timediff + 86400) % 86400;
  }
}

void EarliestSlotOprltd(vector<Tintvl> &machsteptintvls,
                        vector<Tintvl> &oprsteptintvls,
                        const unsigned int jobintid,
                        const unsigned int seqid,
                        const bool is_first_batch,
                        const time_t start,
                        const time_t setup0,
                        const time_t setup1,
                        const int quantity,
                        const double unitdur,
                        const TintvlVec2d &machweekts,
                        const DayTs &machdayts,
                        const int oprdemand,
                        TintvlSet &oprtintvls,
                        const TintvlVec2d &oprweekts,
                        const DayTs &oprdayts) {
  int weekday, produced;
  time_t est, est_daytime, setup, spdur, oprprodur, machend, machdur;
  Tintvl mach_tintvl, opr_tintvl;
  Day stdtm_day;
  tm tm, *lctime;

  //::localtime_r(&start, &lctime);
  lctime = LocaltimeSafe(&start, &tm);
  est = start;
  est_daytime = lctime->tm_hour * 3600 + lctime->tm_min * 60 + lctime->tm_sec;
  weekday = lctime->tm_wday;
  GetStdTmDay(stdtm_day, lctime);
  EarliestUnitRsrcTintvl(mach_tintvl, est, est_daytime, stdtm_day,
                         machdayts, weekday, machweekts);
  assert(mach_tintvl.start >= 0);

  est_daytime = DayTimeByRef(mach_tintvl.start, est, est_daytime);
  est = mach_tintvl.start; //must follow assignment of est_daytime
  EarliestMultiRsrcTintvl(opr_tintvl, est, est_daytime, stdtm_day,
                          oprdayts, weekday, oprweekts, oprdemand, oprtintvls);
  assert(opr_tintvl.start >= mach_tintvl.start);

  setup = (is_first_batch || opr_tintvl.start == start) ? setup0 : setup1;
  spdur = setup + static_cast<time_t>(ceil(quantity * unitdur));
  oprprodur = opr_tintvl.end - opr_tintvl.start + 1 - setup; //duration minus setup = productive duration or produr
  if(oprprodur > 0) {
    machend = opr_tintvl.start + setup - 1 +
              static_cast<time_t>(floor(oprprodur * 2.)); //2. a parameter in future
    if(machend > mach_tintvl.end)
      machend = mach_tintvl.end;
    machdur = machend - opr_tintvl.start + 1;
  } else {
    machend = opr_tintvl.end;
    machdur = 0; //not enough setup time for opr
  }
  if(machdur >= spdur) {
    mach_tintvl.start = opr_tintvl.start;
    mach_tintvl.end = mach_tintvl.start + spdur - 1;
    mach_tintvl.intid = jobintid;
    mach_tintvl.seqid = seqid;
    machsteptintvls.push_back(mach_tintvl);
    //.5 a parameter in future
    opr_tintvl.end = opr_tintvl.start + setup +
                     static_cast<time_t>(ceil(quantity * unitdur * .5)) - 1;
    opr_tintvl.intid = oprdemand;
    oprsteptintvls.push_back(opr_tintvl);
  } else {
    time_t setup0next = setup1;
    if(machdur > setup) {
      produced = static_cast<int>(floor((double)(machdur - setup) / unitdur));
      if(produced > 0) {
        mach_tintvl.start = opr_tintvl.start;
        mach_tintvl.end = mach_tintvl.start + setup +
                          static_cast<time_t>(ceil(produced * unitdur)) - 1;
        mach_tintvl.intid = jobintid;
        mach_tintvl.seqid = seqid;
        machsteptintvls.push_back(mach_tintvl);
        //.5 a parameter in future
        opr_tintvl.end = opr_tintvl.start + setup +
                         static_cast<time_t>(ceil(produced * unitdur * .5)) - 1;
        opr_tintvl.intid = oprdemand;
        oprsteptintvls.push_back(opr_tintvl);
      }
    } else {
      produced = 0;
      //conservative assumption: setup time cannot be sub-divided
      setup0next = setup;
    }
    EarliestSlotOprltd(machsteptintvls, oprsteptintvls, jobintid, seqid,
                       is_first_batch, machend + 1, setup0next, setup1,
                       quantity - produced, unitdur, machweekts, machdayts,
                       oprdemand, oprtintvls, oprweekts, oprdayts);
  }
}

//test if "quantity" @ speed of "unitdur" can fit in ["start", "end")
//NOTE: end is the start time of the next job
//output parameter: actualEnd = the earliest time a future next job can start
//NOTE: actualEnd is 1 second past the end time of the job being tested
bool QuantityTestOprltd(const int quantity, const double unitdur,
                        const time_t setup0, const time_t setup1,
                        const bool is_first_batch, const time_t start,
                        const time_t end, const TintvlVec2d &machweekts,
                        const DayTs &machdayts, const int oprdemand,
                        const TintvlSet &oprtintvls,
                        const TintvlVec2d &oprweekts, const DayTs &oprdayts,
                        time_t &actualEnd) {
  int weekday, produced;
  time_t est, est_daytime, setup, spdur, oprprodur, machend, machdur;
  Tintvl mach_tintvl, opr_tintvl;
  Day stdtm_day;
  tm tm, *lctime;

  //::localtime_r(&start, &lctime);
  lctime = LocaltimeSafe(&start, &tm);
  assert(quantity >= 0);
  est = start;
  est_daytime = lctime->tm_hour * 3600 + lctime->tm_min * 60 + lctime->tm_sec;
  weekday = lctime->tm_wday;
  GetStdTmDay(stdtm_day, lctime);
  EarliestUnitRsrcTintvl(mach_tintvl, est, est_daytime, stdtm_day,
                         machdayts, weekday, machweekts);
  assert(mach_tintvl.start >= 0);
  if(mach_tintvl.start >= end) {  //end is the start time of the next job!
    actualEnd = numeric_limits<time_t>::max();
    return false;
  }

  est_daytime = DayTimeByRef(mach_tintvl.start, est, est_daytime);
  est = mach_tintvl.start; //must follow assignment of est_daytime
  EarliestMultiRsrcTintvl(opr_tintvl, est, est_daytime, stdtm_day,
                          oprdayts, weekday, oprweekts, oprdemand, oprtintvls);
  assert(opr_tintvl.start >= mach_tintvl.start);
  if(opr_tintvl.start >= end) {
    actualEnd = numeric_limits<time_t>::max();
    return false;
  }

  setup = (is_first_batch || opr_tintvl.start == start) ? setup0 : setup1;
  spdur = setup + static_cast<time_t>(ceil(quantity * unitdur));
  oprprodur = opr_tintvl.end - opr_tintvl.start + 1 - setup; //duration minus setup = productive duration or produr
  if(oprprodur > 0) {
    //2. a parameter in future
    machend = opr_tintvl.start + setup - 1 +
              static_cast<time_t>(floor(oprprodur * 2.));
    if(machend > mach_tintvl.end)
      machend = mach_tintvl.end;
    machdur = machend - opr_tintvl.start + 1;
  } else {
    machend = opr_tintvl.end;
    machdur = 0; //not enough setup time for opr
  }
  if(machdur >= spdur) {
    if(opr_tintvl.start + spdur <= end) {//end can be the start of another job
      actualEnd = opr_tintvl.start + spdur;
      return true;
    }
    actualEnd = numeric_limits<time_t>::max();
    return false;
  } else {
    time_t setup0next = setup1;
    if(machend >= end) {
      actualEnd = numeric_limits<time_t>::max();
      return false;
    }
    if(machdur > setup) {
      produced = static_cast<int>(floor((double)(machdur - setup) / unitdur));
    } else {
      produced = 0;
      //conservative assumption: setup time cannot be sub-divided
      setup0next = setup;
    }
    return QuantityTestOprltd(quantity - produced, unitdur, setup0next,
                              setup1, is_first_batch, machend + 1, end,
                              machweekts, machdayts, oprdemand, oprtintvls,
                              oprweekts, oprdayts, actualEnd);
  }
}

bool UninterruptedTimespan(const time_t start, const time_t end,
                           const TintvlVec2d &weekts, const DayTs &dayts) {
  int weekday;
  time_t startime, daytime;
  Tintvl tintvl;
  Day stdtm_day;
  tm tm, *lctime;

  //::localtime_r(&start, &lctime);
  lctime = LocaltimeSafe(&start, &tm);
  assert(start <= end);
  startime = start;
  daytime = lctime->tm_hour * 3600 + lctime->tm_min * 60 + lctime->tm_sec;
  weekday = lctime->tm_wday;
  GetStdTmDay(stdtm_day, lctime);
  EarliestUnitRsrcTintvl(tintvl, startime, daytime, stdtm_day,
                         dayts, weekday, weekts);
  assert(tintvl.start >= 0);
  if(tintvl.start != start)
    return false;
  if(tintvl.end >= end)
    return true;
  return UninterruptedTimespan(tintvl.end + 1, end, weekts, dayts);
}

time_t timespan(const time_t start, const time_t end,
                const TintvlVec2d &weekts, const DayTs &dayts) {
  int weekday;
  time_t startime, daytime, dur_tintvl;
  Tintvl tintvl;
  Day stdtm_day;
  tm tm, *lctime;

  //::localtime_r(&start, &lctime);
  lctime = LocaltimeSafe(&start, &tm);
  if(start == end) return 1;
  assert(start < end);
  startime = start;
  daytime = lctime->tm_hour * 3600 + lctime->tm_min * 60 + lctime->tm_sec;
  weekday = lctime->tm_wday;
  GetStdTmDay(stdtm_day, lctime);
  EarliestUnitRsrcTintvl(tintvl, startime, daytime, stdtm_day,
                         dayts, weekday, weekts);
  assert(tintvl.start >= 0);
  if(tintvl.end < end) {
    dur_tintvl = tintvl.end - tintvl.start + 1;
    return (dur_tintvl + timespan(tintvl.end + 1, end, weekts, dayts));
  } else
    return (end - tintvl.start + 1);
}

void VectorToStr(string &str, const vector<int> &vec, const char *separator) {
  vector<int>::const_iterator i;
  stringstream out;

  if(!vec.empty()) {
    out << vec.front();
    i = vec.begin();
    for(++i; i != vec.end(); ++i) {
      out << separator;
      out << *i;
    }
    str = out.str();
  }
}

void VectorToStr(string &str, const vector<unsigned> &vec, const char *separator) {
  vector<unsigned>::const_iterator i;
  stringstream out;

  if(!vec.empty()) {
    out << vec.front();
    i = vec.begin();
    for(++i; i != vec.end(); ++i) {
      out << separator;
      out << *i;
    }
    str = out.str();
  }
}

void VectorToStr(string &str, const vector<string> &vec, const char *separator) {
  vector<string>::const_iterator i;
  stringstream out;

  if(!vec.empty()) {
    out << vec.front();
    i = vec.begin();
    for(++i; i != vec.end(); ++i) {
      out << separator;
      out << *i;
    }
    str = out.str();
  }
}

void SetToStr(string &str, const set<string> &set, const char *separator) {
  std::set<string>::const_iterator i;
  stringstream out;

  if(!set.empty()) {
    i = set.begin();
    out << (*i);
    for(++i; i != set.end(); ++i) {
      out << separator;
      out << *i;
    }
    str = out.str();
  }
}

void TranslateHtmlStr(string &str) {
  static const char *wordsTranslatedFrom[] = {"&apos;"};
  static const char *wordsTranslatedInto[] = {"'"};
  assert(sizeof(wordsTranslatedFrom) == sizeof(wordsTranslatedInto));
  static unsigned numOfWords = sizeof(wordsTranslatedFrom) / sizeof(char *);
  bool found = true;
  while(found) {
    found = false;
    for(size_t w = 0; w < numOfWords; ++w) {
      size_t pos = str.find(wordsTranslatedFrom[w]);
      if(pos != string::npos) {
        found = true;
        str.replace(pos, strlen(wordsTranslatedFrom[w]), wordsTranslatedInto[w]);
      }
    }
  }
}

/* NOTE: "seqid" field is unspecified in a "resource" tintvl */
void InsertResourceTintvl(TintvlSet &set, unsigned intid, time_t start, time_t end) {
  if(intid > 0) {
    Tintvl tintvl;
    tintvl.intid = intid;
    tintvl.seqid = numeric_limits<unsigned>::max();
    assert(start >= 0 && end >= 0);
    assert(start <= end);
    tintvl.start = start;
    tintvl.end = end;
    set.insert(tintvl);
  }
}

void AddTimePointUsage(map<time_t, int> &time2usage, const TintvlSet &tintvls) {
  TintvlSet::const_iterator i;

  for(i = tintvls.begin(); i != tintvls.end(); ++i) {
    time2usage[(*i).start] += (*i).intid;
    //time2usage[(*i).end] -= (*i).intid;
    time2usage[(*i).end + 1] -= (*i).intid;
  }
}

//adding two TintvlSet means dividing into finer-grained intervals
//s.t. the "intid" of each interval is the same
//e.g., adding {intid = 1, start = 0, end = 10} and {intid = 2, start = 5, end = 8}
//      =  {1, 0, 4}, {3, 5, 8}, and {1, 9, 10}
void TintvlSetAdd(const TintvlSet &set1, const TintvlSet &set2, TintvlSet &result) {
  int intid;
  time_t prev_start;
  map<time_t, int> time2usage;
  map<time_t, int>::const_iterator t;

  intid = 0;
  prev_start = -1;
  AddTimePointUsage(time2usage, set1);
  AddTimePointUsage(time2usage, set2);
  result.clear(); //must follow AddTimePointUsage() to allow set1 = result
  for(t = time2usage.begin(); t != time2usage.end(); ++t) {
    if(prev_start != -1) {
      InsertResourceTintvl(result, intid, prev_start, t->first - 1);
      prev_start = t->first;
      intid += t->second;
    } else {
      prev_start = t->first;
      intid = t->second;
    }
  }
}

void AddTimePointUsage(map<time_t, int> &time2usage, const vector<Tintvl> &tintvls) {
  vector<Tintvl>::const_iterator i;

  for(i = tintvls.begin(); i != tintvls.end(); ++i) {
    time2usage[(*i).start] += (*i).intid;
    time2usage[(*i).end + 1] -= (*i).intid;
  }
}

//adding two TintvlSet means dividing into finer-grained intervals
//s.t. the "intid" of each interval is the same
//e.g., adding {intid = 1, start = 0, end = 10} and {intid = 2, start = 5, end = 8}
//      =  {1, 0, 4}, {3, 5, 8}, and {1, 9, 10}
void TintvlSetAdd(const TintvlSet &set1, const vector<Tintvl> &set2, TintvlSet &result) {
  int intid;
  time_t prev_start;
  map<time_t, int> time2usage;
  map<time_t, int>::const_iterator t;

  intid = 0;
  prev_start = -1;
  AddTimePointUsage(time2usage, set1);
  AddTimePointUsage(time2usage, set2);
  result.clear(); //must follow AddTimePointUsage() to allow set1 = result
  for(t = time2usage.begin(); t != time2usage.end(); ++t) {
    if(prev_start != -1) {
      InsertResourceTintvl(result, intid, prev_start, t->first - 1);
      prev_start = t->first;
      intid += t->second;
    } else {
      prev_start = t->first;
      intid = t->second;
    }
  }
}

//NOTE: set is modified to be set = set - projset
void tintvl_set_project_range(TintvlSet &set, const time_t min_start,
                              const time_t max_end, TintvlSet &projset) {
  TintvlSet::iterator min_itr, max_itr;
  Tintvl tintvl;

  tintvl.start = min_start;
  min_itr = set.lower_bound(tintvl);
  if(min_itr == set.end()) {
    if(min_itr != set.begin()) {
      --min_itr;
      if(min_itr->end < min_start)
        return;
    } else //"set" is empty
      return;
    assert(min_itr->start < min_start);
    assert(min_itr->end >= min_start);
  } else if(min_itr->start != min_start) {
    assert(min_itr->start > min_start);
    if(min_itr != set.begin()) {
      --min_itr;
      assert(min_itr->start < min_start);
      if(min_itr->end < min_start)
        ++min_itr;
    } else if(min_itr->start > max_end)
      return;
    assert(min_itr->end >= min_start);
  }
  assert(!set.empty());
  tintvl.start = max_end;
  max_itr = set.upper_bound(tintvl);
  assert(max_itr != set.begin());
  projset.insert(min_itr, max_itr);
  set.erase(min_itr, max_itr);
}

//adding two TintvlSet means dividing into finer-grained intervals
//s.t. the "intid" of each interval is the same
//e.g., adding {intid = 1, start = 0, end = 10} and {intid = 2, start = 5, end = 8}
//      =  {1, 0, 4}, {3, 5, 8}, and {1, 9, 10}
//ASSUMPTION: set2 is smaller than set1 --> it's more efficient to project set1 onto set2 first
//Results are stored back to set1
void TintvlSetAdd(TintvlSet &set1, const vector<Tintvl> &set2) {
  int intid;
  time_t min_start, max_end, prev_start;
  map<time_t, int> time2usage;
  map<time_t, int>::const_iterator t;
  TintvlSet set1proj;

  intid = 0;
  prev_start = -1;
  min_start = set2.begin()->start;
  max_end = set2.rbegin()->end;
  assert(min_start <= max_end);
  tintvl_set_project_range(set1, min_start, max_end, set1proj);
  AddTimePointUsage(time2usage, set1proj);
  AddTimePointUsage(time2usage, set2);
  for(t = time2usage.begin(); t != time2usage.end(); ++t) {
    if(prev_start != -1) {
      InsertResourceTintvl(set1, intid, prev_start, t->first - 1);
      prev_start = t->first;
      intid += t->second;
    } else {
      prev_start = t->first;
      intid = t->second;
    }
  }
}

void SubtractTimePointUsage(map<time_t, int> &time2usage, const TintvlSet &tintvls) {
  TintvlSet::const_iterator i;

  for(i = tintvls.begin(); i != tintvls.end(); ++i) {
    time2usage[(*i).start] -= (*i).intid;
    time2usage[(*i).end + 1] += (*i).intid;
  }
}

//subtract set2 from set1
//s.t. the "intid" of each interval is the same
//e.g., subtract {intid = 1, start = 0, end = 10} from {1, 0, 4}, {3, 5, 8}, and {1, 9, 10}
//      = {intid = 2, start = 5, end = 8}
void TintvlSetSubtract(const TintvlSet &set1, const TintvlSet &set2, TintvlSet &result) {
  int intid;
  time_t prev_start;
  map<time_t, int> time2usage;
  map<time_t, int>::const_iterator t;

  intid = 0;
  prev_start = -1;
  AddTimePointUsage(time2usage, set1);
  SubtractTimePointUsage(time2usage, set2);
  result.clear(); //must follow AddTimePointUsage() to allow set1 = result
  for(t = time2usage.begin(); t != time2usage.end(); ++t) {
    if(prev_start != -1) {
      InsertResourceTintvl(result, intid, prev_start, t->first - 1);
      prev_start = t->first;
      intid += t->second;
    } else {
      prev_start = t->first;
      intid = t->second;
    }
  }
}

void SubtractTimePointUsage(map<time_t, int> &time2usage, const vector<Tintvl> &tintvls) {
  vector<Tintvl>::const_iterator i;

  for(i = tintvls.begin(); i != tintvls.end(); ++i) {
    time2usage[(*i).start] -= (*i).intid;
    time2usage[(*i).end + 1] += (*i).intid;
  }
}

//subtract set2 from set1
//s.t. the "intid" of each interval is the same
//e.g., subtract {intid = 1, start = 0, end = 10} from {1, 0, 4}, {3, 5, 8}, and {1, 9, 10}
//      = {intid = 2, start = 5, end = 8}
void TintvlSetSubtract(const TintvlSet &set1, const vector<Tintvl> &set2, TintvlSet &result) {
  int intid;
  time_t prev_start;
  map<time_t, int> time2usage;
  map<time_t, int>::const_iterator t;

  intid = 0;
  prev_start = -1;
  AddTimePointUsage(time2usage, set1);
  SubtractTimePointUsage(time2usage, set2);
  result.clear(); //must follow AddTimePointUsage() to allow set1 = result
  for(t = time2usage.begin(); t != time2usage.end(); ++t) {
    if(prev_start != -1) {
      InsertResourceTintvl(result, intid, prev_start, t->first - 1);
      prev_start = t->first;
      intid += t->second;
    } else {
      prev_start = t->first;
      intid = t->second;
    }
  }
}

//subtract set2 from set1
//s.t. the "intid" of each interval is the same
//e.g., subtract {intid = 1, start = 0, end = 10} from {1, 0, 4}, {3, 5, 8}, and {1, 9, 10}
//      = {intid = 2, start = 5, end = 8}
//ASSUMPTION: set2 is smaller than set1 --> it's more efficient to project set1 onto set2 first
//Results are stored back to set1
void TintvlSetSubtract(TintvlSet &set1, const vector<Tintvl> &set2) {
  int intid;
  time_t min_start, max_end, prev_start;
  map<time_t, int> time2usage;
  map<time_t, int>::const_iterator t;
  TintvlSet set1proj;

  intid = 0;
  prev_start = -1;
  min_start = set2.begin()->start;
  max_end = set2.rbegin()->end;
  assert(min_start <= max_end);
  tintvl_set_project_range(set1, min_start, max_end, set1proj);
  AddTimePointUsage(time2usage, set1proj);
  SubtractTimePointUsage(time2usage, set2);
  for(t = time2usage.begin(); t != time2usage.end(); ++t) {
    if(prev_start != -1) {
      InsertResourceTintvl(set1, intid, prev_start, t->first - 1);
      prev_start = t->first;
      intid += t->second;
    } else {
      prev_start = t->first;
      intid = t->second;
    }
  }
}

void TintvlSetSimplify(TintvlSet &tintvls) {
  TintvlSet::iterator t, t0;
  Tintvl simplified;
  bool simplify;

  simplify = false;
  for(t = t0 = tintvls.begin(); t != tintvls.end();) {
    if((*t).intid == (*t0).intid && (*t).seqid == (*t0).seqid &&
        ((!simplify && (*t).start == (*t0).end + 1) ||
         (simplify && (*t).start == simplified.end + 1))) {
      //(*t0).end = (*t).end; cannot do this with set
      if(!simplify) {
        simplify = true;
        simplified.intid = (*t0).intid;
        simplified.start = (*t0).start;
      }
      simplified.end = (*t).end;
      tintvls.erase(t++);
    } else {
      if(simplify) {
        tintvls.erase(t0);
        tintvls.insert(simplified);
        simplify = false;
      }
      t0 = t;
      ++t;
    }
  }
  if(simplify) {
    tintvls.erase(t0);
    tintvls.insert(simplified);
  }
}

void TintvlSetSimplify(vector<Tintvl> &tintvls) {
  size_t i, j, size = tintvls.size();

  for(i = 0, j = 1; j < size; ++j) {
    if(tintvls[j].intid == tintvls[i].intid && tintvls[j].seqid == tintvls[i].seqid
        && tintvls[j].start == tintvls[i].end + 1) {
      tintvls[i].end = tintvls[j].end;
    } else if(++i != j) {
      tintvls[i] = tintvls[j];
    }
  }
  tintvls.resize(i + 1);
}

}
