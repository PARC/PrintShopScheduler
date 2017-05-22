/* pss_multisite_shop.hpp
 *
 *  Created by Rong Zhou on 06/28/10.
 *  Copyright 2010 Palo Alto Research Center Inc. All rights reserved.
 *
 *  class definition for pss multi-site shop
 */

#ifndef PSS_MULTISITE_SHOP_HPP_INCLUDED_
#define PSS_MULTISITE_SHOP_HPP_INCLUDED_

#include "pss_shop.hpp"
#include "pss_multisite_shop_parser.hpp"

namespace pss {

class MultisiteShop {
 private:
  MultisiteShopModel model_;
  std::vector<Shop> shops_;
  std::map<std::string, unsigned> shop_ids_;
  std::vector<time_t> delay_matrix_;

 public:
  MultisiteShop(const char *filename);

  unsigned GetShopId(const std::string &shopname) const;

  const std::string &GetShopName(unsigned shop_id) const;

  const Shop &GetShop(unsigned shop_id) const;

  int NumOfShops(void) const;

  void ValidateDelayMatrix(void) const;

  void CompileDefinition(void);

  time_t GetDelay(unsigned src_shop_id, unsigned dest_shop_id) const;

  time_t GetDelay(std::string src_shop, std::string dest_shop) const;

  void SetDelay(unsigned src_shop_id, unsigned dest_shop_id, time_t delay);

  void SetDelay(std::string src_shop, std::string dest_shop, time_t delay);
};

} // namespace pss

#endif // PSS_MULTISITE_SHOP_HPP_INCLUDED_
