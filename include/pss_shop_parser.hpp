/*  pss_shop_parser.hpp
 *
 *  Created by Rong Zhou on 04/15/09.
 *  Copyright 2009-16 Palo Alto Research Center Inc. All rights reserved.
 *
 *  data types for pss shop parser
 */

#ifndef PSS_SHOP_PARSER_HPP_INCLUDED_
#define PSS_SHOP_PARSER_HPP_INCLUDED_

#include "pss_shop_file.hpp"

namespace pss {

class ShopParser {
  ShopModel &shopmodel_;

 public:
  ShopParser(ShopModel &shopmodel) : shopmodel_(shopmodel) { }
  int ParseFile(char const *path);
};

} // namespace pss

#endif // PSS_SHOP_PARSER_HPP_INCLUDED_
