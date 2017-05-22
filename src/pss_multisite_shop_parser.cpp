/*  pss_multisite_shop_parser.cpp
 *
 *  Created by Rong Zhou on 04/07/16.
 *  Copyright 2009-16 Palo Alto Research Center Inc. All rights reserved.
 *
 *  implementation file pss multi-site shop parser
 */
#include <fstream>
#include "pss_multisite_shop_parser.hpp"

using namespace std;
using namespace rapidxml;

namespace pss {

//Helper function
void ParseInterShopDelay(xml_node<> *node, InterShopDelay &delay) {
  ParseStrAttr(node, "SourceShop", delay.source);
  ParseStrAttr(node, "DestinationShop", delay.destination);
  ParseBoolAttr(node, "Symmetric", delay.symmetric);
  ParseAttr<int>(node, "Hour", delay.hour);
  ParseAttr<int>(node, "Minute", delay.minute);
}

//Helper function
void ParseShop(xml_node<> *node, ShopFile &shopfile) {
  assert(node);
  ParseNameAttr(node, shopfile.shopname);
  ParseStrAttr(node, "File", shopfile.filename);
}

int MultisiteShopParser::ParseFile(char const *path) {
  ifstream file(path);
  //read the entire file into buf
  std::vector<char> buf((istreambuf_iterator<char>(file)),
                        istreambuf_iterator<char>());
  buf.push_back('\0');
  xml_document<> doc;
  //Parse the string pointed to by 'buf'
  doc.parse<0>(&buf[0]);
  //Obtain the multi-site shop node
  xml_node<> *shpnode = doc.first_node("MultiSiteShop");
  ParseBaseInfo(shpnode, model_.baseinfo);
  ParseStrAttr(shpnode, "Creator", model_.creator);
  ParseStrAttr(shpnode, "Version", model_.version);
  //Parse single-site shop file names
  ParseVector<ShopFile>(shpnode, "Shop", model_.shopfiles, ParseShop);
  //Parse intershop delays
  ParseVector<InterShopDelay>(shpnode, "InterShopDelay", model_.delays,
                                 ParseInterShopDelay);
  return 0;
}

}