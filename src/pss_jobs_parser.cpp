/*  pss_jobs_parser.cpp
 *
 *  Created by Rong Zhou on 04/07/16.
 *  Copyright 2009-16 Palo Alto Research Center Inc. All rights reserved.
 *
 *  implementation file pss jobs parser
 */
#include <fstream>
#include "pss_jobs_parser.hpp"
#include "pss_parser_utils.hpp"

using namespace std;
using namespace rapidxml;

namespace pss {
//Helper function
void ParseJobInfo(xml_node<> *node, JobInfo &jobinfo) {
  ParseStrAttr(node, "JobID", jobinfo.jobid);
  ParseStrAttr(node, "JobPartID", jobinfo.jobpartid);
  ParseAttr<int>(node, "Priority", jobinfo.priority);
  ParseStrAttr(node, "Product", jobinfo.product);
  ParseAttr<int>(node, "NumBatches", jobinfo.numbatches);
  ParseBoolAttr(node, "IsRoute", jobinfo.isroute);
}

//Helper function
void ParseJobDate(xml_node<> *node, char const *name, Date &date) {
  xml_node<> *datenode = node->first_node(name);
  ParseDate(datenode, date);
}

//Helper function
void ParseJobDates(xml_node<> *node, Job &job) {
  ParseJobDate(node, "ArrivalDate", job.arrival);
  ParseJobDate(node, "DueDate", job.due);
  ParseJobDate(node, "ReleasedDate", job.released);
  ParseJobDate(node, "CompletedDate", job.completed);
}

//Helper function
void ParseRsrcInfo(xml_node<> *node, RsrcInfo &rsrcinfo) {
  ParseStrAttr(node, "ID", rsrcinfo.id);
  ParseAttr<int>(node, "Class", rsrcinfo.rsrclass);
  ParseStrAttr(node, "PartNumber", rsrcinfo.partnumber);
  ParseAttr<int>(node, "Quality", rsrcinfo.quality);
  ParseStrAttr(node, "Media", rsrcinfo.media);
}

//Helper function
void ParseResource(xml_node<> *node, Resource &resource) {
  ParseBaseInfo(node, resource.baseinfo);
  ParseRsrcInfo(node, resource.rsrcinfo);
  ParseQuantity(node, resource.quantity);
}

//Helper function
void ParseJobStepInfo(xml_node<> *node, StepInfo &stepinfo) {
  ParseStrAttr(node, "Function", stepinfo.function);
  ParseStrAttr(node, "AssignedToCell", stepinfo.cell);
  ParseBoolAttr(node, "Large", stepinfo.large);
}

//Helper function
void ParseJobStepEvent(xml_node<> *node, Event &event) {
  ParseStrAttr(node, "JobID", event.jobid);
  ParseStrAttr(node, "Station", event.station);
  ParseStrAttr(node, "Event", event.eventname);
  ParseStrAttr(node, "Operator", event.oper);
  ParseAttr<int>(node, "Quantity", event.quantity);
  ParseStrAttr(node, "FunctionSequenceID", event.funcseqid);
  ParseAttr<time_t>(node, "SetupTime", event.setup);
  ParseAttr<time_t>(node, "VariableSetupTime", event.varsetup);
  ParseJobDate(node, "Timestamp", event.timestamp);
}

//Helper function
void ParseJobStep(xml_node<> *node, vector<Step> &steps) {
  Step step;
  ParseBaseInfo(node, step.baseinfo);
  ParseJobStepInfo(node, step.stepinfo);
  xml_node<> *inrsrcnode = node->first_node("InputResources");
  ParseVector<string>(inrsrcnode, "Reference", step.inputrsrcs, ParseIdAttr);
  xml_node<> *outrsrcnode = node->first_node("OutputResources");
  ParseVector<string>(outrsrcnode, "Reference", step.outputrsrcs, ParseIdAttr);
  xml_node<> *eventsnode = node->first_node("Events");
  ParseVector<Event>(eventsnode, "Event", step.events, ParseJobStepEvent);
  xml_node<> *atrsnode = node->first_node("Attributes");
  for(xml_node<> *atrnode = atrsnode->first_node("Attr");
      atrnode;
      atrnode = atrnode->next_sibling("Attr")) {
    string name;
    ParseNameAttr(atrnode, name);
    string value;
    ParseStrAttr(atrnode, "Value", value);
    step.attributes[name] = value;
  }
  //must recur BEFORE calling steps.push_back(step), because we want the
  //nested steps to appear at the front of steps
  for(xml_node<> *stepnode = node->first_node("JobStep");
      stepnode;
      stepnode = stepnode->next_sibling("JobStep")) {
    ParseJobStep(stepnode, steps);
  }
  //only now can we insert 'step' into 'steps'
  steps.push_back(step);
}

//Helper function
void ParseJob(xml_node<> *node, Job &job) {
  ParseBaseInfo(node, job.baseinfo);
  ParseJobInfo(node, job.jobinfo);
  ParseJobDates(node, job);
  ParseQuantity(node, job.quantity);
  //Parse resources
  xml_node<> *rsrcsnode = node->first_node("Resources");
  ParseVector<Resource>(rsrcsnode, "Resource", job.resources, ParseResource);
  //Parse steps
  ParseVector2<Step>(node, "JobStep", job.steps, ParseJobStep);
}

int JobsParser::ParseFile(char const *path) {
  ifstream file(path);
  //read the entire file into buf
  std::vector<char> buf((istreambuf_iterator<char>(file)),
                        istreambuf_iterator<char>());
  buf.push_back('\0');
  xml_document<> doc;
  //Parse the string pointed to by 'buf'
  doc.parse<0>(&buf[0]);
  //Obtain the jobs node
  xml_node<> *jobsnode = doc.first_node("JobList");
  ParseBaseInfo(jobsnode, model_.baseinfo);
  ParseStrAttr(jobsnode, "Creator", model_.creator);
  ParseStrAttr(jobsnode, "Version", model_.version);
  //Parse jobs
  ParseVector<Job>(jobsnode, "Job", model_.jobs, ParseJob);
  return 0;
}

}