/* pss_multisite_job_list.hpp
 *
 *  Created by Rong Zhou on 06/29/10.
 *  Copyright 2010 Palo Alto Research Center Inc. All rights reserved.
 *
 *  class definition for pss multi-site job list
 */
#ifndef PSS_MULTISITE_JOB_LIST_HPP_INCLUDED_
#define PSS_MULTISITE_JOB_LIST_HPP_INCLUDED_

#include "pss_job_list.hpp"
#include "pss_multisite_jobs_parser.hpp"
#include "pss_multisite_shop.hpp"

namespace pss {
class MultisiteJobList {
 private:
  MultisiteJobListModel model_;
  std::vector<JobList> lists_;
  std::map<std::string, unsigned> list_ids_;
  std::vector<unsigned> home_shop_ids_;
  std::map<unsigned, std::map<unsigned, pss::ShopJob> > outsource_db_;
  //list of shops that can process all jobs in a group
  std::map<std::string, std::set<unsigned> > group2shop_ids_;
  //list of job ids belong to the same group
  std::map<std::string, std::set<unsigned> > group2job_ids_;

  void CompileOutsourceableJobs(const MultisiteShop &shop);

  void CompileDefinition(const MultisiteShop &shop);

 public:
   int NumOfLists(void) const;

   unsigned GetListId(const std::string &listname) const;

   const std::string &GetListName(const unsigned listId) const;

   const std::string &GetHomeShopName(const unsigned listId) const;

   const std::string &GetHomeShopName(const std::string &listname) const;

   unsigned GetHomeShopId(const unsigned listId) const;

   unsigned GetHomeShopId(const std::string &listname) const;

   bool IsOutsourceableList(const unsigned listId) const;

   void ShopJobPointers(std::vector<pss::ShopJob *> &shop_job_pointers);

   MultisiteJobList(const char *filename, const MultisiteShop &shop);

  friend class MultisiteScheduler;
};

} // namespace pss

#endif // PSS_MULTISITE_JOB_LIST_HPP_INCLUDED_
