/*  pss_multisite_shop_file.hpp
 *
 *  Created by Rong Zhou on 06/28/10.
 *  Copyright 2010 Palo Alto Research Center Inc. All rights reserved.
 *
 *  data types for pss multi-site shop file
 */

#ifndef PSS_MULTISITE_SHOP_FILE_HPP_INCLUDED_
#define PSS_MULTISITE_SHOP_FILE_HPP_INCLUDED_

#include "pss_parser_utils.hpp"

namespace pss {

struct InterShopDelay {
  std::string source;
  std::string destination;
  bool symmetric;
  int hour;
  int minute;
};

struct ShopFile {
  std::string shopname;
  std::string filename;
};

struct MultisiteShopModel {
  pss::BaseInfo baseinfo;
  std::string creator;
  std::string version;
  std::vector<ShopFile> shopfiles;
  std::vector<InterShopDelay> delays;
};

} // namespace pss

#endif // PSS_MULTISITE_SHOP_FILE_HPP_INCLUDED_
