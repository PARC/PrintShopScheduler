#include <boost/algorithm/string.hpp>
#include "pss_parser_utils.hpp"

using namespace std;
using namespace rapidxml;

namespace pss {

void ParseBoolAttr(xml_node<> *node, char const *name, bool &attr) {
  //iequals does case insensitive string comparison
  attr = boost::iequals(node->first_attribute(name)->value(), "true");
}

void ParseStrAttr(xml_node<> *node, char const *name, string &attr) {
  xml_attribute<> *attribute = node->first_attribute(name);
  if(attribute)
    attr = node->first_attribute(name)->value();
  //printf("%s\n", node->first_attribute(name)->value());
  //attr = node->first_attribute(name)->value();
  //cout << "str = " << attr << endl;
}

void ParseNameAttr(xml_node<> *node, string &name) {
  ParseStrAttr(node, "Name", name);
}

void ParseIdAttr(xml_node<> *node, string &id) {
  ParseStrAttr(node, "ID", id);
}

void ParseDay(xml_node<> *node, Day &day) {
  memset(&day, 0, sizeof(day));
  if(node) {
    xml_node<> *daynode = node->first_node("Day");
    if(daynode) {
      ParseAttr<int>(daynode, "Day", day.day);
      ParseAttr<int>(daynode, "Month", day.month);
      ParseAttr<int>(daynode, "Year", day.year);
    }
  }
}

void ParseTimeofDay(xml_node<> *node, TimeofDay &timeofday) {
  memset(&timeofday, 0, sizeof(timeofday));
  if(node) {
    xml_node<> *time = node->first_node("Time");
    if(time) {
      ParseAttr<int>(time, "Hour", timeofday.hour);
      ParseAttr<int>(time, "Minute", timeofday.minute);
      ParseAttr<int>(time, "Second", timeofday.second);
    }
  }
}

void ParseTimeSlot(xml_node<> *node, TimeSlot &timeslot) {
  if(node) {
    xml_node<> *start = node->first_node("TimeStart");
    ParseTimeofDay(start, timeslot.start);
    xml_node<> *stop = node->first_node("TimeStop");
    ParseTimeofDay(stop, timeslot.stop);
  }
}

void ParseBaseInfo(xml_node<> *node, BaseInfo &baseinfo) {
  ParseStrAttr(node, "Name", baseinfo.name);
  ParseStrAttr(node, "Type", baseinfo.type);
  ParseAttr<int>(node, "Status", baseinfo.status);
  ParseStrAttr(node, "Comment", baseinfo.comment);
}

void ParseTimeSlots(xml_node<> *node, vector<TimeSlot> &timeslots) {
  TimeSlot ts;

  for(xml_node<> *timeslot = node->first_node("TimeSlot");
      timeslot;
      timeslot = timeslot->next_sibling("TimeSlot")) {
    ParseTimeSlot(timeslot, ts);
    timeslots.push_back(ts);
  }
}

void ParseDateSchd(xml_node<> *node, DateSchd &dateschd) {
  ParseBoolAttr(node, "EveryYear", dateschd.everyyear);
  ParseDay(node, dateschd.day);
  ParseTimeSlots(node, dateschd.timeslots);
}

void ParseScope(xml_node<> *node, Scope &scope) {
  xml_node<> *start = node->first_node("ScopeStart");
  ParseDay(start, scope.start);
  xml_node<> *stop = node->first_node("ScopeStop");
  ParseDay(stop, scope.stop);
}

void ParseWeekSchd(xml_node<> *node, WeekSchd &weekschd) {
  weekschd.weekday = node->first_attribute("Weekday")->value();
  ParseScope(node, weekschd.scope);
  ParseTimeSlots(node, weekschd.timeslots);
}

void ParseSchedule(xml_node<> *node, vector<Schedule> &schds) {
  assert(node);
  xml_node<> *sched = node->first_node("Schedule");
  assert(sched);
  //Parse all the WeekSchedule nodes
  for(xml_node<> *week = sched->first_node("WeekdaySchedule");
      week;
      week = week->next_sibling("WeekdaySchedule")) {
    WeekSchd weekschd;
    ParseWeekSchd(week, weekschd);
    schds.push_back(weekschd);
  }
  //Parse all the DateSchedule nodes
  for(xml_node<> *date = sched->first_node("DateSchedule");
      date;
      date = date->next_sibling("DateSchedule")) {
    DateSchd dateschd;
    ParseDateSchd(date, dateschd);
    schds.push_back(dateschd);
  }
}

void ParseDimension(xml_node<> *node, Dimension &dim) {
  dim.value = 0.0;
  if(node) {
    xml_node<> *dimension = node->first_node("Dimension");
    if(dimension) {
      ParseAttr<double>(dimension, "Value", dim.value);
      ParseStrAttr(dimension, "Unit", dim.unit);
    }
  }
}

void ParseTwoDims(xml_node<> *node, TwoDims &twodims) {
  xml_node<> *xdim = node->first_node("XDimension");
  ParseDimension(xdim, twodims.x);
  xml_node<> *ydim = node->first_node("YDimension");
  ParseDimension(ydim, twodims.y);
}

void ParseGeometry(xml_node<> *node, Geometry &geometry) {
  if(node) {
    xml_node<> *footprint = node->first_node("Footprint");
    ParseTwoDims(footprint, geometry.footprint);
    xml_node<> *position = node->first_node("Position");
    ParseTwoDims(position, geometry.position);
    geometry.orientation = 0;
    xml_node<> *orientation = node->first_node("Orientation");
    if(orientation)
      ParseAttr<int>(orientation, "Value", geometry.orientation);
  }
}

void ParseDate(xml_node<> *node, Date &date) {
  ParseDay(node, date.day);
  ParseTimeofDay(node, date.time);
}

void ParseQuantity(xml_node<> *node, Quantity &quantity) {
  quantity.value = 0;
  if(node) {
    xml_node<> *qtynode = node->first_node("Quantity");
    if(qtynode) {
      ParseAttr<int>(qtynode, "Value", quantity.value);
      ParseStrAttr(qtynode, "Unit", quantity.unit);
    }
  }
}

}