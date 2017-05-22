/* pss_multisite_jobs_file.hpp
 *
 *  Created by Rong Zhou on 06/29/10.
 *  Copyright 2010 Palo Alto Research Center Inc. All rights reserved.
 *
 *  data types for pss multi-site job list file
 */

#ifndef PSS_MULTISITE_JOBS_FILE_HPP_INCLUDED_
#define PSS_MULTISITE_JOBS_FILE_HPP_INCLUDED_

#include "pss_parser_utils.hpp"

namespace pss {

struct JobFile {
  std::string listname;
  std::string filename;
  std::string homeshop;
  bool outsourceable;
};

struct MultisiteJobListModel {
  pss::BaseInfo baseinfo;
  std::string creator;
  std::string version;
  std::vector<pss::JobFile> jobfiles;
};

} // namespace pss

#endif // PSS_MULTISITE_JOBS_FILE_HPP_INCLUDED_
