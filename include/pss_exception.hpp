/* pss_exception.hpp
 *
 *  Created by Rong Zhou on 06/23/10.
 *  Copyright 2010-2016 Palo Alto Research Center Inc. All rights reserved.
 *
 *  class definition for pss exception
 */

#ifndef PSS_EXCEPTION_HPP_INCLUDED_
#define PSS_EXCEPTION_HPP_INCLUDED_

#include <stdexcept>

namespace pss {

class RuntimeException : public std::runtime_error {
 public:
  RuntimeException(const std::string message) : std::runtime_error(message) {}
};

} // namespace pss

#endif // PSS_EXCEPTION_HPP_INCLUDED_
