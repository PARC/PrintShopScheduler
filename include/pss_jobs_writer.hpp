/*  pss_jobs_writer.hpp
 *
 *  Created by Rong Zhou on 10/08/09.
 *  Copyright 2009 Palo Alto Research Center Inc. All rights reserved.
 *
 *  header file for pss jobs writer
 */

#ifndef PSS_JOBS_WRITER_HPP_INCLUDED
#define PSS_JOBS_WRITER_HPP_INCLUDED

#include "pss_jobs_file.hpp"

namespace pss {

std::ostream &operator << (std::ostream &os, const pss::JobListModel &job_list);

void WriteJobList(std::ostream &os, const BaseInfo &baseinfo,
                  const std::vector<Job *> jobs);

}

#endif // PSS_JOBS_WRITER_HPP_INCLUDED
