/*  pss_utils.hpp
 *
 *  Created by Rong Zhou on 04/15/09.
 *  Copyright 2009-16 Palo Alto Research Center Inc. All rights reserved.
 *
 *  header file for utility data types and functions
 */

#ifndef PSS_UTILS_HPP_INCLUDED_
#define PSS_UTILS_HPP_INCLUDED_

#include <stdarg.h>
#include <set>
#include <time.h>
#include "pss_parser.hpp"

namespace pss {

struct Tintvl {
  //internal id; useful for (1) encoding job id or
  //(2) % rsrc reserved for multi-capacity rsrcs
  unsigned int intid;
  //sequence id: index into ShopJob.funcseqs;
  //useful for attribute-based setup time calculation
  unsigned int seqid;
  time_t start;
  time_t end;
};

struct LtTintvl {
  bool operator()(const Tintvl &tintvl1, const Tintvl &tintvl2) const {
    return tintvl1.start < tintvl2.start;
  }
};

typedef std::set<Tintvl, LtTintvl> TintvlSet;

struct LtDay {
  bool operator()(const Day &day1, const Day &day2) const {
    if(day1.year < day2.year)
      return true;
    if(day1.year > day2.year)
      return false;
    if(day1.month < day2.month)
      return true;
    if(day1.month > day2.month)
      return false;
    if(day1.day < day2.day)
      return true;
    return false;
  }
};

//tintvls must be sorted
typedef std::map<Day, std::vector<Tintvl>, LtDay> DayMap;

struct DayTs {
  DayMap once;
  DayMap everyyear;
};

typedef std::vector<std::vector<Tintvl> > TintvlVec2d;

typedef std::map<std::string, std::string> One2One;

typedef std::map<std::string, std::set<std::string> > One2Many;

typedef std::map<std::string, unsigned> Rsrc2Qty;

int Vsnprintf(char *str, size_t count, char const *format, va_list args);

/* exactly same as snprintf() on Linux, but works also on Windows now
 * Note: Windows' native snprintf() implementation is
 *       slightly different from that of Linux
 */
int Snprintf(char *str, size_t count, char const *format, ...);

int GetTimezoneOffset(void);

time_t GetTime(const Day day, const TimeofDay time);

void GetDate(Date &date, const time_t time);

time_t GetTimeOffset(const TimeofDay time);

void GetTimeStr(std::string &str, const time_t time);

void GetTmslot(TintvlVec2d &weekts, DayTs &dayts,
               const std::vector<Schedule> schds);

void GetTmslot247(TintvlVec2d &weekts, DayTs &dayts);

tm *LocaltimeSafe(const time_t *timep, tm *result);

tm *GmtimeSafe(const time_t *timep, tm *result);

char *AsctimeSafe(const tm *timeptr, char *buf, unsigned bufsize);

bool QuantityTest(const int quantity, const double unitdur, const time_t setup0,
                  const time_t setup1, const bool is_first_batch,
                  const time_t start, const time_t end,
                  const TintvlVec2d &weekts, const DayTs &dayts,
                  time_t &actualEnd);

void EarliestSlot(std::vector<Tintvl> &tintvls,
                  const unsigned jobintid,
                  const unsigned seqid,
                  const bool is_first_batch,
                  const time_t start,
                  const time_t setup0,
                  const time_t setup1,
                  const int quantity,
                  const double unitdur,
                  const TintvlVec2d &weekts,
                  const DayTs &dayts);

bool QuantityTestOprltd(const int quantity, const double unitdur,
                        const time_t setup0,  const time_t setup1,
                        const bool is_first_batch, const time_t start,
                        const time_t end, const TintvlVec2d &machweekts,
                        const DayTs &machdayts, const int oprdemand,
                        const TintvlSet &oprtintvls,
                        const TintvlVec2d &oprweekts,
                        const DayTs &oprdayts, time_t &actualEnd);

void EarliestSlotOprltd(std::vector<Tintvl> &machsteptintvls,
                        std::vector<Tintvl> &oprsteptintvls,
                        const unsigned jobintid,
                        const unsigned seqid,
                        const bool is_first_batch,
                        const time_t start,
                        time_t setup0,
                        const time_t setup1,
                        const int quantity,
                        const double unitdur,
                        const TintvlVec2d &machweekts,
                        const DayTs &machdayts,
                        const int oprdemand,
                        TintvlSet &oprtintvls,
                        const TintvlVec2d &oprweekts,
                        const DayTs &oprdayts);

time_t timespan(const time_t start, const time_t end, const TintvlVec2d &weekts,
                const DayTs &dayts);

bool UninterruptedTimespan(const time_t start, const time_t end,
                           const TintvlVec2d &weekts, const DayTs &dayts);

void VectorToStr(std::string &str, const std::vector<int> &vector,
                 const char *separator);

void VectorToStr(std::string &str, const std::vector<unsigned> &vector,
                 const char *separator);

void VectorToStr(std::string &str, const std::vector<std::string> &vector,
                 const char *separator);

void SetToStr(std::string &str, const std::set<std::string> &set,
              const char *separator);

void TranslateHtmlStr(std::string &str);

void TintvlSetAdd(const TintvlSet &set1, const TintvlSet &set2,
                  TintvlSet &result);

void TintvlSetAdd(const TintvlSet &set1, const std::vector<Tintvl> &set2,
                  TintvlSet &result);

void TintvlSetAdd(TintvlSet &set1, const std::vector<Tintvl> &set2);

void TintvlSetSubtract(const TintvlSet &set1, const TintvlSet &set2,
                       TintvlSet &result);

void TintvlSetSubtract(const TintvlSet &set1, const std::vector<Tintvl> &set2,
                       TintvlSet &result);

void TintvlSetSubtract(TintvlSet &set1, const std::vector<Tintvl> &set2);

void TintvlSetSimplify(TintvlSet &tintvls);

void TintvlSetSimplify(std::vector<Tintvl> &tintvls);

}

#endif // PSS_UTILS_HPP_INCLUDED_
