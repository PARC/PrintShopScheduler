/* pss_multisite_shop.cpp
 *
 *  Created by Rong Zhou on 06/28/10.
 *  Copyright 2010-2016 Palo Alto Research Center Inc. All rights reserved.
 *
 *  implementation file for pss multi-site shop
 */
#include "pss_multisite_shop.hpp"

using namespace std;

namespace pss {

unsigned MultisiteShop::GetShopId(const string &shopname) const {
  map<string, unsigned>::const_iterator it = shop_ids_.find(shopname);
  if(it == shop_ids_.end()) {
    throw RuntimeException("Invalid shop name: " + shopname);
  }
  return it->second;
}

const string &MultisiteShop::GetShopName(unsigned shop_id) const {
  if(shop_id >= model_.shopfiles.size()) {
    throw RuntimeException("Invalid shop id: " + shop_id);
  }
  return model_.shopfiles[shop_id].shopname;
}

const Shop &MultisiteShop::GetShop(unsigned shop_id) const {
  if(shop_id >= shops_.size()) {
    throw RuntimeException("Invalid shop id: " + shop_id);
  }
  return shops_[shop_id];
}

int MultisiteShop::NumOfShops(void) const {
  return (int)shops_.size();
}

void MultisiteShop::ValidateDelayMatrix(void) const {
  unsigned num_of_shops = (unsigned)model_.shopfiles.size();
  for(unsigned src_shop_id = 0; src_shop_id < num_of_shops; ++src_shop_id) {
    for(unsigned dest_shop_id = 0; dest_shop_id < num_of_shops; ++dest_shop_id) {
      if(src_shop_id != dest_shop_id) {
        if(GetDelay(src_shop_id, dest_shop_id) == 0)
          throw RuntimeException("Inter-shop delay undefined from source shop: \""
                                 + GetShopName(src_shop_id) + "\" to destination shop: \""
                                 + GetShopName(dest_shop_id) + '\"');
      } else if(GetDelay(src_shop_id, dest_shop_id) != 0) {
        throw RuntimeException("Same-shop delay greater than zero for shop: \""
                               + GetShopName(src_shop_id) + '\"');
      }
    }
  }
}

void MultisiteShop::CompileDefinition(void) {
  unsigned shop_id = 0;
  for(vector<ShopFile>::iterator it = model_.shopfiles.begin();
      it != model_.shopfiles.end();
      ++it, ++shop_id) {
    Shop shop(it->filename);
    shops_.push_back(shop);
    shop_ids_[it->shopname] = shop_id;
  }
  int num_of_shops = (int)model_.shopfiles.size();
  delay_matrix_.resize(num_of_shops * num_of_shops);
  fill(delay_matrix_.begin(), delay_matrix_.end(), 0);
  for(vector<InterShopDelay>::iterator it = model_.delays.begin();
      it != model_.delays.end();
      ++it) {
    time_t delay = it->hour * 3600 + it->minute * 60;
    SetDelay(it->source, it->destination, delay);
    if(it->symmetric) {
      SetDelay(it->destination, it->source, delay);
    }
  }
  ValidateDelayMatrix();
}

MultisiteShop::MultisiteShop(const char *filename) {
  MultisiteShopParser parser(model_);
  parser.ParseFile(filename);
  CompileDefinition();
}

time_t MultisiteShop::GetDelay(unsigned src_shop_id, unsigned dest_shop_id) const {
  return delay_matrix_[src_shop_id * shops_.size() + dest_shop_id];
}

time_t MultisiteShop::GetDelay(string src_shop, string dest_shop) const {
  unsigned src_shop_id = GetShopId(src_shop);
  unsigned dest_shop_id = GetShopId(dest_shop);
  return GetDelay(src_shop_id, dest_shop_id);
}

void MultisiteShop::SetDelay(unsigned src_shop_id, unsigned dest_shop_id, time_t delay) {
  delay_matrix_[src_shop_id * shops_.size() + dest_shop_id] = delay;
}

void MultisiteShop::SetDelay(string src_shop, string dest_shop, time_t delay) {
  unsigned src_shop_id = GetShopId(src_shop);
  unsigned dest_shop_id = GetShopId(dest_shop);
  SetDelay(src_shop_id, dest_shop_id, delay);
}

} // namespace pss
