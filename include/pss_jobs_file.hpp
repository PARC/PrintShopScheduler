/*  pss_jobs_file.hpp
 *
 *  Created by Rong Zhou on 04/15/09.
 *  Copyright 2009 Palo Alto Research Center Inc. All rights reserved.
 *
 *  data types for pss jobs
 */

#ifndef PSS_JOBS_FILE_HPP_INCLUDED_
#define PSS_JOBS_FILE_HPP_INCLUDED_

#include "pss_parser.hpp"

namespace pss {

struct JobInfo {
  std::string jobid;
  std::string jobpartid;
  int priority;
  std::string product;
  int numbatches;
  bool isroute;
};

struct RsrcInfo {
  std::string id;
  int rsrclass;
  std::string partnumber;
  int quality;
  std::string media;
};

struct Resource {
  pss::BaseInfo baseinfo;
  pss::RsrcInfo rsrcinfo;
  pss::Quantity quantity;
};

struct StepInfo {
  std::string function;
  std::string cell;
  bool large;
};

struct Event {
  std::string jobid;
  std::string station;
  std::string eventname;
  std::string oper;
  int quantity;
  std::string funcseqid;
  time_t setup;
  time_t varsetup;
  pss::Date timestamp;
};

struct Step {
  pss::BaseInfo baseinfo;
  pss::StepInfo stepinfo;
  pss::Date released;
  pss::Date completed;
  std::vector<std::string> inputrsrcs;
  std::vector<std::string> outputrsrcs;
  std::vector<pss::Event> events;
  std::map<std::string, std::string> attributes;
};

struct Job {
  pss::BaseInfo baseinfo;
  pss::JobInfo jobinfo;
  pss::Date arrival;
  pss::Date due;
  pss::Date released;
  pss::Date completed;
  pss::Quantity quantity;
  std::vector<pss::Resource> resources;
  std::vector<pss::Step> steps;
};

struct JobListModel {
  pss::BaseInfo baseinfo;
  std::string creator;
  std::string version;
  std::vector<pss::Job> jobs;
};

}

#endif // PSS_JOBS_FILE_HPP_INCLUDED_
