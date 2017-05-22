/* pss_shop.hpp
 *
 *  Created by Rong Zhou on 06/23/10.
 *  Copyright 2010-2016 Palo Alto Research Center Inc. All rights reserved.
 *
 *  class definition for pss shop
 */

#ifndef PSS_SHOP_HPP_INCLUDED_
#define PSS_SHOP_HPP_INCLUDED_

#include <fstream>
#include "pss_shop_parser.hpp"
#include "pss_shop_func.hpp"
#include "pss_exception.hpp"

namespace pss {

class Shop {
 private:
  ShopModel model_;
  ShopInfo info_;

 public:
  Shop(std::string const &filename) {
    ShopParser shop_parser(model_);
    shop_parser.ParseFile(filename.c_str());
    GetShopInfo(info_, model_);
  }

  const ShopInfo &GetInfo(void) const {
    return info_;
  }
};

} // namespace pss

#endif // PSS_SHOP_HPP_INCLUDED_
