/*  pss_multisite_jobs_parser.cpp
 *
 *  Created by Rong Zhou on 04/07/16.
 *  Copyright 2009-16 Palo Alto Research Center Inc. All rights reserved.
 *
 *  implementation file for pss multi-site jobs parser
 */
#include <fstream>
#include "pss_multisite_jobs_parser.hpp"

using namespace std;
using namespace rapidxml;

namespace pss {

//Helper function
void ParseJobsFile(xml_node<> *node, JobFile &jobsfile) {
  ParseNameAttr(node, jobsfile.listname);
  ParseStrAttr(node, "File", jobsfile.filename);
  ParseStrAttr(node, "HomeShop", jobsfile.homeshop);
  ParseBoolAttr(node, "Outsourceable", jobsfile.outsourceable);
}

int MultisiteJobsParser::ParseFile(char const *path) {
  ifstream file(path);
  //read the entire file into buf
  std::vector<char> buf((istreambuf_iterator<char>(file)),
                        istreambuf_iterator<char>());
  buf.push_back('\0');
  xml_document<> doc;
  //Parse the string pointed to by 'buf'
  doc.parse<0>(&buf[0]);
  //Obtain the jobs node
  xml_node<> *jobsnode = doc.first_node("MultiSiteJobList");
  ParseBaseInfo(jobsnode, model_.baseinfo);
  ParseStrAttr(jobsnode, "Creator", model_.creator);
  ParseStrAttr(jobsnode, "Version", model_.version);
  //Parse job list file names
  ParseVector<JobFile>(jobsnode, "JobList", model_.jobfiles, ParseJobsFile);
  return 0;
}

}