/* pss_multisite_job_list.cpp
 *
 *  Created by Rong Zhou on 06/29/10.
 *  Copyright 2010-2016 Palo Alto Research Center Inc. All rights reserved.
 *
 *  implementation file for pss multi-site job list
 */

#include "pss_multisite_job_list.hpp"

using namespace std;

namespace pss {

void MultisiteJobList::CompileOutsourceableJobs(const MultisiteShop &shop) {
  unsigned num_of_shops = shop.NumOfShops();
  ShopJob shopJob;
  bool firstJobInGroup;
  set<unsigned> shopIdsForFirstJobInGroup;
  unsigned listId = 0;
  for(vector<JobList>::iterator it = lists_.begin();
      it != lists_.end();
      ++it, ++listId) {
    const vector<Job> &list(it->GetList().jobs);
    const vector<ShopJob> &jobs(it->GetJobs());
    vector<Job>::const_iterator j;
    vector<ShopJob>::const_iterator sj;
    if(IsOutsourceableList(listId)) {
      const unsigned homeshop_id = home_shop_ids_[listId];
      map<unsigned, ShopJob> shop2job;
      for(j = list.begin(), sj = jobs.begin(); j != list.end(); ++j, ++sj) {
        if(!sj->group.empty()) {
          group2job_ids_[sj->group].insert(sj->intid);
          if(group2shop_ids_.find(sj->group) != group2shop_ids_.end())
            firstJobInGroup = false;
          else {
            firstJobInGroup = true;
            shopIdsForFirstJobInGroup.clear();
          }
        } else
          firstJobInGroup = false;
        unsigned job_int_id = sj->intid;
        for(unsigned shop_id = 0; shop_id < num_of_shops; ++shop_id) {
          if(shop_id != homeshop_id) {
            bool exception = false;
            try {
              GetShopJob(shopJob, *j, shop.GetShop(shop_id).GetInfo(), job_int_id);
            } catch(RuntimeException) {
              exception = true;
            }
            if(!exception) {
              if(sj->group.empty())   //no grouping information -> no such constraints
                shop2job[shop_id] = shopJob;
              else if(!firstJobInGroup) {
                if(group2shop_ids_[sj->group].find(shop_id) != group2shop_ids_[sj->group].end())
                  shop2job[shop_id] = shopJob; //the shop can process all jobs so far
              } else { //first job in its group
                shop2job[shop_id] = shopJob; //all capable shops are considered OK for now
                shopIdsForFirstJobInGroup.insert(shop_id);
              }
            } else if(!sj->group.empty() && !firstJobInGroup) { //exception == true
              if(group2shop_ids_[sj->group].find(shop_id) != group2shop_ids_[sj->group].end()) {
                group2shop_ids_[sj->group].erase(shop_id); //remove ineligible shop_id
              }
            }
          }
        }
        if(!shop2job.empty()) {
          outsource_db_[job_int_id] = shop2job;
        }
        if(firstJobInGroup) {
          group2shop_ids_[sj->group] = shopIdsForFirstJobInGroup;
        }
      }
    } else { //list is not outsourceable
      shopIdsForFirstJobInGroup.clear();
      for(j = list.begin(), sj = jobs.begin(); j != list.end(); ++j, ++sj) {
        if(!sj->group.empty())   //has group constraints yet the job is non-outsourceable
          group2shop_ids_[sj->group] = shopIdsForFirstJobInGroup; //clear the set of shops for the group
      }
    }
  }
  //prune shops that cannot process all jobs in a group based on group2shop_ids_[]
  listId = 0;
  for(vector<JobList>::iterator it = lists_.begin(); it != lists_.end(); ++it, ++listId) {
    if(IsOutsourceableList(listId)) {
      const vector<Job> &list(it->GetList().jobs);
      const vector<ShopJob> &jobs(it->GetJobs());
      vector<Job>::const_iterator j;
      vector<ShopJob>::const_iterator sj;
      for(j = list.begin(), sj = jobs.begin(); j != list.end(); ++j, ++sj) {
        unsigned job_int_id = sj->intid;
        if(!sj->group.empty() && outsource_db_.find(job_int_id) != outsource_db_.end()) {
          map<unsigned, ShopJob> &shop2jobRef = outsource_db_[job_int_id];
          for(map<unsigned, ShopJob>::iterator s2jItr = shop2jobRef.begin(); s2jItr != shop2jobRef.end();) {
            if(group2shop_ids_[sj->group].find(s2jItr->first) == group2shop_ids_[sj->group].end())
              shop2jobRef.erase(s2jItr++->first);
            else
              ++s2jItr;
          }
          if(shop2jobRef.empty()) {
            outsource_db_.erase(job_int_id);
          }
        }
      }
    }
  }
}

void MultisiteJobList::CompileDefinition(const MultisiteShop &shop) {
  unsigned listId = 0;
  unsigned num_of_shops = shop.NumOfShops();
  vector<unsigned> outsourceableListId(num_of_shops, (unsigned)-1);
  vector<unsigned> nonOutsourceableListId(num_of_shops, (unsigned)-1);
  set<string> filenames;
  unsigned minJobId = 0;
  for(vector<JobFile>::iterator it = model_.jobfiles.begin();
      it != model_.jobfiles.end();
      ++it, ++listId) {
    unsigned shop_id;
    try {
      shop_id = shop.GetShopId(GetHomeShopName(listId));
      home_shop_ids_.push_back(shop_id);
    } catch(RuntimeException) {
      throw RuntimeException("Invalid home shop name: \"" + GetHomeShopName(listId) +
                             "\" for job list: \"" + GetListName(listId) + '\"');
    }
    if(it->outsourceable) {
      if(outsourceableListId[shop_id] == (unsigned)-1) {
        outsourceableListId[shop_id] = listId;
      } else {
        throw RuntimeException("Invalid outsourceable status for job list: \"" + GetListName(listId) + '\"'
                               + "\n\tShop \"" + GetHomeShopName(listId) + "\" already has an outsourceable job list called \""
                               + GetListName(outsourceableListId[shop_id]) + '\"');
      }
    } else {
      if(nonOutsourceableListId[shop_id] == (unsigned)-1) {
        nonOutsourceableListId[shop_id] = listId;
      } else {
        throw RuntimeException("Invalid outsourceable status for job list: \"" + GetListName(listId) + '\"'
                               + "\n\tShop \"" + GetHomeShopName(listId) + "\" already has a non-outsourceable job list called \""
                               + GetListName(nonOutsourceableListId[shop_id]) + '\"');
      }
    }
    if(filenames.find(it->filename) != filenames.end()) {
      throw RuntimeException("Duplicate job list file name: \"" + it->filename + '\"');
    }
    JobList list(it->filename.c_str(), shop.GetShop(shop_id).GetInfo(), minJobId);
    minJobId += list.size();
    filenames.insert(it->filename);
    lists_.push_back(list);
    if(list_ids_.find(it->listname) != list_ids_.end()) {
      throw RuntimeException("Duplicate job list name: \"" + GetListName(listId) + '\"');
    }
    list_ids_[it->listname] = listId;
  }
}

int MultisiteJobList::NumOfLists(void) const {
  return (int)model_.jobfiles.size();
}

unsigned MultisiteJobList::GetListId(const string &listname) const {
  map<string, unsigned>::const_iterator it = list_ids_.find(listname);
  if(it == list_ids_.end()) {
    throw RuntimeException("Invalid job list name: " + listname);
  }
  return it->second;
}

const string &MultisiteJobList::GetListName(const unsigned listId) const {
  if(listId >= model_.jobfiles.size()) {
    throw RuntimeException("Invalid job list id: " + listId);
  }
  return model_.jobfiles[listId].listname;
}

const string &MultisiteJobList::GetHomeShopName(const unsigned listId) const {
  if(listId >= model_.jobfiles.size()) {
    throw RuntimeException("Invalid job list id: " + listId);
  }
  return model_.jobfiles[listId].homeshop;
}

const string &MultisiteJobList::GetHomeShopName(const string &listname) const {
  unsigned listId = GetListId(listname);
  return GetHomeShopName(listId);
}

unsigned MultisiteJobList::GetHomeShopId(const unsigned listId) const {
  if(listId >= model_.jobfiles.size()) {
    throw RuntimeException("Invalid job list id: " + listId);
  }
  return home_shop_ids_[listId];
}

unsigned MultisiteJobList::GetHomeShopId(const string &listname) const {
  unsigned listId = GetListId(listname);
  return GetHomeShopId(listId);
}

bool MultisiteJobList::IsOutsourceableList(const unsigned listId) const {
  if(listId >= model_.jobfiles.size()) {
    stringstream ss;
    ss << listId;
    throw RuntimeException("Invalid job list id: " + ss.str());
  }
  return model_.jobfiles[listId].outsourceable;
}

void MultisiteJobList::ShopJobPointers(vector<ShopJob *> &shop_job_pointers) {
  shop_job_pointers.clear();
  for(vector<JobList>::iterator it = lists_.begin(); it != lists_.end(); ++it) {
    it->AppendShopJobPointers(shop_job_pointers);
  }
}

MultisiteJobList::MultisiteJobList(const char *filename, const MultisiteShop &shop) {
  MultisiteJobsParser parser(model_);
  parser.ParseFile(filename);
  CompileDefinition(shop);
  CompileOutsourceableJobs(shop);
}

} // namespace pss
