/*  pss_shop_job.hpp
 *
 *  Created by Rong Zhou on 05/01/09.
 *  Copyright 2009 Palo Alto Research Center Inc. All rights reserved.
 *
 *  header file for data types and functions for pss jobs expressed
 *  in terms of the function sequences supported by a particular shop
 */

#ifndef PSS_SHOP_JOB_HPP_INCLUDED_
#define PSS_SHOP_JOB_HPP_INCLUDED_

#include "pss_jobs_file.hpp"
#include "pss_shop_func.hpp"

namespace pss {

struct Func {
  std::string name;
  std::vector<std::string> inrsrcs;
  std::vector<std::string> outrsrcs;
  std::vector<unsigned> stepids;  //indices of the corresponding funcsteps[]
  pss::One2One attributes;
};

struct ShopJob {
  unsigned intid;
  std::string id;
  std::string partid;
  std::string group; //group to which the job belongs
  unsigned numbatches;
  unsigned priority;
  time_t arrival;
  time_t due;
  time_t released;
  time_t completed;
  time_t proctime;  //estimation only
  std::vector<pss::Resource> resources;
  std::vector<pss::Func> funcseqs;
  std::vector<pss::Func> funcsteps;
};

void GetShopJob(pss::ShopJob &shop_job, const pss::Job &job,
                const pss::ShopInfo &shop_info, const unsigned job_int_id);

void GetShopJobs(std::vector<pss::ShopJob> &shop_jobs,
                 const std::vector<pss::Job> &jobs,
                 const pss::ShopInfo &shop_info,
                 const unsigned min_job_int_id);

}

#endif // PSS_SHOP_JOB_HPP_INCLUDED_
