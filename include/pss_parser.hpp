/*  pss_parser.hpp
 *
 *  Created by Rong Zhou on 04/15/09.
 *  Copyright 2009 Palo Alto Research Center Inc. All rights reserved.
 *
 *  header file for shared data types for shop and job parsers
 */

#ifndef PSS_PARSER_HPP_INCLUDED_
#define PSS_PARSER_HPP_INCLUDED_

#include <vector>
#include <map>
#include <boost/variant.hpp>

namespace pss {

struct Attribute {
  std::string name;
  std::string value;
};

struct Day {
  int day;
  int month;
  int year;
};

struct Scope {
  pss::Day start;
  pss::Day stop;
};

struct TimeofDay {
  int hour;
  int minute;
  int second;
};

struct Date {
  pss::Day day;
  pss::TimeofDay time;
};

struct Quantity {
  int value;
  std::string unit;
};

struct BaseInfo {
  std::string name;
  std::string type;
  int status;
  std::string comment;
};

struct Dimension {
  double value;
  std::string unit;
};

struct TwoDims {
  pss::Dimension x;
  pss::Dimension y;
};

struct Geometry {
  pss::TwoDims footprint;
  pss::TwoDims position;
  int orientation;
};

struct TimeSlot {
  pss::TimeofDay start;
  pss::TimeofDay stop;
};

struct DateSchd {
  bool everyyear;
  pss::Day day;
  std::vector<pss::TimeSlot> timeslots;
};

struct WeekSchd {
  std::string weekday;
  pss::Scope scope; //all zeros mean unlimited scope
  std::vector<pss::TimeSlot> timeslots;
};

typedef boost::variant<pss::DateSchd, pss::WeekSchd> Schedule;

}

#endif // PSS_PARSER_HPP_INCLUDED_
