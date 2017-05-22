/*  pss_jobs_parser.hpp
 *
 *  Created by Rong Zhou on 04/15/09.
 *  Copyright 2009-16 Palo Alto Research Center Inc. All rights reserved.
 *
 *  data types for pss job parser
 */

#ifndef PSS_JOBS_PARSER_HPP_INCLUDED_
#define PSS_JOBS_PARSER_HPP_INCLUDED_

#include "pss_jobs_file.hpp"

namespace pss {
class JobsParser {
  JobListModel &model_;

 public:
  JobsParser(JobListModel &model) : model_(model) { }
  int ParseFile(char const *path);
};

} // namespace pss

#endif // PSS_JOBS_PARSER_HPP_INCLUDED_
