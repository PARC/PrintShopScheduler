/*  pss_jobs_writer.cpp
 *
 *  Created by Rong Zhou on 10/08/09.
 *  Copyright 2009 Palo Alto Research Center Inc. All rights reserved.
 *
 *  implementation file for pss jobs writer
 */
#include <iostream>
#include "pss_jobs_file.hpp"

using namespace std;

namespace pss {
ostream &operator << (ostream &os, const BaseInfo &baseinfo) {
  os << "Name=\"" << baseinfo.name << '"'
     << " Type=\"" << baseinfo.type << '"'
     << " Status=\"" << baseinfo.status << '"'
     << " Comment=\"" << baseinfo.comment << '"';
  return os;
}

ostream &operator << (ostream &os, const Day day) {
  os << "<Day"
     << " Day=\"" << day.day << '"'
     << " Month=\"" << day.month << '"'
     << " Year=\"" << day.year << '"'
     << "/>";
  return os;
}

ostream &operator << (ostream &os, const TimeofDay time) {
  os << "<Time"
     << " Hour=\"" << time.hour << '"'
     << " Minute=\"" << time.minute << '"'
     << " Second=\"" << time.second << '"'
     << "/>";
  return os;
}

ostream &operator << (ostream &os, const Date date) {
  os << date.day
     << endl
     << date.time;
  return os;
}

ostream &operator << (ostream &os, const Quantity quantity) {
  os << "<Quantity"
     << " Value=\"" << quantity.value << '"'
     << " Unit=\"" << quantity.unit << '"'
     << "/>";
  return os;
}

ostream &operator << (ostream &os, const JobInfo jobinfo) {
  os << "JobID=\"" << jobinfo.jobid << '"'
     << " JobPartID=\"" << jobinfo.jobpartid << '"'
     << " Priority=\"" << jobinfo.priority << '"'
     << " Product=\"" << jobinfo.product << '"'
     << " NumBatches=\"" << jobinfo.numbatches << '"'
     << " IsRoute=\"" << (jobinfo.isroute? "true" : "false") << '"';
  return os;
}

ostream &operator << (ostream &os, const RsrcInfo rsrcinfo) {
  os << "ID=\"" << rsrcinfo.id << '"'
     << " Class=\"" << rsrcinfo.rsrclass << '"'
     << " PartNumber=\"" << rsrcinfo.partnumber << '"'
     << " Quality=\"" << rsrcinfo.quality << '"'
     << " Media=\"" << rsrcinfo.media << '"';
  return os;
}

ostream &operator << (ostream &os, const Resource resource) {
  os << "<Resource "
     << resource.baseinfo
     << ' '
     << resource.rsrcinfo
     << '>' << endl
     << resource.quantity << endl
     << "</Resource>";
  return os;
}

ostream &operator << (ostream &os, const Attribute attr) {
  os << "<Attr Name=\"" << attr.name << "\" Value=\"";
  os << attr.value << "\" />";
  return os;
}

ostream &operator << (ostream &os, const StepInfo stepinfo) {
  os << "Function=\"" << stepinfo.function << '"'
     << " AssignedToCell=\"" << stepinfo.cell << '"'
     << " Large=\"" << (stepinfo.large? "true" : "false") << '"';
  return os;
}

ostream &operator << (ostream &os, const Event &event) {
  os << "<Event"
     << " JobID=\"" << event.jobid << '"'
     << " Station=\"" << event.station << '"'
     << " Event=\"" << event.eventname << '"'
     << " Operator=\"" << event.oper << '"'
     << " Quantity=\"" << event.quantity << '"'
     << " FunctionSequenceID=\"" << event.funcseqid << '"'
     << " SetupTime=\"" << event.setup << '"'
     << " VariableSetupTime=\"" << event.varsetup << '"'
     << '>' << endl
     << "<Timestamp>" << endl
     << event.timestamp << endl
     << "</Timestamp>" << endl
     << "</Event>" << endl;
  return os;
}

void PrintReferences(ostream &os, vector<string> references) {
  vector<string>::const_iterator i;

  for(i = references.begin(); i != references.end(); ++i) {
    os << "<Reference"
       << " ID=\""
       << *i
       << "\"/>"
       << endl;
  }
}

void PrintJobStep(ostream &os, const vector<Step> &steps, vector<Step>::size_type id) {
  const Step &curstep = steps[id];
  os << "<JobStep "
     << curstep.baseinfo << ' '
     << curstep.stepinfo
     << '>' << endl
     << "<ReleasedDate>" << endl << curstep.released << endl << "</ReleasedDate>" << endl
     << "<CompletedDate>" << endl << curstep.completed << endl << "</CompletedDate>" << endl
     << "<InputResources>" << endl;
  PrintReferences(os, curstep.inputrsrcs);
  os << "</InputResources>" << endl
     << "<OutputResources>" << endl;
  PrintReferences(os, curstep.outputrsrcs);
  os << "</OutputResources>" << endl
     << "<Events>" << endl;
  copy(curstep.events.begin(), curstep.events.end(), ostream_iterator<Event>(os, ""));
  os << "</Events>" << endl;
  os << "<Attributes>" << endl;
  for(map<string, string>::const_iterator it = curstep.attributes.begin();
      it != curstep.attributes.end();
      ++it) {
    os << "<Attr Name=\"" << it->first << "\" Value=\"";
    os << it->second << "\" />" << endl;
  }
  os << "</Attributes>" << endl;
  if(id > 0)
    PrintJobStep(os, steps, id - 1);
  os << "</JobStep>" << endl;
}

ostream &operator << (ostream &os, const vector<Step> steps) {
  PrintJobStep(os, steps, steps.size() - 1);
  return os;
}

ostream &operator << (ostream &os, const Job &job) {
  os << "<Job "
     << job.baseinfo << ' '
     << job.jobinfo
     << '>' << endl
     << "<ArrivalDate>" << endl << job.arrival << endl << "</ArrivalDate>" << endl
     << "<DueDate>" << endl << job.due << endl << "</DueDate>" << endl
     << "<ReleasedDate>" << endl << job.released << endl << "</ReleasedDate>" << endl
     << "<CompletedDate>" << endl << job.completed << endl << "</CompletedDate>" << endl
     << job.quantity << endl
     << "<Resources>" << endl;
  copy(job.resources.begin(), job.resources.end(), ostream_iterator<Resource>(os, "\n"));
  os << "</Resources>" << endl;
  os << job.steps
     << "</Job>" << endl;
  return os;
}

ostream &operator << (ostream &os, const Job *job) {
  os << "<Job "
     << job->baseinfo << ' '
     << job->jobinfo
     << '>' << endl
     << "<ArrivalDate>" << endl << job->arrival << endl << "</ArrivalDate>" << endl
     << "<DueDate>" << endl << job->due << endl << "</DueDate>" << endl
     << "<ReleasedDate>" << endl << job->released << endl << "</ReleasedDate>" << endl
     << "<CompletedDate>" << endl << job->completed << endl << "</CompletedDate>" << endl
     << job->quantity << endl
     << "<Resources>" << endl;
  copy(job->resources.begin(), job->resources.end(), ostream_iterator<Resource>(os, "\n"));
  os << "</Resources>" << endl;
  os << job->steps
     << "</Job>" << endl;
  return os;
}

ostream &PrintHeader(ostream &os, const BaseInfo &baseinfo) {
  os << "<?xml version=\"1.0\"?>" << endl
     << "<!DOCTYPE JobList SYSTEM \"http://www.parc.com/PSS/xml/JobList.dtd\">" << endl
     << "<JobList "
     << baseinfo
     << " Creator=\"" << "PARC PSS Scheduler" << '"'
     << " Version=\"" << "1.0" << '"'
     << '>' << endl;
  return os;
}

ostream &operator << (ostream &os, const JobListModel &job_list) {
  PrintHeader(os, job_list.baseinfo);
  copy(job_list.jobs.begin(), job_list.jobs.end(), ostream_iterator<Job>(os, ""));
  os << "</JobList>" << endl;
  return os;
}

void WriteJobList(ostream &os, const BaseInfo &baseinfo, const vector<Job *> jobs) {
  PrintHeader(os, baseinfo);
  copy(jobs.begin(), jobs.end(), ostream_iterator<Job *>(os, ""));
  os << "</JobList>" << endl;
}

} // namespace pss
