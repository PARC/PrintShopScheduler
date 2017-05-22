/*  pss_multisite_jobs_parser.hpp
 *
 *  Created by Rong Zhou on 04/07/16.
 *  Copyright 2009-16 Palo Alto Research Center Inc. All rights reserved.
 *
 *  data types for pss multi-site jobs parser
 */

#ifndef PSS_MULTISITE_JOBS_PARSER_HPP_INCLUDED_
#define PSS_MULTISITE_JOBS_PARSER_HPP_INCLUDED_

#include "pss_multisite_jobs_file.hpp"

namespace pss {
class MultisiteJobsParser {
  MultisiteJobListModel &model_;

 public:
  MultisiteJobsParser(MultisiteJobListModel &model) : model_(model) { }
  int ParseFile(char const *path);
};
} // namespace pss

#endif // PSS_MULTISITE_JOBS_PARSER_HPP_INCLUDED_
